#pragma once

#include <JuceHeader.h>

/**
 * A level meter component that displays audio levels in dB with peak hold.
 */
class LevelMeter : public juce::Component,
                  private juce::Timer
{
public:
    LevelMeter();
    ~LevelMeter() override;
    
    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    
    /**
     * Set the current level to display
     * @param newLevel The level in dB
     */
    void setLevel(float newLevel);
    
    /**
     * Set the range of the level meter
     * @param newMinLevel Minimum level in dB (e.g. -60.0f)
     * @param newMaxLevel Maximum level in dB (e.g. 0.0f)
     * @param stepSize Step size for display ticks
     */
    void setRange(float newMinLevel, float newMaxLevel, float stepSize = 1.0f);
    
private:
    float level;
    float peakLevel = -60.0f;
    float minLevel;
    float maxLevel;
    
    int holdTime;
    int holdCounter;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeter)
};
