#include "FrequencyVisualizer.h"

FrequencyVisualizer::FrequencyVisualizer(const juce::String& name)
    : displayName(name), barColor(juce::Colours::orange)
{
    // Initialize the name label
    nameLabel.setText(displayName, juce::dontSendNotification);
    nameLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    nameLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(nameLabel);
    
    // Initialize with 8 frequency bands (matching Chamber::NUM_FREQUENCY_BANDS)
    frequencyBands = MicFrequencyBands();
}

FrequencyVisualizer::~FrequencyVisualizer()
{
}

void FrequencyVisualizer::paint(juce::Graphics& g)
{
    // Fill background
    g.fillAll(juce::Colours::black);
    
    // Draw border
    g.setColour(juce::Colours::grey);
    g.drawRect(getLocalBounds(), 1);
    
    // Draw frequency bands
    auto bounds = getLocalBounds().reduced(2, 20).withTrimmedTop(10);
    
    int numBands = static_cast<int>(frequencyBands.bands.size());
    if (numBands == 0) return;
    
    float barWidth = bounds.getWidth() / numBands;
    float maxBarHeight = bounds.getHeight() - 4;
    
    // Draw frequency band labels
    g.setColour(juce::Colours::grey);
    g.setFont(juce::Font(10.0f));
    
    for (int i = 0; i < numBands; ++i)
    {
        float x = bounds.getX() + i * barWidth;

        float center = (frequencyBands.bands[i].minFrequency + frequencyBands.bands[i].maxFrequency) / 2;
        if (center > 1000)
        {
            center /= 1000;
        }

        juce::String freqLabel= juce::String::toDecimalStringWithSignificantFigures(center, 3);
        
        g.drawText(freqLabel, 
                  juce::Rectangle<float>(x, bounds.getBottom() - 12, barWidth, 12),
                  juce::Justification::centred, false);
    }
    
    // Draw bars
    g.setColour(barColor);
    
    for (int i = 0; i < numBands; ++i)
    {
        float value = juce::jlimit(0.0f, 1.0f, frequencyBands.bands[i].value);
        float x = bounds.getX() + i * barWidth;
        float barHeight = value * maxBarHeight;
        
        g.fillRect(x + 2, bounds.getBottom() - barHeight - 14, barWidth - 4, barHeight);
    }
    
    // Draw 0dB line
    g.setColour(juce::Colours::darkgrey);
    g.drawHorizontalLine(bounds.getBottom() - 14, bounds.getX(), bounds.getRight());
}

void FrequencyVisualizer::resized()
{
    auto bounds = getLocalBounds();
    nameLabel.setBounds(bounds.removeFromTop(20).reduced(5, 0));
}

void FrequencyVisualizer::updateFrequencyBands(const MicFrequencyBands& bands)
{
    frequencyBands = bands;
    repaint();
}

void FrequencyVisualizer::setColor(const juce::Colour& color)
{
    barColor = color;
    repaint();
}

void FrequencyVisualizer::setName(const juce::String& name)
{
    displayName = name;
    nameLabel.setText(displayName, juce::dontSendNotification);
}
