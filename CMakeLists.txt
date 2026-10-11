cmake_minimum_required(VERSION 3.22)

project(SonicWaveshaper VERSION 1.0.0)

# JUCE 8.0.6 (clone or place JUCE next to this file)
add_subdirectory(JUCE)

include(FetchContent)

# Eigen 3.4.0 (header-only, required by NAM).
FetchContent_Declare(eigen
    URL https://gitlab.com/libeigen/eigen/-/archive/3.4.0/eigen-3.4.0.tar.gz)

# nlohmann/json 3.11.3 (header-only, required by NAM).
FetchContent_Declare(nlohmann_json
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)

# NeuralAmpModelerCore at a pinned commit. We only need its source files, not its CMake tree.
FetchContent_Declare(nam
    GIT_REPOSITORY https://github.com/sdatkinson/NeuralAmpModelerCore.git
    GIT_TAG 0b3d3c97b0859a3a8c92a8628c4dd89a25eb5842
    GIT_SUBMODULES "")

FetchContent_MakeAvailable(eigen nlohmann_json)

# Download NAM's sources only. Populate does NOT run NAM's own CMakeLists.txt,
# so its tools (render, training) and their missing source files are never built.
FetchContent_GetProperties(nam)
if (NOT nam_POPULATED)
    FetchContent_Populate(nam)
endif()

# Download the bundled model at configure time and turn it into a C++ byte array.
set(NAM_GEN_DIR "${CMAKE_BINARY_DIR}/generated")
file(MAKE_DIRECTORY "${NAM_GEN_DIR}")
set(NAM_MODEL_URL "https://raw.githubusercontent.com/sdatkinson/NeuralAmpModelerCore/0b3d3c97b0859a3a8c92a8628c4dd89a25eb5842/example_models/wavenet_a1_standard.nam")
set(NAM_MODEL_FILE "${NAM_GEN_DIR}/wavenet_a1_standard.nam")
set(NAM_MODEL_HEADER "${NAM_GEN_DIR}/EmbeddedModel.h")
set(NAM_MODEL_EXPECTED_BYTES 407452)

if (NOT EXISTS "${NAM_MODEL_FILE}")
    file(DOWNLOAD "${NAM_MODEL_URL}" "${NAM_MODEL_FILE}" STATUS NAM_DL_STATUS TLS_VERIFY ON)
    list(GET NAM_DL_STATUS 0 NAM_DL_CODE)
    if (NOT NAM_DL_CODE EQUAL 0)
        file(REMOVE "${NAM_MODEL_FILE}")
        message(FATAL_ERROR "Failed to download wavenet_a1_standard.nam: ${NAM_DL_STATUS}")
    endif()
endif()

file(SIZE "${NAM_MODEL_FILE}" NAM_MODEL_BYTES)
if (NOT NAM_MODEL_BYTES EQUAL NAM_MODEL_EXPECTED_BYTES)
    file(REMOVE "${NAM_MODEL_FILE}")
    message(FATAL_ERROR "wavenet_a1_standard.nam has ${NAM_MODEL_BYTES} bytes, expected ${NAM_MODEL_EXPECTED_BYTES}")
endif()

file(READ "${NAM_MODEL_FILE}" NAM_MODEL_HEX HEX)
string(REGEX REPLACE "([0-9a-f][0-9a-f])" "0x\\1," NAM_MODEL_ARRAY "${NAM_MODEL_HEX}")
file(WRITE "${NAM_MODEL_HEADER}"
"#pragma once\n\n#include <cstddef>\n\n// Generated at configure time from NeuralAmpModelerCore wavenet_a1_standard.nam.\ninline constexpr unsigned char kWavenetA1Model[] = {\n${NAM_MODEL_ARRAY}\n};\n\ninline constexpr std::size_t kWavenetA1ModelSize = ${NAM_MODEL_BYTES};\n")

# NAM core compiled directly from its sources with float samples.
file(GLOB NAM_CORE_SOURCES
    "${nam_SOURCE_DIR}/NAM/*.cpp"
    "${nam_SOURCE_DIR}/NAM/wavenet/*.cpp")

add_library(nam_core STATIC ${NAM_CORE_SOURCES})
target_compile_features(nam_core PUBLIC cxx_std_20)
target_compile_definitions(nam_core PUBLIC NAM_SAMPLE_FLOAT)
# Include root is include/ so that <nlohmann/...> resolves to include/nlohmann/...
target_include_directories(nam_core PUBLIC
    "${nam_SOURCE_DIR}"
    "${eigen_SOURCE_DIR}"
    "${nlohmann_json_SOURCE_DIR}/include")
set_target_properties(nam_core PROPERTIES POSITION_INDEPENDENT_CODE ON)

juce_add_plugin(SonicWaveshaper
    COMPANY_NAME "SonicWaveshaper"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
    PLUGIN_MANUFACTURER_CODE Sncw
    PLUGIN_CODE Swv1
    FORMATS AU VST3 Standalone
    PRODUCT_NAME "SonicWaveshaper")

target_sources(SonicWaveshaper
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp)

target_include_directories(SonicWaveshaper
    PRIVATE
        "${NAM_GEN_DIR}")

target_compile_definitions(SonicWaveshaper
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0)

# Note: juce_recommended_lto_flags is intentionally NOT linked (avoids arm64 createPluginFilter link errors).
target_link_libraries(SonicWaveshaper
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_dsp
        juce::juce_gui_basics
        juce::juce_gui_extra
        nam_core
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)
