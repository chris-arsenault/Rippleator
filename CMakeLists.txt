cmake_minimum_required(VERSION 3.15)

project(Rippleator VERSION 0.1.0)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Check if JUCE exists, if not clone it
if(NOT EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/JUCE")
    message(STATUS "JUCE not found - cloning...")
    execute_process(
        COMMAND git clone --depth 1 --branch 7.0.5 https://github.com/juce-framework/JUCE.git
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        RESULT_VARIABLE GIT_RESULT
    )
    if(NOT GIT_RESULT EQUAL "0")
        message(FATAL_ERROR "Failed to clone JUCE repository")
    endif()
endif()

# Include the JUCE CMake utilities
add_subdirectory(JUCE)

# For now, we'll only build the Standalone format
# To enable VST3 format, you need to properly set up the VST3 SDK
set(FORMATS Standalone)

# Create the plugin target
juce_add_plugin(Rippleator
    VERSION 0.1.0
    COMPANY_NAME "Ahara"
    IS_SYNTH FALSE
    NEEDS_MIDI_INPUT FALSE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD TRUE
    PLUGIN_MANUFACTURER_CODE Ahra
    PLUGIN_CODE Rip1
    FORMATS ${FORMATS}
    PRODUCT_NAME "Rippleator"
)

# Set platform-specific options for macOS deployment
if(APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum OS X deployment version" FORCE)
    
    # Enable hardened runtime for macOS builds (required for notarization)
    set_target_properties(Rippleator PROPERTIES
        XCODE_ATTRIBUTE_ENABLE_HARDENED_RUNTIME YES
        XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "-"
    )
endif()

# Generate JuceHeader.h
juce_generate_juce_header(Rippleator)

# Add source files
target_sources(Rippleator
    PRIVATE
        Source/PluginProcessor.cpp
        Source/PluginEditor.cpp
        Source/Models/Chamber.cpp
        Source/Models/RayTracer.cpp
        Source/GUI/ChamberVisualizer.cpp
        Source/GUI/ZoneManager.cpp
        Source/GUI/WaveformVisualizer.cpp
        Source/GUI/FrequencyVisualizer.cpp
        Source/GUI/VisualizationsTab.cpp
        Source/GUI/LevelMeter.cpp
)

# Set include directories
target_include_directories(Rippleator
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/Source
)

# Link with JUCE libraries
target_link_libraries(Rippleator
    PRIVATE
        juce::juce_audio_utils
        juce::juce_audio_processors
        juce::juce_audio_plugin_client
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)
