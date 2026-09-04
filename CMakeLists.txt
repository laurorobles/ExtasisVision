# CMakeLists.txt
cmake_minimum_required(VERSION 3.24)

project(ExtasisVision VERSION 0.0.1)

# Fetch JUCE
include(FetchContent)
FetchContent_Declare(
    juce
    URL https://github.com/juce-framework/JUCE/archive/refs/tags/8.0.4.zip
)
FetchContent_MakeAvailable(juce)

juce_add_plugin(ExtasisVision
    COMPANY_NAME "Extasis Records"
    PLUGIN_MANUFACTURER_CODE ExtR
    PLUGIN_CODE ExVi
    FORMATS Standalone VST3 AU
    PRODUCT_NAME "ExtasisVision"
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_SYNTH TRUE
    NEEDS_WEB_BROWSER FALSE
    NEEDS_CURL FALSE
    COPY_PLUGIN_AFTER_BUILD TRUE
)

target_sources(ExtasisVision
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
)

juce_generate_juce_header(ExtasisVision)

target_compile_definitions(ExtasisVision
    PUBLIC
        JUCE_WEB_BROWSER=0
        JUCE_USE_CURL=0
        JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(ExtasisVision
    PRIVATE
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

if(APPLE)
    add_custom_command(TARGET ExtasisVision_VST3 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_BINARY_DIR}/ExtasisVision_artefacts/VST3/ExtasisVision.vst3"
        "$ENV{HOME}/Desktop/ExtasisRecords/ExtasisVision.vst3"
        COMMENT "Copying VST3 to Extasis Records folder..."
    )
endif()
