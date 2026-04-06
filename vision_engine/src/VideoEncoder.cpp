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
    // Video
    const AVCodec*   codec     = nullptr;
    AVCodecContext*   codecCtx  = nullptr;
    AVFrame*         frame     = nullptr;
    AVPacket*        packet    = nullptr;
    SwsContext*      swsCtx    = nullptr;
    // Audio (AAC)
    const AVCodec*   audioCodec    = nullptr;
    AVCodecContext*   audioCodecCtx = nullptr;
    AVFrame*         audioFrame    = nullptr;
    AVPacket*        audioPacket   = nullptr;
    bool             audioInit     = false;
    int              audioFrameSize = 1024;  // AAC frame size
    QByteArray       audioBuffer;            // PCM accumulator
#endif
    EncoderConfig    config;
    bool             initialized = false;
    qint64           frameCount  = 0;
    qint64           audioFrameCount = 0;
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
        // Use VideoToolbox ONLY for non-CBR (local recording, NDI, etc.)
        // For CBR streaming (RTMP/SRT), use libx264 which properly supports CBR + filler
        if (config.rateControl != "cbr") {
            m_impl->codec = avcodec_find_encoder_by_name("h264_videotoolbox");
            if (m_impl->codec) codecName = "h264_videotoolbox";
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

    // Fractional frame rates: 29.97 = 30000/1001, 59.94 = 60000/1001, 23.976 = 24000/1001
    ctx->time_base = {config.fpsDen, config.fpsNum};
    ctx->framerate = {config.fpsNum, config.fpsDen};

    ctx->bit_rate  = static_cast<int64_t>(config.bitrateMbps) * 1000000;
    ctx->gop_size  = config.gopSize;

    // ── Pixel format: 4:2:0 or 4:2:2, 8-bit or 10-bit (SMPTE ST 274 / EBU R 137) ──
    if (config.chromaFormat == "yuv422p" && config.bitDepth == 10)
        ctx->pix_fmt = AV_PIX_FMT_YUV422P10LE;
    else if (config.chromaFormat == "yuv422p")
        ctx->pix_fmt = AV_PIX_FMT_YUV422P;
    else if (config.bitDepth == 10)
        ctx->pix_fmt = AV_PIX_FMT_YUV420P10LE;
    else
        ctx->pix_fmt = AV_PIX_FMT_YUV420P;

    // ── Color space: BT.601 (SD), BT.709 (HD), BT.2020 (UHD) per ITU-R ──
    if (config.resolution.height() <= 576) {
        // SD content: BT.601
        ctx->colorspace      = AVCOL_SPC_BT470BG;
        ctx->color_primaries = AVCOL_PRI_BT470BG;
        ctx->color_trc       = AVCOL_TRC_BT709;
    } else if (config.resolution.height() > 1080) {
        // UHD/4K: BT.2020
        ctx->colorspace      = AVCOL_SPC_BT2020_NCL;
        ctx->color_primaries = AVCOL_PRI_BT2020;
        ctx->color_trc       = AVCOL_TRC_BT2020_10;
    } else {
        // HD (720p/1080p): BT.709
        ctx->colorspace      = AVCOL_SPC_BT709;
        ctx->color_primaries = AVCOL_PRI_BT709;
        ctx->color_trc       = AVCOL_TRC_BT709;
    }
    ctx->color_range = AVCOL_RANGE_MPEG; // Limited range (16-235) — broadcast standard

    // ── H.264/H.265 Profile (ATSC A/53, DVB) ──
    if (QString::fromUtf8(codecName).contains("libx264")) {
        if (config.profile == "baseline")
            av_opt_set(ctx->priv_data, "profile", "baseline", 0);
        else if (config.profile == "main")
            av_opt_set(ctx->priv_data, "profile", "main", 0);
        else
            av_opt_set(ctx->priv_data, "profile", "high", 0);

        if (config.bitDepth == 10)
            av_opt_set(ctx->priv_data, "profile", "high10", 0);
        if (config.chromaFormat == "yuv422p")
            av_opt_set(ctx->priv_data, "profile", "high422", 0);
    }

    // ── Rate control: ABR, CBR, or VBR (DVB/ATSC transport compliance) ──
    if (config.rateControl == "cbr") {
        ctx->rc_max_rate = ctx->bit_rate;
        ctx->rc_min_rate = ctx->bit_rate;
        ctx->rc_buffer_size = config.bufSizeMbps > 0
            ? static_cast<int>(config.bufSizeMbps) * 1000000
            : static_cast<int>(ctx->bit_rate);  // 1 second buffer
        if (QString::fromUtf8(codecName).contains("libx264"))
            av_opt_set(ctx->priv_data, "nal-hrd", "cbr", 0);
    } else if (config.rateControl == "vbr") {
        ctx->rc_max_rate = config.maxBitrateMbps > 0
            ? static_cast<int64_t>(config.maxBitrateMbps) * 1000000
            : ctx->bit_rate * 2;
        ctx->rc_buffer_size = config.bufSizeMbps > 0
            ? static_cast<int>(config.bufSizeMbps) * 1000000
            : static_cast<int>(ctx->rc_max_rate);
    }

    if (config.lowLatency) {
        if (QString::fromUtf8(codecName).contains("libx264")) {
            if (config.rateControl == "cbr") {
                // ── OBS Studio-compatible CBR streaming settings ──
                // Matches OBS x264 defaults for YouTube/Twitch RTMP
                av_opt_set(ctx->priv_data, "preset", "veryfast", 0);
                // NO tune — OBS doesn't set tune for streaming

                // VBV strict CBR: maxrate = bufsize = bitrate (OBS default)
                ctx->rc_max_rate = ctx->bit_rate;
                ctx->rc_min_rate = ctx->bit_rate;
                ctx->rc_buffer_size = static_cast<int>(ctx->bit_rate); // 1 second buffer

                // B-frames for better compression (OBS uses bframes=2)
                ctx->max_b_frames = 2;

                // Keyframe every 2 seconds (YouTube/Twitch requirement)
                int fpsInt = ctx->framerate.den > 0 ? ctx->framerate.num / ctx->framerate.den : 30;
                ctx->gop_size = fpsInt * 2;

                // x264-specific: rc-lookahead for stable bitrate + force-cfr
                av_opt_set(ctx->priv_data, "rc-lookahead", "60", 0);
                av_opt_set(ctx->priv_data, "force-cfr", "1", 0);
                // Profile High (YouTube recommended, enables CABAC)
                av_opt_set(ctx->priv_data, "profile", "high", 0);
            } else {
                // Non-streaming: ultrafast + zerolatency for minimum latency
                ctx->max_b_frames = 0;
                ctx->flags |= AV_CODEC_FLAG_LOW_DELAY;
                av_opt_set(ctx->priv_data, "preset", "ultrafast", 0);
                av_opt_set(ctx->priv_data, "tune", "zerolatency", 0);
            }
        } else {
            // Hardware encoder (VideoToolbox, NVENC)
            ctx->max_b_frames = 0;
            ctx->flags |= AV_CODEC_FLAG_LOW_DELAY;
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

    // Color conversion: RGB32 → target pixel format (4:2:0 or 4:2:2, 8 or 10 bit)
    m_impl->swsCtx = sws_getContext(
        ctx->width, ctx->height, AV_PIX_FMT_BGRA,
        ctx->width, ctx->height, ctx->pix_fmt,  // Matches encoder: YUV420P, YUV422P, 10-bit variants
        SWS_BICUBIC, nullptr, nullptr, nullptr   // Broadcast quality (not FAST_BILINEAR)
    );

    m_impl->initialized = true;
    qInfo() << "[Encoder] Video initialized:" << codecName
            << config.resolution << config.bitrateMbps << "Mbps"
            << (config.lowLatency ? "low-latency" : "normal");

    // ── Initialize AAC audio encoder ──────────────────────
    if (config.audioEnabled) {
        m_impl->audioCodec = avcodec_find_encoder(AV_CODEC_ID_AAC);
        if (m_impl->audioCodec) {
            m_impl->audioCodecCtx = avcodec_alloc_context3(m_impl->audioCodec);
            m_impl->audioCodecCtx->sample_fmt = AV_SAMPLE_FMT_FLTP;
            m_impl->audioCodecCtx->sample_rate = config.audioSampleRate;
            m_impl->audioCodecCtx->ch_layout = AV_CHANNEL_LAYOUT_STEREO;
            m_impl->audioCodecCtx->bit_rate = config.audioBitrate;
            m_impl->audioCodecCtx->time_base = {1, config.audioSampleRate};
            m_impl->audioCodecCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

            if (avcodec_open2(m_impl->audioCodecCtx, m_impl->audioCodec, nullptr) == 0) {
                m_impl->audioFrame = av_frame_alloc();
                m_impl->audioFrame->format = AV_SAMPLE_FMT_FLTP;
                m_impl->audioFrame->ch_layout = AV_CHANNEL_LAYOUT_STEREO;
                m_impl->audioFrame->sample_rate = config.audioSampleRate;
                m_impl->audioFrame->nb_samples = m_impl->audioCodecCtx->frame_size;
                av_frame_get_buffer(m_impl->audioFrame, 0);
                m_impl->audioPacket = av_packet_alloc();
                m_impl->audioFrameSize = m_impl->audioCodecCtx->frame_size;
                m_impl->audioInit = true;
                qInfo() << "[Encoder] Audio AAC initialized:" << config.audioSampleRate << "Hz"
                        << config.audioBitrate / 1000 << "kbps";
            } else {
                qWarning() << "[Encoder] Failed to open AAC encoder — audio disabled";
                avcodec_free_context(&m_impl->audioCodecCtx);
            }
        } else {
            qWarning() << "[Encoder] AAC encoder not found — audio disabled";
        }
    }

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
    if (m_impl->swsCtx)        { sws_freeContext(m_impl->swsCtx); m_impl->swsCtx = nullptr; }
    if (m_impl->packet)         { av_packet_free(&m_impl->packet); }
    if (m_impl->frame)          { av_frame_free(&m_impl->frame); }
    if (m_impl->codecCtx)       { avcodec_free_context(&m_impl->codecCtx); }
    // Audio cleanup
    if (m_impl->audioPacket)    { av_packet_free(&m_impl->audioPacket); }
    if (m_impl->audioFrame)     { av_frame_free(&m_impl->audioFrame); }
    if (m_impl->audioCodecCtx)  { avcodec_free_context(&m_impl->audioCodecCtx); }
    m_impl->audioInit = false;
#endif
    m_impl->initialized = false;
}

bool VideoEncoder::hasAudio() const
{
#ifdef PRESTIGE_HAVE_FFMPEG
    return m_impl->audioInit;
#else
    return false;
#endif
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

QList<EncodedPacket> VideoEncoder::encodeAudio(const QByteArray& pcmData)
{
    QList<EncodedPacket> packets;
#ifdef PRESTIGE_HAVE_FFMPEG
    if (!m_impl->audioInit || pcmData.isEmpty())
        return packets;

    // Accumulate PCM data
    m_impl->audioBuffer.append(pcmData);

    int bytesPerSample = 2; // S16
    int channels = m_impl->audioCodecCtx->ch_layout.nb_channels;
    int frameSamples = m_impl->audioFrameSize;
    int frameBytes = frameSamples * bytesPerSample * channels;

    // Encode complete frames from buffer
    while (m_impl->audioBuffer.size() >= frameBytes) {
        const int16_t* src = reinterpret_cast<const int16_t*>(m_impl->audioBuffer.constData());

        av_frame_make_writable(m_impl->audioFrame);
        m_impl->audioFrame->nb_samples = frameSamples;

        // Convert S16 interleaved → FLTP (planar float) for AAC
        float* left  = reinterpret_cast<float*>(m_impl->audioFrame->data[0]);
        float* right = reinterpret_cast<float*>(m_impl->audioFrame->data[1]);
        for (int i = 0; i < frameSamples; ++i) {
            left[i]  = src[i * channels]     / 32768.0f;
            right[i] = src[i * channels + 1] / 32768.0f;
        }

        m_impl->audioFrame->pts = m_impl->audioFrameCount;
        m_impl->audioFrameCount += frameSamples;

        int ret = avcodec_send_frame(m_impl->audioCodecCtx, m_impl->audioFrame);
        if (ret < 0) {
            m_impl->audioBuffer.remove(0, frameBytes);
            continue;
        }

        while (ret >= 0) {
            ret = avcodec_receive_packet(m_impl->audioCodecCtx, m_impl->audioPacket);
            if (ret < 0) break;

            EncodedPacket ep;
            ep.data = QByteArray(reinterpret_cast<char*>(m_impl->audioPacket->data),
                                  m_impl->audioPacket->size);
            ep.pts = m_impl->audioPacket->pts;
            ep.dts = m_impl->audioPacket->dts;
            ep.isKeyframe = true; // AAC frames are always keyframes
            packets.append(ep);

            av_packet_unref(m_impl->audioPacket);
        }

        m_impl->audioBuffer.remove(0, frameBytes);
    }
#else
    Q_UNUSED(pcmData)
#endif
    return packets;
}

} // namespace prestige
