#include "LevelMeter.h"

LevelMeter::LevelMeter()
    : level(0.0f), minLevel(-60.0f), maxLevel(0.0f), holdTime(500), holdCounter(0)
{
    startTimerHz(30); // Update at 30Hz
}

LevelMeter::~LevelMeter()
{
    stopTimer();
}

void LevelMeter::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Background
    g.setColour(juce::Colours::black);
    g.fillRect(bounds);
    
    // Border
    g.setColour(juce::Colours::grey);
    g.drawRect(bounds, 1.0f);
    
    // Calculate level height
    float normalizedLevel = juce::jmap(level, minLevel, maxLevel, 0.0f, 1.0f);
    float levelHeight = bounds.getHeight() * normalizedLevel;
    
    // Draw level meter
    juce::Colour meterColour;
    if (normalizedLevel > 0.9f)
        meterColour = juce::Colours::red;
    else if (normalizedLevel > 0.7f)
        meterColour = juce::Colours::orange;
    else
        meterColour = juce::Colours::green;
    
    g.setColour(meterColour);
    g.fillRect(bounds.getX() + 2.0f, 
               bounds.getBottom() - levelHeight - 2.0f, 
               bounds.getWidth() - 4.0f, 
               levelHeight);
    
    // Draw peak hold line
    g.setColour(juce::Colours::white);
    float peakY = bounds.getBottom() - (bounds.getHeight() * juce::jmap(peakLevel, minLevel, maxLevel, 0.0f, 1.0f)) - 2.0f;
    g.drawLine(bounds.getX() + 2.0f, peakY, bounds.getRight() - 2.0f, peakY, 1.0f);
    
    // Draw level text
    g.setColour(juce::Colours::white);
    g.setFont(10.0f);
    g.drawText(juce::String(static_cast<int>(level)) + " dB", 
               bounds.reduced(2.0f), 
               juce::Justification::centredBottom, 
               false);
}

void LevelMeter::resized()
{
    // Nothing to do here
}

void LevelMeter::setLevel(float newLevel)
{
    level = juce::jlimit(minLevel, maxLevel, newLevel);
    
    // Update peak level
    if (level > peakLevel)
    {
        peakLevel = level;
        holdCounter = holdTime;
    }
    
    repaint();
}

void LevelMeter::setRange(float newMinLevel, float newMaxLevel, float stepSize)
{
    minLevel = newMinLevel;
    maxLevel = newMaxLevel;
    repaint();
}

void LevelMeter::timerCallback()
{
    // Update peak hold
    if (holdCounter > 0)
    {
        holdCounter--;
        if (holdCounter <= 0)
        {
            // Gradually reduce peak level
            peakLevel = juce::jmax(level, peakLevel - 3.0f);
            repaint();
        }
    }
}
