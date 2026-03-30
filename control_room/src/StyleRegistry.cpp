// ============================================================
// Prestige AI — Style Registry Implementation
// ============================================================

#include "StyleRegistry.h"
#include <QVariantMap>

namespace prestige {

static QList<OverlayStyleInfo> s_styles = {
    // ── Info ──
    {"bfm",          "BFM TV",           "info",      "BFM TV",           "styles/StyleBFM.qml",           QColor("#E30613"), "Bandeau noir, barre rouge gauche",      true,  false},
    {"lci",          "LCI",              "info",      "LCI",              "styles/StyleLCI.qml",           QColor("#FF6D00"), "Dégradé sombre, triangle orange",       true,  false},
    {"france2",      "France 2",         "info",      "France 2",         "styles/StyleFrance2.qml",       QColor("#003189"), "Fond blanc, accent bleu France",        true,  false},
    {"france24",     "France 24",        "info",      "France 24",        "styles/StyleFrance24.qml",      QColor("#D4001A"), "Double ligne rouge/bleu, wipe",         true,  false},
    {"cnn",          "CNN",              "info",      "CNN International","styles/StyleCNN.qml",           QColor("#CC0000"), "Fond rouge plein, diagonale",           true,  true },
    {"bbc",          "BBC News",         "info",      "BBC News",         "styles/StyleBBC.qml",           QColor("#BB1919"), "Rouge BBC, clip-rect reveal",           true,  false},
    {"skynews",      "Sky News",         "info",      "Sky News",         "styles/StyleSkyNews.qml",       QColor("#00A0DC"), "Bleu Sky, slide depuis droite",         true,  false},
    {"aljazeera",    "Al Jazeera",       "info",      "Al Jazeera English","styles/StyleAljazeeraEn.qml",  QColor("#D4A017"), "Bordeaux, texte or",                    true,  false},

    // ── Sports ──
    {"sports",       "Sports",           "sports",    "Générique",        "styles/StyleSports.qml",        QColor("#00FF88"), "Noir + badge numéro, accent couleur",   false, false},
    {"football",     "Football TV",      "sports",    "Football",         "styles/StyleFootball.qml",      QColor("#FFFF00"), "Design pelouse, numéro fantôme",        false, false},
    {"olympics",     "Olympiques",       "sports",    "JO",               "styles/StyleOlympics.qml",      QColor("#0033A0"), "Fond blanc, anneaux olympiques",        false, false},

    // ── Politique ──
    {"politique",    "Débats politiques","politique", "Politique",        "styles/StylePolitique.qml",     QColor("#1B2A4A"), "Bleu nuit, drapeau, iris reveal",       false, false},
    {"gouvernement", "Officiel gouv.",   "politique", "Gouvernement",     "styles/StyleGouvernement.qml",  QColor("#002395"), "Blanc officiel, tricolore, sceau RF",   false, false},

    // ── Premium ──
    {"cinema",       "Cinéma / Festival","premium",  "Cannes/Venise",    "styles/StyleCinema.qml",        QColor("#D4AF37"), "Noir absolu, texte or, fade lent",      true,  false},
    {"luxury",       "Luxury / Prestige","premium",  "Haut de gamme",    "styles/StyleLuxury.qml",        QColor("#C0C0C0"), "Minimaliste luxe, argent, léger",       false, false},
    {"tech",         "Tech / Innovation","premium",  "Tech moderne",     "styles/StyleTechModern.qml",    QColor("#00E5FF"), "Bleu néon, monospace, glitch-in",       false, false},
    {"minimal",      "Minimaliste",      "premium",  "Épuré",            "styles/StyleMinimal.qml",       QColor("#FFFFFF"), "Sans fond, ligne fine, fade pur",        false, false},

    // ── Spéciaux ──
    {"dual",         "Deux personnes",   "special",  "Interview/Débat",  "styles/StyleDualPerson.qml",    QColor("#5B4FDB"), "Double bloc gauche/droite",              false, false},
    {"fullscreen",   "Plein écran",      "special",  "Reportage",        "styles/StyleFullscreen.qml",    QColor("#5B4FDB"), "Centré bas, gradient, médaillon",        false, false},
    {"breaking",     "Breaking News",    "special",  "Alerte info",      "styles/StyleBreaking.qml",      QColor("#CC0000"), "Bannière rouge + ticker défilant",       true,  true },
};

StyleRegistry::StyleRegistry(QObject* parent)
    : QObject(parent)
{
}

QList<OverlayStyleInfo> StyleRegistry::allStyles()
{
    return s_styles;
}

OverlayStyleInfo StyleRegistry::findById(const QString& id)
{
    for (const auto& s : s_styles) {
        if (s.id == id) return s;
    }
    return {};
}

QVariantList StyleRegistry::stylesForCategory(const QString& category) const
{
    QVariantList result;
    for (const auto& s : s_styles) {
        if (category.isEmpty() || category == "all" || s.category == category) {
            QVariantMap map;
            map["id"] = s.id;
            map["displayName"] = s.displayName;
            map["category"] = s.category;
            map["reference"] = s.reference;
            map["accentColor"] = s.defaultAccent.name();
            map["description"] = s.description;
            result.append(map);
        }
    }
    return result;
}

QVariantList StyleRegistry::allStylesList() const
{
    return stylesForCategory(QString());
}

QString StyleRegistry::qmlFileFor(const QString& id) const
{
    auto info = findById(id);
    return info.qmlFile;
}

QColor StyleRegistry::defaultAccentFor(const QString& id) const
{
    auto info = findById(id);
    return info.defaultAccent;
}

} // namespace prestige
