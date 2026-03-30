// ============================================================
// Prestige AI — Output Router (FFmpeg-based)
// ============================================================

#include "OutputRouter.h"
#include "VideoEncoder.h"

#include <QDebug>
#include <QElapsedTimer>
#include <QDateTime>
#include <QDir>
#include <QStandardPaths>

#ifdef PRESTIGE_HAVE_FFMPEG
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/opt.h>
}
#endif

namespace prestige {

// ── OutputRouter ───────────────────────────────────────────

OutputRouter::OutputRouter(QObject* parent)
    : QObject(parent)
{
}

OutputRouter::~OutputRouter()
{
    removeAllOutputs();
}

bool OutputRouter::addOutput(int type, const QString& url, int bitrateMbps, int fps)
{
    removeOutput(type);

    auto entry = std::make_unique<OutputEntry>();
    entry->config.type = static_cast<OutputType>(type);
    entry->config.url = url;
    entry->config.bitrateMbps = bitrateMbps;
    entry->config.codec = QStringLiteral("h264");
    entry->bitrateMbps = bitrateMbps;
    entry->fps = fps;
    entry->active = true;
    entry->initialized = false; // Will init on first frame with actual resolution

    m_outputs.emplace(type, std::move(entry));
    emit outputChanged();

    qInfo() << "[OutputRouter] Added output type" << type << "→" << url
            << "(will init at first frame resolution)";
    return true;
}

bool OutputRouter::initializeEntry(OutputEntry& entry, int width, int height)
{
    // Create encoder at the actual frame resolution
    entry.encoder = std::make_unique<VideoEncoder>(this);
    EncoderConfig encConfig;
    encConfig.resolution = {width, height};
    encConfig.fps = entry.fps;
    encConfig.bitrateMbps = entry.bitrateMbps;
    encConfig.lowLatency = true;

    if (!entry.encoder->initialize(encConfig)) {
        qWarning() << "[OutputRouter] Encoder init failed at" << width << "x" << height;
        return false;
    }

    // Create stream
    if (entry.config.type == OutputType::File) {
        entry.stream = std::make_unique<FileOutputStream>(this);
    } else {
        entry.stream = std::make_unique<FfmpegOutputStream>(this);
    }

    if (!entry.stream->open(entry.config, width, height, entry.fps)) {
        qWarning() << "[OutputRouter] Stream open failed:" << entry.config.url;
        entry.encoder.reset();
        return false;
    }

    entry.initialized = true;
    qInfo() << "[OutputRouter] Initialized output at" << width << "x" << height
            << entry.bitrateMbps << "Mbps";
    return true;
}

void OutputRouter::removeOutput(int type)
{
    auto it = m_outputs.find(type);
    if (it == m_outputs.end()) return;

    if (it->second->stream) it->second->stream->close();
    if (it->second->encoder) it->second->encoder->shutdown();

    m_outputs.erase(it);
    emit outputChanged();
}

void OutputRouter::removeAllOutputs()
{
    for (auto& [key, entry] : m_outputs) {
        if (entry->stream) entry->stream->close();
        if (entry->encoder) entry->encoder->shutdown();
    }
    m_outputs.clear();
}

void OutputRouter::sendFrame(const QImage& compositedFrame)
{
    if (compositedFrame.isNull() || m_outputs.empty())
        return;

    int w = compositedFrame.width();
    int h = compositedFrame.height();

    for (auto& [key, entry] : m_outputs) {
        if (!entry->active) continue;

        // Lazy init: initialize encoder+stream at first frame's resolution
        if (!entry->initialized) {
            if (!initializeEntry(*entry, w, h)) {
                entry->active = false;
                emit outputError(key, QStringLiteral("Init failed at %1x%2").arg(w).arg(h));
                continue;
            }
        }

        auto packets = entry->encoder->encode(compositedFrame);

        for (const auto& pkt : packets) {
            if (entry->stream && entry->stream->isOpen()) {
                entry->stream->writePacket(pkt.data, pkt.pts, pkt.dts, pkt.isKeyframe);
                entry->stats.framesSent++;
            }
        }
    }
}

int OutputRouter::activeCount() const
{
    int count = 0;
    for (const auto& [key, entry] : m_outputs) {
        if (entry->active) count++;
    }
    return count;
}

OutputStats OutputRouter::stats(OutputType type) const
{
    auto it = m_outputs.find(static_cast<int>(type));
    if (it != m_outputs.end())
        return it->second->stats;
    return {};
}

// ── FfmpegOutputStream (RTMP/SRT) ─────────────────────────

struct FfmpegOutputStream::Impl {
#ifdef PRESTIGE_HAVE_FFMPEG
    AVFormatContext* fmtCtx   = nullptr;
    AVStream*        stream   = nullptr;
#endif
    OutputConfig     config;
    OutputStats      outputStats;
    bool             opened   = false;
    qint64           ptsBase  = 0;
};

FfmpegOutputStream::FfmpegOutputStream(QObject* parent)
    : IOutputStream(parent)
    , m_impl(std::make_unique<Impl>())
{
}

FfmpegOutputStream::~FfmpegOutputStream()
{
    close();
}

bool FfmpegOutputStream::open(const OutputConfig& config, int width, int height, int fps)
{
    m_impl->config = config;

#ifdef PRESTIGE_HAVE_FFMPEG
    const char* url = config.url.toUtf8().constData();
    QByteArray urlBytes = config.url.toUtf8();

    // Determine format from URL
    const char* formatName = nullptr;
    if (config.url.startsWith("rtmp://"))
        formatName = "flv";
    else if (config.url.startsWith("srt://"))
        formatName = "mpegts";
    else
        formatName = "mpegts";

    int ret = avformat_alloc_output_context2(&m_impl->fmtCtx, nullptr, formatName, urlBytes.constData());
    if (ret < 0 || !m_impl->fmtCtx) {
        qWarning() << "[Output] Failed to create output context for" << config.url;
        return false;
    }

    // Add video stream
    m_impl->stream = avformat_new_stream(m_impl->fmtCtx, nullptr);
    if (!m_impl->stream) {
        avformat_free_context(m_impl->fmtCtx);
        m_impl->fmtCtx = nullptr;
        return false;
    }

    auto* codecpar = m_impl->stream->codecpar;
    codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
    codecpar->codec_id = AV_CODEC_ID_H264;
    codecpar->width = width;
    codecpar->height = height;
    codecpar->bit_rate = static_cast<int64_t>(config.bitrateMbps) * 1000000;
    m_impl->stream->time_base = {1, fps};

    // Open output URL
    if (!(m_impl->fmtCtx->oformat->flags & AVFMT_NOFILE)) {
        ret = avio_open(&m_impl->fmtCtx->pb, urlBytes.constData(), AVIO_FLAG_WRITE);
        if (ret < 0) {
            qWarning() << "[Output] Failed to open" << config.url;
            avformat_free_context(m_impl->fmtCtx);
            m_impl->fmtCtx = nullptr;
            return false;
        }
    }

    // Write header
    ret = avformat_write_header(m_impl->fmtCtx, nullptr);
    if (ret < 0) {
        qWarning() << "[Output] Failed to write header to" << config.url;
        close();
        return false;
    }

    m_impl->opened = true;
    qInfo() << "[Output] Opened:" << config.url << "(" << formatName << ")";
    return true;
#else
    Q_UNUSED(width) Q_UNUSED(height) Q_UNUSED(fps)
    qWarning() << "[Output] FFmpeg not available — output disabled";
    return false;
#endif
}

void FfmpegOutputStream::close()
{
#ifdef PRESTIGE_HAVE_FFMPEG
    if (m_impl->fmtCtx && m_impl->opened) {
        av_write_trailer(m_impl->fmtCtx);
        if (m_impl->fmtCtx->pb)
            avio_closep(&m_impl->fmtCtx->pb);
        avformat_free_context(m_impl->fmtCtx);
        m_impl->fmtCtx = nullptr;
    }
#endif
    m_impl->opened = false;
}

bool FfmpegOutputStream::writePacket(const QByteArray& data, qint64 pts, qint64 dts, bool isKeyframe)
{
#ifdef PRESTIGE_HAVE_FFMPEG
    if (!m_impl->opened || !m_impl->fmtCtx)
        return false;

    AVPacket* pkt = av_packet_alloc();
    pkt->data = reinterpret_cast<uint8_t*>(const_cast<char*>(data.constData()));
    pkt->size = data.size();
    pkt->pts = pts;
    pkt->dts = dts;
    pkt->stream_index = 0;
    if (isKeyframe) pkt->flags |= AV_PKT_FLAG_KEY;

    av_packet_rescale_ts(pkt, {1, 25}, m_impl->stream->time_base);

    int ret = av_interleaved_write_frame(m_impl->fmtCtx, pkt);
    av_packet_free(&pkt);

    if (ret >= 0) {
        m_impl->outputStats.framesSent++;
        return true;
    }
    return false;
#else
    Q_UNUSED(data) Q_UNUSED(pts) Q_UNUSED(dts) Q_UNUSED(isKeyframe)
    return false;
#endif
}

bool FfmpegOutputStream::isOpen() const { return m_impl->opened; }
OutputStats FfmpegOutputStream::stats() const { return m_impl->outputStats; }

// ── FileOutputStream ───────────────────────────────────────

struct FileOutputStream::Impl {
#ifdef PRESTIGE_HAVE_FFMPEG
    AVFormatContext* fmtCtx = nullptr;
    AVStream*        stream = nullptr;
#endif
    OutputStats      outputStats;
    bool             opened = false;
};

FileOutputStream::FileOutputStream(QObject* parent)
    : IOutputStream(parent)
    , m_impl(std::make_unique<Impl>())
{
}

FileOutputStream::~FileOutputStream()
{
    close();
}

bool FileOutputStream::open(const OutputConfig& config, int width, int height, int fps)
{
#ifdef PRESTIGE_HAVE_FFMPEG
    QString path = config.url;
    if (path.isEmpty()) {
        path = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation)
               + "/PrestigeAI_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".mp4";
    }
    QByteArray pathBytes = path.toUtf8();

