#pragma once

// ============================================================
// Prestige AI — Runtime SDK Loader
// Loads broadcast SDK libraries at runtime via dlopen/LoadLibrary.
// The application binary is compiled ONCE and works everywhere.
// Users only need to install hardware + driver.
// ============================================================

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariantMap>
#include <QVariantList>

namespace prestige {

class RuntimeLoader : public QObject {
    Q_OBJECT

public:
    explicit RuntimeLoader(QObject* parent = nullptr);
    ~RuntimeLoader() override;

    // Probe all SDKs — call once at startup
    void probeAll();

    // Individual SDK status
    bool isDeckLinkAvailable() const { return m_deckLinkLoaded; }
    bool isNdiAvailable() const { return m_ndiLoaded; }
    bool isMagewellAvailable() const { return m_magewellLoaded; }
    bool isAjaAvailable() const { return m_ajaDriverPresent; }

    // Status for UI
    Q_INVOKABLE QVariantList sdkStatus() const;

    // Symbol lookup for hardware verification
    void* magewellSymbol(const char* name) const;

    // Function pointers loaded at runtime
    // DeckLink
    void* deckLinkCreateIterator() const;

    // NDI
    void* ndiLib() const { return m_ndiHandle; }

private:
    bool loadDeckLink();
    bool loadNdi();
    bool loadMagewell();
    bool probeAjaDriver();

    void* m_deckLinkHandle  = nullptr;
    void* m_ndiHandle       = nullptr;
    void* m_magewellHandle  = nullptr;

    bool m_deckLinkLoaded   = false;
    bool m_ndiLoaded        = false;
    bool m_magewellLoaded   = false;
    bool m_ajaDriverPresent = false;

    // Cached function pointers
    void* m_fnDeckLinkCreateIterator = nullptr;
};

} // namespace prestige
