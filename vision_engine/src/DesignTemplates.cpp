// ============================================================
// Prestige AI — Design Templates (STUB — removed)
// All animations now handled by Lottie engine (Samsung rlottie)
// ============================================================

#include "DesignTemplates.h"
#include <QPainter>

namespace prestige {

DesignRegistry& DesignRegistry::instance() {
    static DesignRegistry reg;
    return reg;
}

void DesignRegistry::registerAll() {}
void DesignRegistry::addDesign(DesignTemplate&&) {}

std::vector<DesignTemplate> DesignRegistry::getDesigns(const QString&) const { return {}; }
const DesignTemplate* DesignRegistry::getDesign(const QString&, const QString&) const { return nullptr; }
QStringList DesignRegistry::getDesignIds(const QString&) const { return {}; }
QStringList DesignRegistry::getDesignNames(const QString&) const { return {}; }

void DesignRegistry::render(QPainter&, const QString&, const QString&,
                             const QRectF&, double, double,
                             const QColor&, const QString&,
                             const QString&, const QFont&) {}

} // namespace prestige
