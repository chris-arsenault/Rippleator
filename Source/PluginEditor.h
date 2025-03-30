#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GUI/ChamberVisualizer.h"
#include "GUI/ZoneManager.h"
#include "GUI/LevelMeter.h"
#include "GUI/VisualizationsTab.h"

class RippleatorAudioProcessorEditor : public juce::AudioProcessorEditor,
                                      private juce::Timer
{
public:
    RippleatorAudioProcessorEditor(RippleatorAudioProcessor&);
    ~RippleatorAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    
    void timerCallback() override;
    
    // Handle key presses directly in the component
    bool keyPressed(const juce::KeyPress& key) override;

private:
    RippleatorAudioProcessor& audioProcessor;
    
    // Tabbed component for different views
    juce::TabbedComponent tabbedComponent;
    
    // Main tabs
    ChamberVisualizer chamberVisualizer;
    ZoneManager zoneManager;
    VisualizationsTab visualizationsTab;
    
    // UI components
    juce::Label titleLabel;
    
    // Bypass processing button
    juce::ToggleButton bypassButton;
    
    // For tab name reset
    int tabNameResetCounter;
    
    // Microphone controls
    struct MicControls
    {
        juce::Label label;
        juce::Slider volumeSlider;
        juce::ToggleButton soloButton;
        juce::ToggleButton muteButton;
        LevelMeter levelMeter;
        std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> volumeAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> soloAttachment;
        std::unique_ptr<juce::AudioProcessorValueTreeState::ButtonAttachment> muteAttachment;
    };
    
    std::array<MicControls, 3> micControls;
    
    // Chamber parameter controls
    juce::Label densityLabel;
    juce::Slider densitySlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> densityAttachment;
    
    juce::Label reflectivityLabel;
    juce::Slider reflectivitySlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> reflectivityAttachment;
    
    juce::Label dampingLabel;
    juce::Slider dampingSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dampingAttachment;
    
    // Output controls
    juce::Label outputGainLabel;
    juce::Slider outputGainSlider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> outputGainAttachment;
    
    // Level meters
    LevelMeter inputLevelMeter;
    LevelMeter outputLevelMeter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RippleatorAudioProcessorEditor)
};
