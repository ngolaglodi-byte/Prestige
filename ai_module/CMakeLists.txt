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
)

set(AI_HEADERS
    src/FaceDetector.h
    src/FaceRecognizer.h
    src/FaceTracker.h
    src/TalentDatabase.h
    src/AiPipeline.h
)

add_library(prestige_ai_module STATIC ${AI_SOURCES} ${AI_HEADERS})

target_include_directories(prestige_ai_module PUBLIC src)

target_link_libraries(prestige_ai_module PRIVATE
    Qt6::Core
    Qt6::Gui
)

# ONNX Runtime (optional — stubs used when not available)
find_path(ONNXRUNTIME_INCLUDE_DIR onnxruntime_cxx_api.h
    PATHS /usr/local/include/onnxruntime /opt/homebrew/include/onnxruntime
    PATH_SUFFIXES onnxruntime/core/session
)
find_library(ONNXRUNTIME_LIBRARY onnxruntime
    PATHS /usr/local/lib /opt/homebrew/lib
)
if(ONNXRUNTIME_INCLUDE_DIR AND ONNXRUNTIME_LIBRARY)
    target_include_directories(prestige_ai_module PRIVATE ${ONNXRUNTIME_INCLUDE_DIR})
    target_link_libraries(prestige_ai_module PRIVATE ${ONNXRUNTIME_LIBRARY})
    target_compile_definitions(prestige_ai_module PUBLIC PRESTIGE_HAVE_ONNXRT)
    message(STATUS "ONNX Runtime found — AI detection enabled")
else()
    message(STATUS "ONNX Runtime not found — AI module uses stubs (detection disabled)")
endif()
