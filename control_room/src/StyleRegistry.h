#pragma once

// ============================================================
// Prestige AI — Style Registry
// Metadata for all 20 overlay styles
// ============================================================

#include <QObject>
#include <QString>
#include <QColor>
#include <QList>
#include <QVariantList>

namespace prestige {

struct OverlayStyleInfo {
    QString id;
    QString displayName;
    QString category;       // "info", "sports", "politique", "premium", "special"
    QString reference;      // "BFM TV", "CNN", etc.
    QString qmlFile;
    QColor  defaultAccent;
    QString description;
    bool    hasLowerThird;
    bool    hasBreakingMode;
};

class StyleRegistry : public QObject {
    Q_OBJECT

public:
    explicit StyleRegistry(QObject* parent = nullptr);

    static QList<OverlayStyleInfo> allStyles();
    static OverlayStyleInfo findById(const QString& id);

    Q_INVOKABLE QVariantList stylesForCategory(const QString& category) const;
    Q_INVOKABLE QVariantList allStylesList() const;
    Q_INVOKABLE QString qmlFileFor(const QString& id) const;
    Q_INVOKABLE QColor defaultAccentFor(const QString& id) const;
};

} // namespace prestige
