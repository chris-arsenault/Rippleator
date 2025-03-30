#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "GUI/ChamberVisualizer.h"
#include "GUI/ZoneManager.h"
#include "GUI/LevelMeter.h"

class RippleatorAudioProcessorEditor : public juce::AudioProcessorEditor,
                                      private juce::Timer
{
public:
    RippleatorAudioProcessorEditor(RippleatorAudioProcessor&);
    ~RippleatorAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;
    
    void timerCallback() override;

private:
    RippleatorAudioProcessor& audioProcessor;
    
    ChamberVisualizer chamberVisualizer;
    ZoneManager zoneManager;
    
    // UI components
    juce::Label titleLabel;
    
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
    juce::Label reflectivityLabel;
    juce::Slider reflectivitySlider;
    juce::Label dampingLabel;
    juce::Slider dampingSlider;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> densityAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> reflectivityAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> dampingAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RippleatorAudioProcessorEditor)
};
