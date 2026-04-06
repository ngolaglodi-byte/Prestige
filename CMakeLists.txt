# ============================================================
# Prestige AI — Professional Broadcast Overlay System
# Copyright (c) 2024-2026 Prestige Technologie Company
# All rights reserved.
# ============================================================

cmake_minimum_required(VERSION 3.21)
project(PrestigeAI VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# ── Qt 6 ────────────────────────────────────────────────────
find_package(Qt6 6.6 REQUIRED COMPONENTS
    Core Gui Widgets Qml Quick QuickControls2 Multimedia OpenGL Network
)
qt_standard_project_setup()

# ── Broadcast SDKs ──────────────────────────────────────────
# ALL SDKs are always compiled in. Detection is at RUNTIME.
# Users install hardware + driver, Prestige AI auto-detects.
# No cmake flags needed — no -DPRESTIGE_ENABLE_* required.

# ── ZeroMQ ──────────────────────────────────────────────────
find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
    pkg_check_modules(ZMQ IMPORTED_TARGET libzmq)
endif()

if(NOT ZMQ_FOUND)
    find_library(ZMQ_LIBRARY NAMES zmq libzmq
        PATHS /opt/homebrew/lib /usr/local/lib)
    find_path(ZMQ_INCLUDE_DIR zmq.h
        PATHS /opt/homebrew/include /usr/local/include)
    if(ZMQ_LIBRARY AND ZMQ_INCLUDE_DIR)
        set(ZMQ_FOUND TRUE)
        add_library(PkgConfig::ZMQ INTERFACE IMPORTED)
        target_link_libraries(PkgConfig::ZMQ INTERFACE ${ZMQ_LIBRARY})
        target_include_directories(PkgConfig::ZMQ INTERFACE ${ZMQ_INCLUDE_DIR})
    endif()
endif()

if(NOT ZMQ_FOUND)
    message(WARNING "ZeroMQ not found — ZMQ features will be stubbed out")
endif()

# ── FFmpeg ──────────────────────────────────────────────────
if(PkgConfig_FOUND)
    pkg_check_modules(AVCODEC  IMPORTED_TARGET libavcodec)
    pkg_check_modules(AVFORMAT IMPORTED_TARGET libavformat)
    pkg_check_modules(AVUTIL   IMPORTED_TARGET libavutil)
    pkg_check_modules(SWSCALE  IMPORTED_TARGET libswscale)
endif()

if(AVCODEC_FOUND AND AVFORMAT_FOUND AND AVUTIL_FOUND AND SWSCALE_FOUND)
    set(FFMPEG_FOUND TRUE)
    message(STATUS "FFmpeg found: avcodec ${AVCODEC_VERSION}")
else()
    set(FFMPEG_FOUND FALSE)
    message(WARNING "FFmpeg not found — encoder/output will be stubbed")
endif()

# ── ONNX Runtime (AI detection) ───────────────────────────
# Downloaded automatically if not found
set(ONNXRUNTIME_VERSION "1.17.0")
find_library(ONNXRUNTIME_LIB NAMES onnxruntime PATHS /opt/homebrew/lib /usr/local/lib)
find_path(ONNXRUNTIME_INC onnxruntime_cxx_api.h PATH_SUFFIXES onnxruntime PATHS /opt/homebrew/include /usr/local/include)

if(ONNXRUNTIME_LIB AND ONNXRUNTIME_INC)
    set(ONNXRUNTIME_FOUND TRUE)
    message(STATUS "ONNX Runtime found: ${ONNXRUNTIME_LIB}")
else()
    set(ONNXRUNTIME_FOUND FALSE)
    message(STATUS "ONNX Runtime not found — AI detection will use stubs")
endif()

# ── Platform detection ──────────────────────────────────────
if(WIN32)
    add_compile_definitions(PRESTIGE_PLATFORM_WINDOWS)
elseif(APPLE)
    add_compile_definitions(PRESTIGE_PLATFORM_MACOS)
    # Suppress macOS deprecation warnings (OpenGL)
    add_compile_options(-Wno-deprecated-declarations)
else()
    add_compile_definitions(PRESTIGE_PLATFORM_LINUX)
endif()

# ── Shared headers (interface library) ──────────────────────
add_library(prestige_shared INTERFACE)
target_include_directories(prestige_shared INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/shared
)

# ── Sub-projects ────────────────────────────────────────────
add_subdirectory(broadcast_sdk)
add_subdirectory(ai_module)
add_subdirectory(vision_engine)
add_subdirectory(control_room)
