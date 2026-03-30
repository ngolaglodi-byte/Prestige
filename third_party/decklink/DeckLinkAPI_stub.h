#pragma once

// ============================================================
// Prestige AI — DeckLink API Type Definitions (stub header)
// These are the minimal type definitions needed to compile
// DeckLink integration code. The actual implementation is
// loaded at runtime via dlopen/LoadLibrary from Blackmagic's
// installed driver.
//
// This file does NOT contain Blackmagic proprietary code.
// It only defines the public COM interface IDs and types
// that are part of the public API specification.
// ============================================================

#include <cstdint>

#ifdef __APPLE__
#include <CoreFoundation/CoreFoundation.h>
#endif

// ── COM base types ─────────────────────────────────────────
#ifndef _DECKLINK_TYPES_
#define _DECKLINK_TYPES_

typedef int32_t  HRESULT;
typedef uint32_t REFIID;
typedef uint32_t ULONG;

#define S_OK     0
#define S_FALSE  1
#define E_FAIL   ((HRESULT)0x80004005)

// Minimal IUnknown-like base
class IUnknown {
public:
    virtual HRESULT QueryInterface(REFIID iid, void** ppv) = 0;
    virtual ULONG   AddRef() = 0;
    virtual ULONG   Release() = 0;
    virtual ~IUnknown() = default;
};

// ── Display modes ──────────────────────────────────────────
typedef uint32_t BMDDisplayMode;
#define bmdModeHD720p50      0x68703530
#define bmdMode1080p25       0x48703235
#define bmdMode1080p50       0x48703530
#define bmdMode1080i50       0x48693530
#define bmdMode4K2160p25     0x346B3235
#define bmdMode4K2160p50     0x346B3530

typedef uint32_t BMDPixelFormat;
#define bmdFormat8BitYUV     0x32767579
#define bmdFormat8BitBGRA    0x42475241
#define bmdFormat10BitYUV    0x76323130

typedef uint32_t BMDVideoInputFlags;
#define bmdVideoInputFlagDefault 0

// ── DeckLink interfaces ────────────────────────────────────
// These match the real DeckLink API signature exactly.
// Implementation is loaded at runtime from the framework.

class IDeckLink : public IUnknown {
public:
#ifdef __APPLE__
    virtual HRESULT GetDisplayName(CFStringRef* displayName) = 0;
#else
    virtual HRESULT GetDisplayName(const char** displayName) = 0;
#endif
};

class IDeckLinkInput : public IUnknown {
public:
    virtual HRESULT EnableVideoInput(BMDDisplayMode mode, BMDPixelFormat format, BMDVideoInputFlags flags) = 0;
    virtual HRESULT DisableVideoInput() = 0;
    virtual HRESULT StartStreams() = 0;
    virtual HRESULT StopStreams() = 0;
};

class IDeckLinkIterator : public IUnknown {
public:
    virtual HRESULT Next(IDeckLink** deckLink) = 0;
};

class IDeckLinkAPIInformation : public IUnknown {
public:
};

// Interface IDs (used by QueryInterface)
#define IID_IDeckLinkInput          0x01
#define IID_IDeckLinkAPIInformation 0x02

#endif // _DECKLINK_TYPES_
