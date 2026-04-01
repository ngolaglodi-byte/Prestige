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
    find_path(ONNXRT_SYS_INC onnxruntime_cxx_api.h PATHS /opt/homebrew/include /usr/local/include PATH_SUFFIXES onnxruntime/core/session)
    if(ONNXRT_SYS_LIB AND ONNXRT_SYS_INC)
        target_include_directories(prestige_ai_module PRIVATE ${ONNXRT_SYS_INC})
        target_link_libraries(prestige_ai_module PRIVATE ${ONNXRT_SYS_LIB})
        target_compile_definitions(prestige_ai_module PUBLIC PRESTIGE_HAVE_ONNXRT)
        message(STATUS "ONNX Runtime (system) found — AI detection ENABLED")
    else()
        message(STATUS "ONNX Runtime not found — AI module uses stubs (detection disabled)")
    endif()
endif()
