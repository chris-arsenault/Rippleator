#pragma once

#include <JuceHeader.h>
#include "../Models/MicFrequencyBands.h"

/**
 * Component that visualizes frequency band attenuation.
 */
class FrequencyVisualizer : public juce::Component
{
public:
    FrequencyVisualizer(const juce::String& name = "Frequency Response");
    ~FrequencyVisualizer() override;
    
    void paint(juce::Graphics&) override;
    void resized() override;
    
    /**
     * Update the frequency band values
     * @param bands Vector of frequency band values (typically 0.0 to 1.0)
     */
    void updateFrequencyBands(const MicFrequencyBands& bands);
    
    /**
     * Set the color of the frequency bars
     * @param color The color to use for the bars
     */
    void setColor(const juce::Colour& color);
    
    /**
     * Set the name/label for this visualizer
     * @param name The name to display
     */
    void setName(const juce::String& name);

private:
    juce::String displayName;
    juce::Colour barColor;
    
    // Frequency band data
    MicFrequencyBands frequencyBands;
    
    // UI elements
    juce::Label nameLabel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(FrequencyVisualizer)
};
