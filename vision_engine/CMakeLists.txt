project(prestige_vision LANGUAGES CXX)

set(VISION_SOURCES
    src/main.cpp
    src/VideoCapture.cpp
    src/FrameSender.cpp
    src/Compositor.cpp
    src/OverlayRenderer.cpp
    src/ZmqSubscriber.cpp
    src/VideoEncoder.cpp
    src/OutputRouter.cpp
    src/PreviewSender.cpp
    src/VirtualStudio.cpp
    src/BroadcastEffects.cpp
    src/GpuEffects.cpp
    src/DesignTemplates.cpp
    # AE Effects Engine (220+ After Effects effects)
    src/AEEasing.cpp
    src/AEGpuEffects.cpp
    src/AEBroadcastEffects.cpp
    # 3D PBR Studio Renderer
    src/Studio3DRenderer.cpp
    # Cinematic Overlay Animations
    src/CinematicOverlays.cpp
    # Lottie Animation Engine (AE title overlays)
    src/LottieEngine.cpp
)

set(VISION_HEADERS
    src/VideoCapture.h
    src/FrameSender.h
    src/Compositor.h
    src/OverlayRenderer.h
    src/ZmqSubscriber.h
    src/VideoEncoder.h
    src/OutputRouter.h
    src/PreviewSender.h
    src/VirtualStudio.h
    src/BroadcastEffects.h
    src/GpuEffects.h
    src/DesignTemplates.h
    src/AEEasing.h
    src/Studio3DRenderer.h
    src/CinematicOverlays.h
    src/LottieEngine.h
)

qt_add_executable(prestige_vision WIN32 ${VISION_SOURCES} ${VISION_HEADERS})

# ── QML module ──────────────────────────────────────────────
qt_add_qml_module(prestige_vision
    URI PrestigeVision
    VERSION 1.0
    QML_FILES
        qml/OverlayNameplate.qml
        qml/LowerThird.qml
        qml/TransitionEffect.qml
        qml/OverlayStyleEngine.qml
        qml/styles/StyleBFM.qml
        qml/styles/StyleLCI.qml
        qml/styles/StyleFrance2.qml
        qml/styles/StyleFrance24.qml
        qml/styles/StyleCNN.qml
        qml/styles/StyleBBC.qml
        qml/styles/StyleSkyNews.qml
        qml/styles/StyleAljazeeraEn.qml
        qml/styles/StyleSports.qml
        qml/styles/StyleFootball.qml
        qml/styles/StyleOlympics.qml
        qml/styles/StylePolitique.qml
        qml/styles/StyleGouvernement.qml
        qml/styles/StyleCinema.qml
        qml/styles/StyleLuxury.qml
        qml/styles/StyleTechModern.qml
        qml/styles/StyleMinimal.qml
        qml/styles/StyleDualPerson.qml
        qml/styles/StyleFullscreen.qml
        qml/styles/StyleBreaking.qml
)

target_link_libraries(prestige_vision PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Qml
    Qt6::Quick
    Qt6::QuickControls2
    Qt6::Multimedia
    Qt6::OpenGL
    Qt6::Network
    prestige_shared
    prestige_broadcast_sdk
    prestige_ai_module
)

# ── Qt 3D (PBR Virtual Studio — optional) ────────────────────
find_package(Qt6 REQUIRED COMPONENTS 3DCore 3DRender 3DExtras QUIET)
if(TARGET Qt6::3DCore AND TARGET Qt6::3DRender AND TARGET Qt6::3DExtras)
    target_link_libraries(prestige_vision PRIVATE
        Qt6::3DCore
        Qt6::3DRender
        Qt6::3DExtras
    )
    target_compile_definitions(prestige_vision PRIVATE PRESTIGE_HAVE_QT3D)
    message(STATUS "Qt 3D found — PBR virtual studios ENABLED")
else()
    message(STATUS "Qt 3D not found — using enhanced 2D fallback studios")
endif()

# ── rlottie (Samsung — Lottie animation rendering) ───────────
find_library(RLOTTIE_LIB rlottie PATHS ${CMAKE_SOURCE_DIR}/third_party/rlottie/lib /opt/homebrew/lib /usr/local/lib)
find_path(RLOTTIE_INC rlottie.h PATHS ${CMAKE_SOURCE_DIR}/third_party/rlottie/include /opt/homebrew/include /usr/local/include)
if(RLOTTIE_LIB AND RLOTTIE_INC)
    target_include_directories(prestige_vision PRIVATE ${RLOTTIE_INC})
    target_link_libraries(prestige_vision PRIVATE ${RLOTTIE_LIB})
    target_compile_definitions(prestige_vision PRIVATE PRESTIGE_HAVE_RLOTTIE)
    message(STATUS "rlottie found — Lottie animations ENABLED (pixel-perfect AE)")
else()
    message(STATUS "rlottie not found — using simplified Lottie fallback")
endif()

# ── ZeroMQ ──────────────────────────────────────────────────
if(ZMQ_FOUND)
    target_link_libraries(prestige_vision PRIVATE PkgConfig::ZMQ)
    target_compile_definitions(prestige_vision PRIVATE PRESTIGE_HAVE_ZMQ)
endif()

# ── FFmpeg ──────────────────────────────────────────────────
if(FFMPEG_FOUND)
    target_link_libraries(prestige_vision PRIVATE
        PkgConfig::AVCODEC
        PkgConfig::AVFORMAT
        PkgConfig::AVUTIL
        PkgConfig::SWSCALE
    )
    target_compile_definitions(prestige_vision PRIVATE PRESTIGE_HAVE_FFMPEG)
    message(STATUS "Vision Engine: FFmpeg enabled")
endif()

# ── Platform-specific ───────────────────────────────────────
if(APPLE)
    find_library(VIDEOTOOLBOX_FW VideoToolbox)
    find_library(COREMEDIA_FW CoreMedia)
    find_library(COREVIDEO_FW CoreVideo)
    find_library(AVFOUNDATION_FW AVFoundation)
    if(VIDEOTOOLBOX_FW)
        target_link_libraries(prestige_vision PRIVATE
            ${VIDEOTOOLBOX_FW} ${COREMEDIA_FW} ${COREVIDEO_FW} ${AVFOUNDATION_FW}
        )
        target_compile_definitions(prestige_vision PRIVATE PRESTIGE_HAVE_VIDEOTOOLBOX)
    endif()
endif()
