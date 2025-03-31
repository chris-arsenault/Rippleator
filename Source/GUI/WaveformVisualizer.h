#pragma once

#include <JuceHeader.h>
#include <deque>

/**
 * Component that visualizes an audio waveform over time.
 */
class WaveformVisualizer : public juce::Component,
                          public juce::Timer
{
public:
    WaveformVisualizer(const juce::String& name = "Waveform");
    ~WaveformVisualizer() override;
    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    
    /**
     * Add a new sample to the waveform buffer
     * @param sample The audio sample value (-1.0 to 1.0)
     */
    void addSample(float sample);
    
    /**
     * Add multiple samples to the waveform buffer
     * @param samples Array of samples to add
     * @param numSamples Number of samples to add
     */
    void addSamples(const float* samples, int numSamples);
    
    /**
     * Clear all samples from the buffer
     */
    void clear();
    
    /**
     * Set the color of the waveform
     * @param color The color to use for the waveform
     */
    void setColor(const juce::Colour& color);
    
    /**
     * Set the name/label for this waveform
     * @param name The name to display
     */
    void setName(const juce::String& name);
    
    /**
     * Start the timer to update the visualization
     * @param intervalMs The update interval in milliseconds
     */
    void startVisualization(int intervalMs = 30);
    
    /**
     * Stop the visualization timer
     */
    void stopVisualization();

private:
    juce::String displayName;
    juce::Colour waveformColor;
    
    // Buffer for waveform data
    std::deque<float> samples;
    const int maxSamples = 1024;
    
    // UI elements
    juce::Label nameLabel;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WaveformVisualizer)
};
