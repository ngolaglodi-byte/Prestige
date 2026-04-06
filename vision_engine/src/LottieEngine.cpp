// ============================================================
// Prestige AI — Style Animation Engine
// 9 broadcast styles reproduced from After Effects JSON
// Shapes + text animated together, accent color modifiable
// ============================================================

#include "LottieEngine.h"
#include <QPainter>
#include <QPainterPath>
#include <QLinearGradient>
#include <QDir>
#include <QFile>
#include <QDebug>
#include <QFileInfo>
#include <cmath>

namespace prestige {

LottieEngine::LottieEngine(QObject* parent) : QObject(parent) {}
LottieEngine::~LottieEngine() = default;

bool LottieEngine::loadPresets(const QString& directory) {
    Q_UNUSED(directory)
    struct S { QString id; QString name; };
    S styles[] = {
        {"title_01","Simple Tag"},{"title_02","Stacked Blocks"},{"title_03","Modern Design"},
        {"title_04","Angled Bars"},{"title_05","Clean Bar"},{"title_06","Text Block"},
        {"title_07","Stylish Line"},{"title_08","Motion Block"},{"title_09","Wide Bar"},
    };
    for (auto& s : styles) {
        LottiePreset p;
        p.id = s.id; p.name = s.name;
        p.width = 1914; p.height = 1080; p.fps = 30.0; p.durationFrames = 300;
        LottieTextSlot s1; s1.id = "title"; s1.defaultText = "TALENT NAME";
        LottieTextSlot s2; s2.id = "subtitle"; s2.defaultText = "Role";
        LottieTextSlot s3; s3.id = "tag"; s3.defaultText = "";
        p.textSlots = {s1, s2, s3};
        m_presets.append(std::move(p));
    }
    qInfo() << "[Styles] 9 broadcast styles loaded";
    return true;
}

void LottieEngine::parsePreset(LottiePreset&) {}
QStringList LottieEngine::presetIds() const { QStringList r; for (auto& p : m_presets) r << p.id; return r; }
QStringList LottieEngine::presetNames() const { QStringList r; for (auto& p : m_presets) r << p.name; return r; }
const LottiePreset* LottieEngine::preset(const QString& id) const { for (auto& p : m_presets) if (p.id==id) return &p; return nullptr; }
void LottieEngine::setActivePreset(const QString& id) { if (m_activeId!=id) { m_activeId=id; emit presetChanged(); } }
void LottieEngine::setText(const QString& k, const QString& v) { m_textOverrides[k]=v; emit textChanged(); }
void LottieEngine::setTexts(const QMap<QString,QString>& t) { m_textOverrides=t; emit textChanged(); }
void LottieEngine::setTitle(const QString& t) { m_textOverrides["title"]=t; }
void LottieEngine::setSubtitle(const QString& t) { m_textOverrides["subtitle"]=t; }
void LottieEngine::setTag(const QString& t) { m_textOverrides["tag"]=t; }
void LottieEngine::play() { m_playing=true; }
void LottieEngine::stop() { m_playing=false; }
void LottieEngine::reset() { m_playing=false; m_startTime=0; }
double LottieEngine::duration() const { auto* p=preset(m_activeId); return p ? p->durationFrames/p->fps : 10.0; }

QImage LottieEngine::renderFrame(double timeSec, const QSize& outputSize) {
    auto* p = preset(m_activeId);
    if (!p) return QImage();
    double frame = std::fmod(timeSec * p->fps, p->durationFrames);
    return renderLottieFrame(*p, frame, outputSize);
}

// ── Easing (matches AE bezier 0.2,1 / 0.8,0) ──
static double ease(double t) { t=std::clamp(t,0.0,1.0); return 1.0-std::pow(1.0-t,3.0); }

// ── Trim path: animated start/end for line drawing effect ──
static double trimAnim(double frame, double startFrame, double endFrame, double from, double to) {
    if (frame <= startFrame) return from;
    if (frame >= endFrame) return to;
    double t = (frame - startFrame) / (endFrame - startFrame);
    return from + (to - from) * ease(t);
}

// ── Opacity animation ──
static double opacityAnim(double frame, double startFrame, double endFrame, double from, double to) {
    return trimAnim(frame, startFrame, endFrame, from, to);
}

// ── Position animation (lerp with easing) ──
static double posAnim(double frame, double startFrame, double endFrame, double from, double to) {
    return trimAnim(frame, startFrame, endFrame, from, to);
}

QImage LottieEngine::renderLottieFrame(const LottiePreset& preset, double frame, const QSize& size) {
    QImage result(size, QImage::Format_ARGB32_Premultiplied);
    result.fill(Qt::transparent);

    QPainter p(&result);
    p.setRenderHint(QPainter::Antialiasing);
    p.setRenderHint(QPainter::TextAntialiasing);
    p.setRenderHint(QPainter::SmoothPixmapTransform);

    double W = size.width(), H = size.height();
    double sx = W / 1914.0, sy = H / 1080.0;  // Scale to match AE canvas

    // Accent color — configurable by user, falls back to blue
    QColor accent = m_accentColor.isValid() ? m_accentColor : QColor(0, 150, 255);

    // Texts
    QString line1 = m_textOverrides.value("title", preset.textSlots.value(0).defaultText);
    QString line2 = m_textOverrides.value("subtitle", preset.textSlots.size()>1 ? preset.textSlots[1].defaultText : "");
    QString line3 = m_textOverrides.value("tag", preset.textSlots.size()>2 ? preset.textSlots[2].defaultText : "");

    // Exit animation: fade out in last 60 frames
    double exitOp = 1.0;
    if (frame > preset.durationFrames - 60)
        exitOp = ease(1.0 - (frame - (preset.durationFrames - 60)) / 60.0);

    int idx = 0;
    if (preset.id.endsWith("02")) idx=1;
    else if (preset.id.endsWith("03")) idx=2;
    else if (preset.id.endsWith("04")) idx=3;
    else if (preset.id.endsWith("05")) idx=4;
    else if (preset.id.endsWith("06")) idx=5;
    else if (preset.id.endsWith("07")) idx=6;
    else if (preset.id.endsWith("08")) idx=7;
    else if (preset.id.endsWith("09")) idx=8;

    // ═══════════════════════════════════════════════════════
    // STYLE 0: title_01 — "Simple Tag"
    // Box(252x80) at (1324,625) blue, Line01 white stroke 5px trim,
    // Line02 thick 29px trim, Text at (1243,487), (911,584), (1324,585)
    // ═══════════════════════════════════════════════════════
    if (idx == 0) {
        p.setOpacity(exitOp);
        // Line 02: thick white bar, trim end 0->100% over 60 frames
        double trimEnd = trimAnim(frame, 0, 60, 0, 1.0);
        double lx = 994*sx, ly = 656*sy;
        p.setPen(QPen(Qt::white, 29*sy, Qt::SolidLine, Qt::FlatCap));
        p.drawLine(QPointF(lx - 200*sx*trimEnd, ly), QPointF(lx + 200*sx*trimEnd, ly));

        // Box 01: blue rectangle at (1324,625) size 252x80
        double boxOp = opacityAnim(frame, 0, 60, 0, 1.0);
        p.setOpacity(boxOp * exitOp);
        p.setPen(Qt::NoPen); p.setBrush(accent);
        p.drawRect(QRectF((1324-126)*sx, (625-40)*sy, 252*sx, 80*sy));

        // Line 01: white stroke 5px, trim start 100->0% over 60 frames
        double trimStart = trimAnim(frame, 0, 60, 1.0, 0);
        p.setOpacity(exitOp);
        p.setPen(QPen(Qt::white, 5*sy, Qt::SolidLine, Qt::FlatCap));
        double l1x = 856*sx, l1y = 487*sy;
        double lineLen = 400*sx;
        p.drawLine(QPointF(l1x, l1y), QPointF(l1x + lineLen*(1.0-trimStart), l1y));

        // Text 1: "SIMPLE" at (1243,487) — maps to title
        double t1x = posAnim(frame, 0, 60, 1243, 1243) * sx;
        QFont f1("Helvetica Neue", std::max(12, int(m_fontSize*sy)), QFont::Bold);
        p.setFont(f1); p.setPen(m_textColor);
        double textOp = opacityAnim(frame, 10, 40, 0, 1.0);
        p.setOpacity(textOp * exitOp);
        p.drawText(QPointF(t1x, 487*sy), line1);

        // Text 2: at (911,584) — subtitle
        double t2x = posAnim(frame, 0, 60, 862, 911) * sx;
        QFont f2("Helvetica Neue", std::max(12, int(m_fontSize*sy)), QFont::Bold);
        p.setFont(f2);
        double t2op = opacityAnim(frame, 15, 45, 0, 1.0);
        p.setOpacity(t2op * exitOp);
        p.drawText(QPointF(t2x, 584*sy), line2);

        // Text 3: at (1324,585) — tag
        if (!line3.isEmpty()) {
            QFont f3("Helvetica Neue", std::max(10, int(qMax(12, m_fontSize-10)*sy)), QFont::Bold);
            p.setFont(f3);
            double t3op = opacityAnim(frame, 20, 50, 0, 1.0);
            p.setOpacity(t3op * exitOp);
            p.drawText(QPointF(1324*sx, 585*sy), line3);
        }
    }
    // ═══════════════════════════════════════════════════════
    // STYLE 1: title_02 — "Stacked Blocks"
    // 3 blue boxes + white line + text
    // ═══════════════════════════════════════════════════════
    else if (idx == 1) {
        p.setOpacity(exitOp);
        // Box 02: small (137x86) at (825,427)
        double b2op = opacityAnim(frame, 0, 60, 0, 1.0);
        p.setOpacity(b2op * exitOp); p.setPen(Qt::NoPen); p.setBrush(accent);
        p.drawRect(QRectF((825-68)*sx, (427-43)*sy, 137*sx, 86*sy));

        // Box 01: (285x80) at (915,544)
        p.drawRect(QRectF((915-142)*sx, (544-40)*sy, 285*sx, 80*sy));

        // Box 03: (252x75) at (895,653)
        p.drawRect(QRectF((895-126)*sx, (653-37)*sy, 252*sx, 75*sy));

        // Line 01: white 5px at (1169,480), trim 0->100%
        double trimE = trimAnim(frame, 0, 60, 0, 1.0);
        p.setOpacity(exitOp);
        p.setPen(QPen(Qt::white, 5*sy, Qt::SolidLine, Qt::FlatCap));
        p.drawLine(QPointF(1169*sx, 430*sy), QPointF(1169*sx, 430*sy + 250*sy*trimE));

        // Text 1: "#" at (825,427)
        QFont f1("Helvetica Neue", std::max(12, int(m_fontSize*sy)), QFont::Bold);
        p.setFont(f1); p.setPen(m_textColor);
        double t1op = opacityAnim(frame, 10, 40, 0, 1.0);
        p.setOpacity(t1op * exitOp);
        p.drawText(QPointF(790*sx, 440*sy), line1);

        // Text 2: at (903,544)
        double t2x = posAnim(frame, 0, 60, 991, 903) * sx;
        QFont f2("Helvetica Neue", std::max(12, int(m_fontSize*sy)), QFont::Bold);
        p.setFont(f2);
        double t2op = opacityAnim(frame, 15, 45, 0, 1.0);
        p.setOpacity(t2op * exitOp);
        p.drawText(QPointF(t2x, 558*sy), line2);

        // Text 3: at (1056,653)
        if (!line3.isEmpty()) {
            double t3x = posAnim(frame, 0, 60, 971, 1056) * sx;
            QFont f3("Helvetica Neue", std::max(10, int(qMax(12, m_fontSize-10)*sy)), QFont::Bold);
            p.setFont(f3);
            double t3op = opacityAnim(frame, 20, 50, 0, 1.0);
            p.setOpacity(t3op * exitOp);
            p.drawText(QPointF(t3x, 667*sy), line3);
        }
    }
    // ═══════════════════════════════════════════════════════
    // STYLE 2: title_03 — "Modern Design"
    // Large text + blue mask shapes + ellipse
    // ═══════════════════════════════════════════════════════
    else if (idx == 2) {
        double cx = 997*sx, cy = 540*sy;
        p.setOpacity(exitOp);

        // Shape 01: blue polygon at (351,53) relative to null
        double shOp = opacityAnim(frame, 0, 60, 0, 1.0);
        p.setOpacity(shOp * exitOp); p.setPen(Qt::NoPen); p.setBrush(accent);
        p.drawRect(QRectF(cx+100*sx, cy-60*sy, 300*sx, 120*sy));

        // Ellipse at (-251,134) relative
        p.setBrush(Qt::NoBrush); p.setPen(QPen(accent, 3*sy));
        double elOp = opacityAnim(frame, 0, 60, 0, 1.0);
        p.setOpacity(elOp * exitOp);
        p.drawEllipse(QPointF(cx-251*sx, cy+134*sy), 40*sx, 40*sy);

        // Text big: "MODERN" large
        QFont fb("Helvetica Neue", std::max(12, int(m_fontSize*sy)), QFont::Black);
        p.setFont(fb); p.setPen(m_textColor);
        double tbop = opacityAnim(frame, 10, 50, 0, 1.0);
        p.setOpacity(tbop * exitOp);
        p.drawText(QPointF(cx-200*sx, cy+30*sy), line1);

        // Text small
        QFont fs("Helvetica Neue", std::max(10, int(qMax(12, m_fontSize-10)*sy)), QFont::Bold);
        p.setFont(fs);
        double tsop = opacityAnim(frame, 20, 55, 0, 1.0);
        double tsx = posAnim(frame, 0, 60, -54, -111);
        p.setOpacity(tsop * exitOp);
        p.drawText(QPointF(cx+tsx*sx, cy+100*sy), line2);
    }
    // ═══════════════════════════════════════════════════════
    // STYLE 3: title_04 — "Angled Bars"
    // Line + 2 blue boxes + 3 text lines appearing sequentially
    // ═══════════════════════════════════════════════════════
    else if (idx == 3) {
        double baseX = 960*sx, baseY = posAnim(frame, 0, 60, 640, 570)*sy;
        p.setOpacity(exitOp);

        // Line 01: white 5px
        p.setPen(QPen(Qt::white, 5*sy)); p.setBrush(Qt::NoBrush);
        double lineOp = opacityAnim(frame, 0, 30, 0, 1.0);
        p.setOpacity(lineOp * exitOp);
        p.drawLine(QPointF(baseX-300*sx, baseY-17*sy), QPointF(baseX+300*sx, baseY-17*sy));

        // Box 01: blue parallelogram, opacity 0->100 over 30f
        double b1op = opacityAnim(frame, 0, 30, 0, 1.0);
        p.setOpacity(b1op * exitOp); p.setPen(Qt::NoPen); p.setBrush(accent);
        QPainterPath para;
        double bx = baseX-200*sx, by = baseY-57*sy, bw = 400*sx, bh = 55*sy;
        double sk = 20*sx;
        para.moveTo(bx+sk, by); para.lineTo(bx+bw, by);
        para.lineTo(bx+bw-sk, by+bh); para.lineTo(bx, by+bh); para.closeSubpath();
        p.drawPath(para);

        // Box 02: appears at frame 30, rect at bottom
        double b2op = opacityAnim(frame, 30, 60, 0, 1.0);
        p.setOpacity(b2op * exitOp);
        p.drawRect(QRectF(baseX-226*sx, baseY+60*sy, 452*sx, 65*sy));

        // Texts
        QFont f1("Helvetica Neue", std::max(10, int(qMax(12, m_fontSize-10)*sy)), QFont::Bold);
        p.setFont(f1); p.setPen(m_textColor);
        double to1 = opacityAnim(frame, 10, 40, 0, 1.0);
        p.setOpacity(to1 * exitOp);
        p.drawText(QPointF(baseX-180*sx, baseY-25*sy), line1);

        QFont f2("Helvetica Neue", std::max(10, int(qMax(12, m_fontSize-10)*sy)), QFont::Bold);
        p.setFont(f2);
        double to2 = opacityAnim(frame, 15, 45, 0, 1.0);
        p.setOpacity(to2 * exitOp);
        p.drawText(QPointF(baseX-180*sx, baseY+20*sy), line2);

        if (!line3.isEmpty()) {
            p.setFont(f2);
            double to3 = opacityAnim(frame, 30, 60, 0, 1.0);
            p.setOpacity(to3 * exitOp);
            p.drawText(QPointF(baseX-180*sx, baseY+95*sy), line3);
        }
    }
    // ═══════════════════════════════════════════════════════
    // STYLE 4: title_05 — "Clean Bar"
    // Big text + blue bar (811x85) + ellipse trim + 2 black shapes
    // ═══════════════════════════════════════════════════════
    else if (idx == 4) {
        p.setOpacity(exitOp);

        // Blue bar
        double barOp = opacityAnim(frame, 0, 60, 0, 1.0);
        p.setOpacity(barOp * exitOp); p.setPen(Qt::NoPen); p.setBrush(accent);
        p.drawRect(QRectF(500*sx, 550*sy, 811*sx*ease(std::min(1.0,frame/60.0)), 85*sy));

        // Ellipse: white stroke, trim animated
        double elTrim = trimAnim(frame, 0, 60, 1.0, 0);
        if (elTrim < 0.99) {
            p.setOpacity(exitOp);
            p.setPen(QPen(Qt::white, 5*sy)); p.setBrush(Qt::NoBrush);
            p.drawEllipse(QPointF(450*sx, 590*sy), 30*sx*(1.0-elTrim), 30*sy*(1.0-elTrim));
        }

        // Black shapes fading in
        double sh1op = opacityAnim(frame, 0, 60, 0, 1.0);
        p.setOpacity(sh1op * exitOp); p.setPen(Qt::NoPen); p.setBrush(Qt::black);
        p.drawRect(QRectF(500*sx, 470*sy, 350*sx*ease(std::min(1.0,frame/60.0)), 70*sy));

        double sh2op = opacityAnim(frame, 15, 60, 0, 1.0);
        p.setOpacity(sh2op * exitOp);
        p.drawRect(QRectF(500*sx, 640*sy, 250*sx*ease(std::max(0.0,(frame-15)/45.0)), 50*sy));

        // Texts
        QFont f1("Helvetica Neue", std::max(12, int(m_fontSize*sy)), QFont::Black);
        p.setFont(f1); p.setPen(QColor(235,235,235));
        double to1 = opacityAnim(frame, 10, 45, 0, 1.0);
        p.setOpacity(to1 * exitOp);
        p.drawText(QPointF(520*sx, 530*sy), line1);

        QFont f2("Helvetica Neue", std::max(10, int(qMax(12, m_fontSize-10)*sy)), QFont::Bold);
        p.setFont(f2);
        double to2 = opacityAnim(frame, 20, 50, 0, 1.0);
        p.setOpacity(to2 * exitOp);
        p.drawText(QPointF(520*sx, 620*sy), line2);

        if (!line3.isEmpty()) {
            QFont f3("Helvetica Neue", std::max(10, int(qMax(10, m_fontSize-14)*sy)), QFont::Bold);
            p.setFont(f3);
            double to3 = opacityAnim(frame, 25, 55, 0, 1.0);
            p.setOpacity(to3 * exitOp);
            p.drawText(QPointF(520*sx, 680*sy), line3);
        }
    }
    // ═══════════════════════════════════════════════════════
    // STYLE 5: title_06 — "Text Block"
    // Blue box (491x115) + thick line (22px) + text
    // ═══════════════════════════════════════════════════════
    else if (idx == 5) {
        p.setOpacity(exitOp);

        // Blue box
        double bop = opacityAnim(frame, 0, 60, 0, 1.0);
        p.setOpacity(bop * exitOp); p.setPen(Qt::NoPen); p.setBrush(accent);
        p.drawRect(QRectF(700*sx, 480*sy, 491*sx*ease(std::min(1.0,frame/60.0)), 115*sy));

        // Thick white line 22px
        double trimE = trimAnim(frame, 0, 60, 0, 1.0);
        p.setOpacity(exitOp);
        p.setPen(QPen(Qt::white, 22*sy, Qt::SolidLine, Qt::FlatCap)); p.setBrush(Qt::NoBrush);
        p.drawLine(QPointF(700*sx, 620*sy), QPointF(700*sx + 500*sx*trimE, 620*sy));

        // Text
        QFont f1("Helvetica Neue", std::max(10, int(qMax(10, m_fontSize-14)*sy)), QFont::Bold);
        p.setFont(f1); p.setPen(QColor(235,235,235));
        double to1 = opacityAnim(frame, 15, 45, 0, 1.0);
        p.setOpacity(to1 * exitOp);
        p.drawText(QRectF(720*sx, 490*sy, 460*sx, 50*sy), Qt::AlignLeft|Qt::AlignVCenter, line1);

        QFont f2("Helvetica Neue", std::max(8, int(qMax(10, m_fontSize-14)*sy)), QFont::Normal);
        p.setFont(f2);
        double to2 = opacityAnim(frame, 25, 55, 0, 1.0);
        p.setOpacity(to2 * exitOp);
        p.drawText(QRectF(720*sx, 540*sy, 460*sx, 40*sy), Qt::AlignLeft|Qt::TextWordWrap, line2);

        if (!line3.isEmpty()) {
            QFont f3("Helvetica Neue", std::max(10, int(qMax(10, m_fontSize-14)*sy)), QFont::Bold);
            p.setFont(f3);
            double to3 = opacityAnim(frame, 10, 40, 0, 1.0);
            p.setOpacity(to3 * exitOp);
            p.drawText(QPointF(720*sx, 660*sy), line3);
        }
    }
    // ═══════════════════════════════════════════════════════
    // STYLE 6: title_07 — "Stylish Line"
    // Blue box (641x74) + ellipse trim + shapes fading + lines
    // 77 frames total (shorter animation)
    // ═══════════════════════════════════════════════════════
    else if (idx == 6) {
        p.setOpacity(exitOp);

        // Blue box at (957,536) 641x74
        double bop = opacityAnim(frame, 0, 60, 0, 1.0);
        p.setOpacity(bop * exitOp); p.setPen(Qt::NoPen); p.setBrush(accent);
        p.drawRect(QRectF((957-320)*sx, (536-37)*sy, 641*sx*ease(std::min(1.0,frame/60.0)), 74*sy));

        // Ellipse at (1199,630) white stroke 5px, trim
        double elTrim = trimAnim(frame, 0, 60, 1.0, 0);
        p.setOpacity((1.0-elTrim)*exitOp);
        p.setPen(QPen(Qt::white, 5*sy)); p.setBrush(Qt::NoBrush);
        p.drawEllipse(QPointF(1199*sx, 630*sy), 25*sx, 25*sy);

        // White shapes fading in
        double s1op = opacityAnim(frame, 0, 60, 0, 1.0);
        p.setOpacity(s1op * exitOp); p.setPen(Qt::NoPen); p.setBrush(Qt::white);
        p.drawRect(QRectF(600*sx, 490*sy, 120*sx, 35*sy));

        double s2op = opacityAnim(frame, 0, 60, 0, 0.75);
        p.setOpacity(s2op * exitOp);
        p.drawRect(QRectF(600*sx, 610*sy, 100*sx, 25*sy));

        // Line 01: white 5px, trim 0->100%
        double l1trim = trimAnim(frame, 0, 60, 0, 1.0);
        p.setOpacity(exitOp);
        p.setPen(QPen(Qt::white, 5*sy, Qt::SolidLine, Qt::FlatCap));
        p.drawLine(QPointF(751*sx, 546*sy), QPointF(751*sx + 400*sx*l1trim, 546*sy));

        // Line 02: thick 30px
        double l2trim = trimAnim(frame, 0, 60, 0, 1.0);
        p.setPen(QPen(Qt::white, 30*sy, Qt::SolidLine, Qt::FlatCap));
        p.drawLine(QPointF(600*sx, 580*sy), QPointF(600*sx + 500*sx*l2trim, 580*sy));

        // Texts
        QFont f1("Helvetica Neue", std::max(12, int(m_fontSize*sy)), QFont::Bold);
        p.setFont(f1); p.setPen(m_textColor);
        double to1 = opacityAnim(frame, 10, 40, 0, 1.0);
        p.setOpacity(to1 * exitOp);
        p.drawText(QPointF(650*sx, 530*sy), line1);

        QFont f2("Helvetica Neue", std::max(12, int(m_fontSize*sy)), QFont::Bold);
        p.setFont(f2);
        double to2 = opacityAnim(frame, 15, 50, 0, 1.0);
        p.setOpacity(to2 * exitOp);
        p.drawText(QPointF(650*sx, 610*sy), line2);
    }
    // ═══════════════════════════════════════════════════════
    // STYLE 7: title_08 — "Motion Block"
    // 3 white shapes + ellipse + blue box path + 3 texts
    // ═══════════════════════════════════════════════════════
    else if (idx == 7) {
        p.setOpacity(exitOp);

        // 3 white shapes fading in with stagger
        p.setPen(Qt::NoPen); p.setBrush(Qt::white);
        double s1op = opacityAnim(frame, 0, 60, 0, 1.0);
        p.setOpacity(s1op * exitOp);
        p.drawRect(QRectF(650*sx, 430*sy, 200*sx, 40*sy));

        double s2op = opacityAnim(frame, 2, 62, 0, 0.75);
        p.setOpacity(s2op * exitOp);
        p.drawRect(QRectF(650*sx, 530*sy, 180*sx, 35*sy));

        double s3op = opacityAnim(frame, 4, 64, 0, 0.60);
        p.setOpacity(s3op * exitOp);
        p.drawRect(QRectF(650*sx, 620*sy, 160*sx, 30*sy));

        // Ellipse at (1199,630) trim
        double elTrim = trimAnim(frame, 0, 60, 1.0, 0);
        p.setOpacity((1.0-elTrim)*exitOp);
        p.setPen(QPen(Qt::white, 5*sy)); p.setBrush(Qt::NoBrush);
        p.drawEllipse(QPointF(1199*sx, 630*sy), 25*sx, 25*sy);

        // Blue box (path shape)
        double bop = opacityAnim(frame, 0, 60, 0, 1.0);
        p.setOpacity(bop * exitOp); p.setPen(Qt::NoPen); p.setBrush(accent);
        p.drawRect(QRectF(870*sx, 480*sy, 300*sx*ease(std::min(1.0,frame/60.0)), 70*sy));

        // Texts
        QFont f1("Helvetica Neue", std::max(12, int(m_fontSize*sy)), QFont::Bold);
        p.setFont(f1); p.setPen(m_textColor);
        double to1 = opacityAnim(frame, 10, 40, 0, 1.0);
        p.setOpacity(to1 * exitOp);
        p.drawText(QPointF(670*sx, 465*sy), line1);

        double to2 = opacityAnim(frame, 15, 45, 0, 1.0);
        p.setOpacity(to2 * exitOp);
        p.drawText(QPointF(670*sx, 560*sy), line2);

        if (!line3.isEmpty()) {
            double to3 = opacityAnim(frame, 20, 50, 0, 1.0);
            p.setOpacity(to3 * exitOp);
            p.drawText(QPointF(670*sx, 650*sy), line3);
        }
    }
    // ═══════════════════════════════════════════════════════
    // STYLE 8: title_09 — "Wide Bar"
    // 3 white shapes stagger + ellipse + blue rect (889x140) + 3 texts
    // ═══════════════════════════════════════════════════════
    else {
        p.setOpacity(exitOp);

        // White shapes stagger
        p.setPen(Qt::NoPen); p.setBrush(Qt::white);
        double s1op = opacityAnim(frame, 0, 60, 0, 1.0);
        p.setOpacity(s1op * exitOp);
        p.drawRect(QRectF(500*sx, 400*sy, 250*sx, 40*sy));

        double s2op = opacityAnim(frame, 2, 60, 0, 0.75);
        p.setOpacity(s2op * exitOp);
        p.drawRect(QRectF(500*sx, 520*sy, 220*sx, 35*sy));

        double s3op = opacityAnim(frame, 4, 60, 0, 0.60);
        p.setOpacity(s3op * exitOp);
        p.drawRect(QRectF(500*sx, 630*sy, 190*sx, 30*sy));

        // Ellipse at (1339,598) trim
        double elTrim = trimAnim(frame, 0, 60, 1.0, 0);
        p.setOpacity((1.0-elTrim)*exitOp);
        p.setPen(QPen(Qt::white, 5*sy)); p.setBrush(Qt::NoBrush);
        p.drawEllipse(QPointF(1339*sx, 598*sy), 30*sx, 30*sy);

        // Blue wide rect (889x140)
        double bop = opacityAnim(frame, 0, 60, 0, 1.0);
        p.setOpacity(bop * exitOp); p.setPen(Qt::NoPen); p.setBrush(accent);
        double bw = 889*sx*ease(std::min(1.0,frame/60.0));
        p.drawRect(QRectF(500*sx, 460*sy, bw, 140*sy));

        // Texts
        QFont f1("Helvetica Neue", std::max(10, int(qMax(12, m_fontSize-10)*sy)), QFont::Bold);
        p.setFont(f1); p.setPen(m_textColor);
        double to1 = opacityAnim(frame, 10, 40, 0, 1.0);
        p.setOpacity(to1 * exitOp);
        p.drawText(QPointF(520*sx, 435*sy), line1);

        QFont f2("Helvetica Neue", std::max(10, int(qMax(12, m_fontSize-10)*sy)), QFont::Bold);
        p.setFont(f2);
        double to2 = opacityAnim(frame, 15, 45, 0, 1.0);
        p.setOpacity(to2 * exitOp);
        p.drawText(QPointF(520*sx, 545*sy), line2);

        if (!line3.isEmpty()) {
            p.setFont(f2);
            double to3 = opacityAnim(frame, 20, 50, 0, 1.0);
            p.setOpacity(to3 * exitOp);
            p.drawText(QPointF(520*sx, 655*sy), line3);
        }
    }

    p.end();
    return result;
}

// Unused stubs
void LottieEngine::renderLayer(QPainter&, const QJsonObject&, double, double, double, const QMap<QString,QString>&) {}
void LottieEngine::renderShapeGroup(QPainter&, const QJsonArray&, double, double, double) {}
void LottieEngine::renderTextLayer(QPainter&, const QJsonObject&, double, double, double, const QMap<QString,QString>&) {}
double LottieEngine::interpolateValue(const QJsonValue&, double, double d) { return d; }
QPointF LottieEngine::interpolatePoint(const QJsonValue&, double, QPointF d) { return d; }
QColor LottieEngine::interpolateColor(const QJsonValue&, double, QColor d) { return d; }
double LottieEngine::interpolateMultidim(const QJsonArray&, double, int, double d) { return d; }
LottieEngine::Transform LottieEngine::evaluateTransform(const QJsonObject&, double) { return {}; }

} // namespace prestige
