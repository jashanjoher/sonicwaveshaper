cmake_minimum_required(VERSION 3.22)

project(SonicWaveshaper VERSION 1.0.0)

# JUCE 8.0.6 (clone or place JUCE next to this file)
add_subdirectory(JUCE)

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
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_warning_flags)
