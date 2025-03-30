#include "WaveformVisualizer.h"

WaveformVisualizer::WaveformVisualizer(const juce::String& name)
    : displayName(name), waveformColor(juce::Colours::green)
{
    // Initialize the name label
    nameLabel.setText(displayName, juce::dontSendNotification);
    nameLabel.setFont(juce::Font(14.0f, juce::Font::bold));
    nameLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(nameLabel);
    
    // Pre-fill the sample buffer with zeros
    samples.resize(maxSamples, 0.0f);
}

WaveformVisualizer::~WaveformVisualizer()
{
    stopTimer();
}

void WaveformVisualizer::paint(juce::Graphics& g)
{
    // Fill background
    g.fillAll(juce::Colours::black);
    
    // Draw border
    g.setColour(juce::Colours::grey);
    g.drawRect(getLocalBounds(), 1);
    
    // Draw center line
    auto bounds = getLocalBounds().reduced(2, 20).withTrimmedTop(10);
    int centerY = bounds.getCentreY();
    g.setColour(juce::Colours::darkgrey);
    g.drawHorizontalLine(centerY, bounds.getX(), bounds.getRight());
    
    // Draw waveform
    g.setColour(waveformColor);
    
    float width = bounds.getWidth();
    float height = bounds.getHeight();
    float halfHeight = height / 2.0f;
    
    juce::Path waveformPath;
    bool pathStarted = false;
    
    // Draw samples
    int numSamples = static_cast<int>(samples.size());
    float xScale = width / numSamples;
    
    for (int i = 0; i < numSamples; ++i)
    {
        float x = bounds.getX() + i * xScale;
        float y = centerY - samples[i] * halfHeight;
        
        if (!pathStarted)
        {
            waveformPath.startNewSubPath(x, y);
            pathStarted = true;
        }
        else
        {
            waveformPath.lineTo(x, y);
        }
    }
    
    g.strokePath(waveformPath, juce::PathStrokeType(1.5f));
}

void WaveformVisualizer::resized()
{
    auto bounds = getLocalBounds();
    nameLabel.setBounds(bounds.removeFromTop(20).reduced(5, 0));
}

void WaveformVisualizer::timerCallback()
{
    repaint();
}

void WaveformVisualizer::addSample(float sample)
{
    // Add new sample and remove oldest if buffer is full
    if (samples.size() >= maxSamples)
    {
        samples.pop_front();
    }
    
    // Clamp sample value to prevent extreme values
    sample = juce::jlimit(-1.0f, 1.0f, sample);
    samples.push_back(sample);
}

void WaveformVisualizer::addSamples(const float* newSamples, int numSamples)
{
    // Add multiple samples at once
    for (int i = 0; i < numSamples; ++i)
    {
        addSample(newSamples[i]);
    }
}

void WaveformVisualizer::clear()
{
    samples.clear();
    samples.resize(maxSamples, 0.0f);
}

void WaveformVisualizer::setColor(const juce::Colour& color)
{
    waveformColor = color;
    repaint();
}

void WaveformVisualizer::setName(const juce::String& name)
{
    displayName = name;
    nameLabel.setText(displayName, juce::dontSendNotification);
}

void WaveformVisualizer::startVisualization(int intervalMs)
{
    startTimer(intervalMs);
}

void WaveformVisualizer::stopVisualization()
{
    stopTimer();
}