    int ret = avformat_alloc_output_context2(&m_impl->fmtCtx, nullptr, nullptr, pathBytes.constData());
    if (ret < 0 || !m_impl->fmtCtx) return false;

    m_impl->stream = avformat_new_stream(m_impl->fmtCtx, nullptr);
    if (!m_impl->stream) { avformat_free_context(m_impl->fmtCtx); m_impl->fmtCtx = nullptr; return false; }

    auto* cp = m_impl->stream->codecpar;
    cp->codec_type = AVMEDIA_TYPE_VIDEO;
    cp->codec_id = AV_CODEC_ID_H264;
    cp->width = width;
    cp->height = height;
    cp->bit_rate = static_cast<int64_t>(config.bitrateMbps) * 1000000;
    m_impl->stream->time_base = {1, fps};

    if (!(m_impl->fmtCtx->oformat->flags & AVFMT_NOFILE)) {
        if (avio_open(&m_impl->fmtCtx->pb, pathBytes.constData(), AVIO_FLAG_WRITE) < 0)
            return false;
    }

    if (avformat_write_header(m_impl->fmtCtx, nullptr) < 0) { close(); return false; }

    m_impl->opened = true;
    qInfo() << "[FileOutput] Recording to:" << path;
    return true;
#else
    Q_UNUSED(config) Q_UNUSED(width) Q_UNUSED(height) Q_UNUSED(fps)
    return false;
#endif
}

