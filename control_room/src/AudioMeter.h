#pragma once
#include <QObject>
#include <QAudioSource>
#include <QIODevice>
#include <QTimer>

namespace prestige {
class AudioMeter : public QObject {
    Q_OBJECT
    Q_PROPERTY(double levelL READ levelL NOTIFY levelChanged)
    Q_PROPERTY(double levelR READ levelR NOTIFY levelChanged)
    Q_PROPERTY(double peakL READ peakL NOTIFY levelChanged)
    Q_PROPERTY(double peakR READ peakR NOTIFY levelChanged)
    Q_PROPERTY(bool active READ isActive NOTIFY activeChanged)
public:
    explicit AudioMeter(QObject* parent = nullptr);
    ~AudioMeter() override;

    double levelL() const { return m_levelL; }
    double levelR() const { return m_levelR; }
    double peakL() const { return m_peakL; }
    double peakR() const { return m_peakR; }
    bool isActive() const { return m_active; }

    Q_INVOKABLE void start();
    Q_INVOKABLE void stop();
signals:
    void levelChanged();
    void activeChanged();
private:
    void processAudio();
    QAudioSource* m_source = nullptr;
    QIODevice* m_device = nullptr;
    QTimer* m_timer = nullptr;
    double m_levelL = 0, m_levelR = 0;
    double m_peakL = 0, m_peakR = 0;
    bool m_active = false;
};
} // namespace prestige
