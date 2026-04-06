# ============================================================
# Prestige AI — C++ AI Module
# Copyright (c) 2024-2026 Prestige Technologie Company
# ============================================================

project(prestige_ai_module LANGUAGES CXX)

set(AI_SOURCES
    src/FaceDetector.cpp
    src/FaceRecognizer.cpp
    src/FaceTracker.cpp
    src/TalentDatabase.cpp
    src/AiPipeline.cpp
    src/WhisperEngine.cpp
    src/TwitchChat.cpp
)

set(AI_HEADERS
    src/FaceDetector.h
    src/FaceRecognizer.h
    src/FaceTracker.h
    src/TalentDatabase.h
    src/AiPipeline.h
    src/WhisperEngine.h
    src/TwitchChat.h
)

add_library(prestige_ai_module STATIC ${AI_SOURCES} ${AI_HEADERS})

target_include_directories(prestige_ai_module PUBLIC src)

target_link_libraries(prestige_ai_module PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Multimedia
    Qt6::Network
)

# ONNX Runtime detection (accepts -DONNXRUNTIME_LIB and -DONNXRUNTIME_INC from parent)
if(ONNXRUNTIME_LIB AND ONNXRUNTIME_INC)
    # Passed from root CMake or CI
    if(EXISTS "${ONNXRUNTIME_LIB}")
        target_include_directories(prestige_ai_module PRIVATE ${ONNXRUNTIME_INC})
        target_link_libraries(prestige_ai_module PRIVATE ${ONNXRUNTIME_LIB})
        target_compile_definitions(prestige_ai_module PUBLIC PRESTIGE_HAVE_ONNXRT)
        message(STATUS "ONNX Runtime found — AI detection ENABLED")
    else()
        message(STATUS "ONNX Runtime path invalid — AI module uses stubs")
    endif()
else()
    # Try system paths
    find_library(ONNXRT_SYS_LIB onnxruntime PATHS /opt/homebrew/lib /usr/local/lib)
    find_path(ONNXRT_SYS_INC onnxruntime_cxx_api.h PATHS /opt/homebrew/include /usr/local/include PATH_SUFFIXES onnxruntime)
    if(ONNXRT_SYS_LIB AND ONNXRT_SYS_INC)
        target_include_directories(prestige_ai_module PRIVATE ${ONNXRT_SYS_INC})
        target_link_libraries(prestige_ai_module PRIVATE ${ONNXRT_SYS_LIB})
        target_compile_definitions(prestige_ai_module PUBLIC PRESTIGE_HAVE_ONNXRT)
        message(STATUS "ONNX Runtime (system) found — AI detection ENABLED")
    else()
        message(STATUS "ONNX Runtime not found — AI module uses stubs (detection disabled)")
    endif()
endif()

# whisper.cpp detection (for speech-to-text)
find_library(WHISPER_LIB whisper PATHS /opt/homebrew/lib /usr/local/lib ${CMAKE_SOURCE_DIR}/third_party/whisper)
find_path(WHISPER_INC whisper.h PATHS /opt/homebrew/include /usr/local/include ${CMAKE_SOURCE_DIR}/third_party/whisper)
if(WHISPER_LIB AND WHISPER_INC)
    target_include_directories(prestige_ai_module PRIVATE ${WHISPER_INC})
    target_link_libraries(prestige_ai_module PRIVATE ${WHISPER_LIB})
    target_compile_definitions(prestige_ai_module PUBLIC PRESTIGE_HAVE_WHISPER)
    message(STATUS "whisper.cpp found — speech-to-text ENABLED")
else()
    message(STATUS "whisper.cpp not found — subtitle engine uses stub")
endif()
