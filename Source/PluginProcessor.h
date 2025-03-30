#pragma once

#include <JuceHeader.h>
#include "Models/Chamber.h"

class RippleatorAudioProcessor : public juce::AudioProcessor,
                               public juce::AudioProcessorValueTreeState::Listener
{
public:
    RippleatorAudioProcessor();
    ~RippleatorAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;
    
    void parameterChanged(const juce::String& parameterID, float newValue) override;
    
    Chamber& getChamber();
    juce::AudioProcessorValueTreeState& getParameters();
    
    // Get the current level for a microphone (0-1 range)
    float getMicrophoneLevel(int micIndex) const {return 0.5;}
    
    // Helper method to update microphone levels
    void updateMicrophoneLevel(int micIndex, float level);
    
    void setMicrophonePosition(int index, float x, float y);
    void setMicrophoneEnabled(int index, bool enabled);
    
    // Debug controls
    void setBypassProcessing(bool bypass);
    bool isBypassProcessingEnabled() const;
    
private:
    //==============================================================================
    juce::AudioProcessorValueTreeState parameters;
    Chamber chamber;
    
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();
    
    std::array<float, 3> micLevels;
    std::array<float, 3> micLevelSmoothed;
    std::array<bool, 3> microphoneEnabled;
    float levelDecayRate;
    
    // Phase accumulators for test tone generation
    double phase440Hz = 0.0;
    double phase880Hz = 0.0;
    double phase1760Hz = 0.0;
    
    bool bypassProcessing = false;
    
    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (RippleatorAudioProcessor)
};
