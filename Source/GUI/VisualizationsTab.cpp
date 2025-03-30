#include "VisualizationsTab.h"

VisualizationsTab::VisualizationsTab(Chamber& chamber)
    : chamber(chamber),
      speakerWaveform("Speaker Input"),
      speakerFrequency("Speaker Frequency Response")
{
    // Set up speaker visualizers
    speakerWaveform.setColor(juce::Colours::yellow);
    addAndMakeVisible(speakerWaveform);
    
    speakerFrequency.setColor(juce::Colours::yellow);
    addAndMakeVisible(speakerFrequency);
    
    // Set up microphone visualizers
    juce::Colour micColors[3] = {
        juce::Colours::green,
        juce::Colours::cyan,
        juce::Colours::magenta
    };
    
    for (int i = 0; i < 3; ++i)
    {
        micWaveforms[i].setName("Mic " + juce::String(i + 1) + " Output");
        micWaveforms[i].setColor(micColors[i]);
        addAndMakeVisible(micWaveforms[i]);
        
        micFrequencies[i].setName("Mic " + juce::String(i + 1) + " Frequency Response");
        micFrequencies[i].setColor(micColors[i]);
        addAndMakeVisible(micFrequencies[i]);
    }
    
    // Start visualizations
    startVisualizations();
}

VisualizationsTab::~VisualizationsTab()
{
    stopVisualizations();
}

void VisualizationsTab::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
}

void VisualizationsTab::resized()
{
    auto area = getLocalBounds().reduced(10);
    
    // Calculate sizes - adjust the height distribution to ensure all visualizers fit
    int totalHeight = area.getHeight();
    int speakerHeight = totalHeight / 5;  // Speaker gets 1/5 of the height
    int micHeight = (totalHeight - speakerHeight - 30) / 3;  // Each mic gets equal share of remaining height
    int waveformWidth = area.getWidth() / 2;
    
    // Speaker visualizers at the top
    auto speakerRow = area.removeFromTop(speakerHeight);
    speakerWaveform.setBounds(speakerRow.removeFromLeft(waveformWidth));
    speakerFrequency.setBounds(speakerRow);
    
    // Add spacing
    area.removeFromTop(10);
    
    // Microphone visualizers (3 rows, each with waveform and frequency response)
    for (int i = 0; i < 3; ++i)
    {
        auto micRow = area.removeFromTop(micHeight);
        micWaveforms[i].setBounds(micRow.removeFromLeft(waveformWidth));
        micFrequencies[i].setBounds(micRow);
        
        // Add spacing between rows (except after the last one)
        if (i < 2)
        {
            area.removeFromTop(10);
        }
    }
}

void VisualizationsTab::timerCallback()
{
    // Update frequency visualizers with the latest frequency responses
    const auto& micFrequencyResponses = chamber.getMicFrequencyResponses();
    
    // Update microphone frequency visualizers
    for (int i = 0; i < 3; ++i)
    {
        micFrequencies[i].updateFrequencyBands(micFrequencyResponses[i]);
    }
    
    // For speaker frequency, we'll use a flat response for now
    MicFrequencyBands speakerBands = MicFrequencyBands();
    speakerFrequency.updateFrequencyBands(speakerBands);
    
    // Update waveform visualizers
    // Get the latest audio samples from the microphone buffers
    const auto& micBuffers = chamber.getMicBuffers();
    
    // For each microphone, get the most recent samples and add to visualizer
    for (int i = 0; i < 3; ++i)
    {
        if (!micBuffers[i].empty())
        {
            // Get the most recent samples (up to SAMPLES_PER_UPDATE)
            int numSamples = std::min(SAMPLES_PER_UPDATE, static_cast<int>(micBuffers[i].size()));
            int startIdx = static_cast<int>(micBuffers[i].size()) - numSamples;
            
            if (startIdx >= 0)
            {
                for (int j = 0; j < numSamples; ++j)
                {
                    micWaveforms[i].addSample(micBuffers[i][startIdx + j]);
                }
            }
        }
    }
    
    // For speaker waveform, we'll use a simple sine wave for now
    static float phase = 0.0f;
    for (int i = 0; i < SAMPLES_PER_UPDATE; ++i)
    {
        float sample = 0.5f * std::sin(phase);
        speakerWaveform.addSample(sample);
        phase += 0.1f;
    }
}

void VisualizationsTab::startVisualizations()
{
    // Start timer for updating visualizations
    startTimerHz(UPDATE_RATE_HZ);
    
    // Start individual waveform visualizers
    speakerWaveform.startVisualization();
    for (auto& waveform : micWaveforms)
    {
        waveform.startVisualization();
    }
}

void VisualizationsTab::stopVisualizations()
{
    stopTimer();
    
    speakerWaveform.stopVisualization();
    for (auto& waveform : micWaveforms)
    {
        waveform.stopVisualization();
    }
}
