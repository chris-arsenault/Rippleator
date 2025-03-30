#pragma once

#include <JuceHeader.h>
#include "WaveformVisualizer.h"
#include "FrequencyVisualizer.h"
#include "../Models/Chamber.h"

/**
 * Component that contains visualizations for audio waveforms and frequency responses.
 */
class VisualizationsTab : public juce::Component,
                         private juce::Timer
{
public:
    VisualizationsTab(Chamber& chamber);
    ~VisualizationsTab() override;
    
    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    
    void startVisualizations();
    void stopVisualizations();

private:
    Chamber& chamber;
    
    // Speaker visualizers
    WaveformVisualizer speakerWaveform;
    FrequencyVisualizer speakerFrequency;
    
    // Microphone visualizers (one for each microphone)
    std::array<WaveformVisualizer, 3> micWaveforms;
    std::array<FrequencyVisualizer, 3> micFrequencies;
    
    // Constants
    static constexpr int UPDATE_RATE_HZ = 30;
    static constexpr int SAMPLES_PER_UPDATE = 10;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VisualizationsTab)
};
