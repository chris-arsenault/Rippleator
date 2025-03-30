#pragma once

#include <JuceHeader.h>

/**
 * A simple level meter component that displays audio levels with customizable colors.
 */
class LevelMeter : public juce::Component
{
public:
    LevelMeter()
    {
        level = 0.0f;
        setOpaque(false);
    }
    
    void setLevel(float newLevel)
    {
        // Clamp the level between 0 and 1
        newLevel = juce::jlimit(0.0f, 1.0f, newLevel);
        
        if (newLevel != level)
        {
            level = newLevel;
            repaint();
        }
    }
    
    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        
        // Background
        g.setColour(juce::Colours::darkgrey.withAlpha(0.8f));
        g.fillRoundedRectangle(bounds, 3.0f);
        
        // Calculate meter height based on level
        float meterHeight = bounds.getHeight() * level;
        juce::Rectangle<float> meterRect(bounds.getX(), bounds.getBottom() - meterHeight, 
                                        bounds.getWidth(), meterHeight);
        
        // Gradient for meter
        juce::ColourGradient gradient(
            juce::Colours::green,
            bounds.getBottomLeft(),
            juce::Colours::red,
            bounds.getTopLeft(),
            false);
        gradient.addColour(0.5, juce::Colours::yellow);
        
        g.setGradientFill(gradient);
        g.fillRoundedRectangle(meterRect, 3.0f);
        
        // Border
        g.setColour(juce::Colours::black);
        g.drawRoundedRectangle(bounds, 3.0f, 1.0f);
        
        // Level ticks
        g.setColour(juce::Colours::white.withAlpha(0.5f));
        for (float y = 0.25f; y <= 0.75f; y += 0.25f)
        {
            float tickY = bounds.getY() + bounds.getHeight() * (1.0f - y);
            g.drawLine(bounds.getX(), tickY, bounds.getRight(), tickY, 1.0f);
        }
    }
    
private:
    float level;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LevelMeter)
};
