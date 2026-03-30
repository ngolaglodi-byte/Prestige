project(prestige_broadcast_sdk LANGUAGES CXX)

# ════════════════════════════════════════════════════════════
# ALL SDK CODE IS ALWAYS COMPILED.
# No #ifdef guards. No -DPRESTIGE_ENABLE_* flags needed.
# SDKs are loaded at RUNTIME via dlopen/LoadLibrary.
# Users only install hardware + driver. No recompilation.
# ════════════════════════════════════════════════════════════

set(SDK_SOURCES
    RuntimeLoader.cpp
    HardwareScanner.cpp
    DecklinkCapture.cpp
    NdiCapture.cpp
    AjaCapture.cpp
    MagewellCapture.cpp
)

set(SDK_HEADERS
    RuntimeLoader.h
    HardwareScanner.h
    DecklinkCapture.h
    NdiCapture.h
    AjaCapture.h
    MagewellCapture.h
)

add_library(prestige_broadcast_sdk STATIC ${SDK_SOURCES} ${SDK_HEADERS})

target_include_directories(prestige_broadcast_sdk PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/third_party/decklink
)

target_link_libraries(prestige_broadcast_sdk PUBLIC
    Qt6::Core
    Qt6::Gui
    Qt6::Multimedia
    prestige_shared
)

# dlopen support (always available on macOS/Linux, kernel32 on Windows)
if(UNIX)
    target_link_libraries(prestige_broadcast_sdk PRIVATE ${CMAKE_DL_LIBS})
endif()

# ── NDI SDK (compile-time optional, runtime always probed) ──
# If NDI headers are present at build time, we use the real types.
# Otherwise we use our stub types and runtime-load everything.
find_path(NDI_INCLUDE_DIR Processing.NDI.Lib.h
    PATHS
        "$ENV{NDI_SDK_DIR}/include"
        "/Library/NDI SDK for Apple/include"
        "/opt/homebrew/include"
        "/usr/local/include"
        "C:/Program Files/NDI/NDI 6 SDK/Include"
    NO_DEFAULT_PATH
)

if(NDI_INCLUDE_DIR)
    target_include_directories(prestige_broadcast_sdk PRIVATE ${NDI_INCLUDE_DIR})
    target_compile_definitions(prestige_broadcast_sdk PUBLIC PRESTIGE_HAVE_NDI)
    message(STATUS "NDI SDK headers found: ${NDI_INCLUDE_DIR}")

    # Try to find the library for link-time binding too
    find_library(NDI_LIBRARY NAMES ndi Processing.NDI.Lib.x64
        PATHS
            "/Library/NDI SDK for Apple/lib/macOS"
            "/opt/homebrew/lib"
            "/usr/local/lib"
            "C:/Program Files/NDI/NDI 6 SDK/Lib/x64"
        NO_DEFAULT_PATH
    )
    if(NDI_LIBRARY)
        target_link_libraries(prestige_broadcast_sdk PRIVATE ${NDI_LIBRARY})
        message(STATUS "NDI SDK library found: ${NDI_LIBRARY}")
    endif()
else()
    message(STATUS "NDI SDK headers not found — NDI will be runtime-loaded only")
endif()
