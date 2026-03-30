// ============================================================
// Prestige AI — Video Encoder (FFmpeg)
// ============================================================

#include "VideoEncoder.h"
#include <QDebug>

#ifdef PRESTIGE_HAVE_FFMPEG
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libavutil/opt.h>
#include <libswscale/swscale.h>
}
#endif

namespace prestige {

struct VideoEncoder::Impl {
#ifdef PRESTIGE_HAVE_FFMPEG
    const AVCodec*   codec     = nullptr;
    AVCodecContext*   codecCtx  = nullptr;
    AVFrame*         frame     = nullptr;
    AVPacket*        packet    = nullptr;
    SwsContext*      swsCtx    = nullptr;
#endif
    EncoderConfig    config;
    bool             initialized = false;
    qint64           frameCount  = 0;
    QString          codecName;
};

VideoEncoder::VideoEncoder(QObject* parent)
    : QObject(parent)
    , m_impl(std::make_unique<Impl>())
{
}

VideoEncoder::~VideoEncoder()
{
    shutdown();
}

bool VideoEncoder::initialize(const EncoderConfig& config)
{
#ifdef PRESTIGE_HAVE_FFMPEG
    m_impl->config = config;

    // Find encoder — prefer hardware (VideoToolbox on macOS)
    const char* codecName = nullptr;

    if (config.codec == "h264") {
#ifdef PRESTIGE_PLATFORM_MACOS
        m_impl->codec = avcodec_find_encoder_by_name("h264_videotoolbox");
        if (m_impl->codec) {
            codecName = "h264_videotoolbox";
        }
#endif
        if (!m_impl->codec) {
            m_impl->codec = avcodec_find_encoder_by_name("libx264");
            codecName = "libx264";
        }
        if (!m_impl->codec) {
            m_impl->codec = avcodec_find_encoder(AV_CODEC_ID_H264);
            codecName = "h264_default";
        }
    } else if (config.codec == "hevc") {
#ifdef PRESTIGE_PLATFORM_MACOS
        m_impl->codec = avcodec_find_encoder_by_name("hevc_videotoolbox");
        if (m_impl->codec) codecName = "hevc_videotoolbox";
#endif
        if (!m_impl->codec) {
            m_impl->codec = avcodec_find_encoder_by_name("libx265");
            codecName = "libx265";
        }
    }

    if (!m_impl->codec) {
        emit errorOccurred("No suitable encoder found");
        return false;
    }

    m_impl->codecName = QString::fromUtf8(codecName);

    // Allocate context
    m_impl->codecCtx = avcodec_alloc_context3(m_impl->codec);
    if (!m_impl->codecCtx) {
        emit errorOccurred("Failed to allocate encoder context");
        return false;
    }

    auto* ctx = m_impl->codecCtx;
    ctx->width     = config.resolution.width();
    ctx->height    = config.resolution.height();
    ctx->time_base = {1, config.fps};
    ctx->framerate = {config.fps, 1};
    ctx->bit_rate  = static_cast<int64_t>(config.bitrateMbps) * 1000000;
    ctx->gop_size  = config.gopSize;
    ctx->pix_fmt   = AV_PIX_FMT_YUV420P;

    if (config.lowLatency) {
        ctx->max_b_frames = 0;
        ctx->flags |= AV_CODEC_FLAG_LOW_DELAY;
        if (QString::fromUtf8(codecName).contains("libx264")) {
            av_opt_set(ctx->priv_data, "preset", "ultrafast", 0);
            av_opt_set(ctx->priv_data, "tune", "zerolatency", 0);
        }
    }

    if (avcodec_open2(ctx, m_impl->codec, nullptr) < 0) {
        emit errorOccurred("Failed to open encoder");
        avcodec_free_context(&m_impl->codecCtx);
        return false;
    }

    // Allocate frame + packet
    m_impl->frame = av_frame_alloc();
    m_impl->frame->format = ctx->pix_fmt;
    m_impl->frame->width  = ctx->width;
    m_impl->frame->height = ctx->height;
    av_frame_get_buffer(m_impl->frame, 0);

    m_impl->packet = av_packet_alloc();

    // Color conversion: RGB32 → YUV420P
    m_impl->swsCtx = sws_getContext(
        ctx->width, ctx->height, AV_PIX_FMT_BGRA,
        ctx->width, ctx->height, AV_PIX_FMT_YUV420P,
        SWS_FAST_BILINEAR, nullptr, nullptr, nullptr
    );

    m_impl->initialized = true;
    qInfo() << "[Encoder] Initialized:" << codecName
            << config.resolution << config.bitrateMbps << "Mbps"
            << (config.lowLatency ? "low-latency" : "normal");
    return true;
#else
    m_impl->config = config;
    m_impl->initialized = true;
    m_impl->codecName = "stub";
    qWarning() << "[Encoder] FFmpeg not available — stub mode";
    return true;
#endif
}

void VideoEncoder::shutdown()
{
#ifdef PRESTIGE_HAVE_FFMPEG
    if (m_impl->swsCtx)   { sws_freeContext(m_impl->swsCtx); m_impl->swsCtx = nullptr; }
    if (m_impl->packet)    { av_packet_free(&m_impl->packet); }
    if (m_impl->frame)     { av_frame_free(&m_impl->frame); }
    if (m_impl->codecCtx)  { avcodec_free_context(&m_impl->codecCtx); }
#endif
    m_impl->initialized = false;
}

bool VideoEncoder::isInitialized() const
{
    return m_impl->initialized;
}

QString VideoEncoder::codecName() const
{
    return m_impl->codecName;
}

QList<EncodedPacket> VideoEncoder::encode(const QImage& image)
{
    QList<EncodedPacket> packets;

    if (!m_impl->initialized || image.isNull())
        return packets;

#ifdef PRESTIGE_HAVE_FFMPEG
    auto* ctx = m_impl->codecCtx;
    auto* frame = m_impl->frame;
    auto* pkt = m_impl->packet;

    // Ensure image matches encoder resolution
    QImage scaled = image;
    if (image.size() != QSize(ctx->width, ctx->height)) {
        scaled = image.scaled(ctx->width, ctx->height, Qt::IgnoreAspectRatio, Qt::FastTransformation);
    }
    if (scaled.format() != QImage::Format_RGB32 && scaled.format() != QImage::Format_ARGB32) {
        scaled = scaled.convertToFormat(QImage::Format_RGB32);
    }

    // Convert RGB32 → YUV420P
    av_frame_make_writable(frame);

    const uint8_t* srcData[1] = { scaled.constBits() };
    int srcLinesize[1] = { static_cast<int>(scaled.bytesPerLine()) };

    sws_scale(m_impl->swsCtx,
              srcData, srcLinesize, 0, ctx->height,
              frame->data, frame->linesize);

    frame->pts = m_impl->frameCount++;

    // Encode
    int ret = avcodec_send_frame(ctx, frame);
    if (ret < 0) return packets;

    while (ret >= 0) {
        ret = avcodec_receive_packet(ctx, pkt);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            break;
        if (ret < 0) break;

        EncodedPacket ep;
        ep.data = QByteArray(reinterpret_cast<const char*>(pkt->data), pkt->size);
        ep.pts = pkt->pts;
        ep.dts = pkt->dts;
        ep.isKeyframe = (pkt->flags & AV_PKT_FLAG_KEY) != 0;
        packets.append(ep);

        av_packet_unref(pkt);
    }
#else
    Q_UNUSED(image)
    m_impl->frameCount++;
#endif

    return packets;
}

} // namespace prestige