void FileOutputStream::close()
{
#ifdef PRESTIGE_HAVE_FFMPEG
    if (m_impl->fmtCtx && m_impl->opened) {
        av_write_trailer(m_impl->fmtCtx);
        if (m_impl->fmtCtx->pb) avio_closep(&m_impl->fmtCtx->pb);
        avformat_free_context(m_impl->fmtCtx);
        m_impl->fmtCtx = nullptr;
    }
#endif
    m_impl->opened = false;
    qInfo() << "[FileOutput] Recording stopped";
}

bool FileOutputStream::writePacket(const QByteArray& data, qint64 pts, qint64 dts, bool isKeyframe)
{
#ifdef PRESTIGE_HAVE_FFMPEG
    if (!m_impl->opened || !m_impl->fmtCtx) return false;

    AVPacket* pkt = av_packet_alloc();
    pkt->data = reinterpret_cast<uint8_t*>(const_cast<char*>(data.constData()));
    pkt->size = data.size();
    pkt->pts = pts; pkt->dts = dts; pkt->stream_index = 0;
    if (isKeyframe) pkt->flags |= AV_PKT_FLAG_KEY;
    av_packet_rescale_ts(pkt, {1, 25}, m_impl->stream->time_base);

    int ret = av_interleaved_write_frame(m_impl->fmtCtx, pkt);
    av_packet_free(&pkt);
    if (ret >= 0) { m_impl->outputStats.framesSent++; return true; }
    return false;
#else
    Q_UNUSED(data) Q_UNUSED(pts) Q_UNUSED(dts) Q_UNUSED(isKeyframe)
    return false;
#endif
}

bool FileOutputStream::isOpen() const { return m_impl->opened; }
OutputStats FileOutputStream::stats() const { return m_impl->outputStats; }

} // namespace prestige
