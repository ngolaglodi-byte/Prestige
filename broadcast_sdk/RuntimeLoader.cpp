// ============================================================
// Prestige AI — Runtime SDK Loader Implementation
// Uses dlopen (macOS/Linux) or LoadLibrary (Windows)
// ============================================================

#include "RuntimeLoader.h"
#include <QDebug>
#include <QFile>
#include <QVariantMap>

#ifdef Q_OS_MAC
#include <dlfcn.h>
#elif defined(Q_OS_WIN)
#include <windows.h>
#endif

namespace prestige {

RuntimeLoader::RuntimeLoader(QObject* parent)
    : QObject(parent)
{
}

RuntimeLoader::~RuntimeLoader()
{
#ifdef Q_OS_MAC
    if (m_deckLinkHandle) dlclose(m_deckLinkHandle);
    if (m_ndiHandle)      dlclose(m_ndiHandle);
    if (m_magewellHandle) dlclose(m_magewellHandle);
#elif defined(Q_OS_WIN)
    if (m_deckLinkHandle) FreeLibrary((HMODULE)m_deckLinkHandle);
    if (m_ndiHandle)      FreeLibrary((HMODULE)m_ndiHandle);
    if (m_magewellHandle) FreeLibrary((HMODULE)m_magewellHandle);
#endif
}

void RuntimeLoader::probeAll()
{
    qInfo() << "[RuntimeLoader] Probing installed broadcast SDKs...";

    m_deckLinkLoaded   = loadDeckLink();
    m_ndiLoaded        = loadNdi();
    m_magewellLoaded   = loadMagewell();
    m_ajaDriverPresent = probeAjaDriver();

    qInfo() << "[RuntimeLoader] Results:"
            << "DeckLink=" << (m_deckLinkLoaded ? "YES" : "no")
            << "NDI=" << (m_ndiLoaded ? "YES" : "no")
            << "Magewell=" << (m_magewellLoaded ? "YES" : "no")
            << "AJA=" << (m_ajaDriverPresent ? "DRIVER" : "no");
}

// ── DeckLink ───────────────────────────────────────────────

bool RuntimeLoader::loadDeckLink()
{
#ifdef Q_OS_MAC
    // Blackmagic Desktop Video installs to:
    const char* paths[] = {
        "/Library/Frameworks/DeckLinkAPI.framework/DeckLinkAPI",
        "/Library/Frameworks/DeckLinkAPI.framework/Versions/A/DeckLinkAPI",
        nullptr
    };

    for (int i = 0; paths[i]; ++i) {
        m_deckLinkHandle = dlopen(paths[i], RTLD_LAZY);
        if (m_deckLinkHandle) {
            m_fnDeckLinkCreateIterator = dlsym(m_deckLinkHandle, "CreateDeckLinkIteratorInstance");
            if (m_fnDeckLinkCreateIterator) {
                qInfo() << "[RuntimeLoader] DeckLink SDK loaded from" << paths[i];
                return true;
            }
            dlclose(m_deckLinkHandle);
            m_deckLinkHandle = nullptr;
        }
    }

    qInfo() << "[RuntimeLoader] DeckLink SDK not found — install Blackmagic Desktop Video";

#elif defined(Q_OS_WIN)
    // Windows: DeckLink COM is registered system-wide when driver is installed
    // We check by trying to create a COM iterator
    m_deckLinkHandle = LoadLibraryA("DeckLinkAPI.dll");
    if (!m_deckLinkHandle)
        m_deckLinkHandle = LoadLibraryA("DeckLinkAPI_64.dll");
    if (m_deckLinkHandle) {
        qInfo() << "[RuntimeLoader] DeckLink SDK loaded (Windows)";
        return true;
    }
#endif

    return false;
}

void* RuntimeLoader::deckLinkCreateIterator() const
{
    if (!m_fnDeckLinkCreateIterator) return nullptr;

    // Call the function pointer: IDeckLinkIterator* (*)(void)
    typedef void* (*CreateIteratorFunc)();
    auto fn = reinterpret_cast<CreateIteratorFunc>(m_fnDeckLinkCreateIterator);
    return fn();
}

// ── NDI ────────────────────────────────────────────────────

bool RuntimeLoader::loadNdi()
{
#ifdef Q_OS_MAC
    const char* paths[] = {
        "/Library/NDI SDK for Apple/lib/macOS/libndi.dylib",
        "/Library/NDI SDK for Apple/lib/macOS/libndi.4.dylib",
        "/opt/homebrew/lib/libndi.dylib",
        "/usr/local/lib/libndi.dylib",
        nullptr
    };

    for (int i = 0; paths[i]; ++i) {
        m_ndiHandle = dlopen(paths[i], RTLD_LAZY);
        if (m_ndiHandle) {
            // Verify key function exists
            if (dlsym(m_ndiHandle, "NDIlib_initialize")) {
                qInfo() << "[RuntimeLoader] NDI SDK loaded from" << paths[i];
                return true;
            }
            dlclose(m_ndiHandle);
            m_ndiHandle = nullptr;
        }
    }

    qInfo() << "[RuntimeLoader] NDI SDK not found — install with: brew install libndi";

#elif defined(Q_OS_WIN)
    // Windows: NDI runtime is typically in PATH or Program Files
    const char* paths[] = {
        "Processing.NDI.Lib.x64.dll",
        "C:/Program Files/NDI/NDI 6 SDK/Bin/x64/Processing.NDI.Lib.x64.dll",
        "C:/Program Files/NDI/NDI 5 SDK/Bin/x64/Processing.NDI.Lib.x64.dll",
        nullptr
    };

    for (int i = 0; paths[i]; ++i) {
        m_ndiHandle = LoadLibraryA(paths[i]);
        if (m_ndiHandle) {
            qInfo() << "[RuntimeLoader] NDI SDK loaded (Windows)";
            return true;
        }
    }
#endif

    return false;
}

// ── Magewell ───────────────────────────────────────────────

bool RuntimeLoader::loadMagewell()
{
#ifdef Q_OS_MAC
    const char* paths[] = {
        "/Library/Frameworks/MWCapture.framework/MWCapture",
        "/usr/local/lib/libMWCapture.dylib",
        nullptr
    };

    for (int i = 0; paths[i]; ++i) {
        m_magewellHandle = dlopen(paths[i], RTLD_LAZY);
        if (m_magewellHandle) {
            if (dlsym(m_magewellHandle, "MWGetChannelCount")) {
                qInfo() << "[RuntimeLoader] Magewell SDK loaded from" << paths[i];
                return true;
            }
            dlclose(m_magewellHandle);
            m_magewellHandle = nullptr;
        }
    }

    qInfo() << "[RuntimeLoader] Magewell SDK not found — download from magewell.com/sdk";

#elif defined(Q_OS_WIN)
    m_magewellHandle = LoadLibraryA("LibMWCapture.dll");
    if (m_magewellHandle) {
        qInfo() << "[RuntimeLoader] Magewell SDK loaded (Windows)";
        return true;
    }
#endif

    return false;
}

// ── AJA ────────────────────────────────────────────────────

bool RuntimeLoader::probeAjaDriver()
{
    // AJA doesn't have a simple shared library to dlopen.
    // We check if the kernel extension / driver is present.
#ifdef Q_OS_MAC
    if (QFile::exists("/Library/Extensions/AJA_NTV2.kext") ||
        QFile::exists("/System/Library/Extensions/AJA_NTV2.kext")) {
        qInfo() << "[RuntimeLoader] AJA driver detected (kext)";
        return true;
    }
#elif defined(Q_OS_WIN)
    // Check Windows device manager for AJA devices
    if (QFile::exists("C:/Program Files/AJA/NTV2/bin/ajantv2.dll")) {
        return true;
    }
#endif

    qInfo() << "[RuntimeLoader] AJA driver not found";
    return false;
}

// ── Status for UI ──────────────────────────────────────────

QVariantList RuntimeLoader::sdkStatus() const
{
    QVariantList list;

    auto add = [&](const QString& name, bool loaded, const QString& install) {
        QVariantMap m;
        m["name"] = name;
        m["installed"] = loaded;
        m["installHint"] = install;
        list.append(m);
    };

    add("Blackmagic DeckLink", m_deckLinkLoaded,
        "Installez 'Blackmagic Desktop Video' depuis blackmagicdesign.com");
    add("NewTek NDI", m_ndiLoaded,
        "Exécutez: brew install libndi (macOS) ou installez NDI Tools (Windows)");
    add("Magewell Pro Capture", m_magewellLoaded,
        "Téléchargez le SDK depuis magewell.com/sdk");
    add("AJA NTV2", m_ajaDriverPresent,
        "Installez le pilote AJA depuis aja.com/software");

    return list;
}

} // namespace prestige
