#include "PluginProcessor.h"
#include "PluginEditor.h"

RippleatorAudioProcessor::RippleatorAudioProcessor()
    : AudioProcessor(BusesProperties()
                    .withInput("Input", juce::AudioChannelSet::mono(), true)
                    .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, juce::Identifier("Rippleator"),
                {
                    std::make_unique<juce::AudioParameterFloat>(
                        "mediumDensity",
                        "Medium Density",
                        juce::NormalisableRange<float>(0.1f, 10.0f, 0.1f),
                        1.0f),
                    std::make_unique<juce::AudioParameterFloat>(
                        "wallReflectivity",
                        "Wall Reflectivity",
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                        0.5f),
                    std::make_unique<juce::AudioParameterFloat>(
                        "wallDamping",
                        "Wall Damping",
                        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
                        0.1f),
                    std::make_unique<juce::AudioParameterFloat>(
                        "mic1Volume",
                        "Mic 1 Volume",
                        juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f),
                        1.0f),
                    std::make_unique<juce::AudioParameterFloat>(
                        "mic1Pan",
                        "Mic 1 Pan",
                        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f),
                        -1.0f),  // Default to full left
                    std::make_unique<juce::AudioParameterBool>(
                        "mic1Solo",
                        "Mic 1 Solo",
                        false),
                    std::make_unique<juce::AudioParameterBool>(
                        "mic1Mute",
                        "Mic 1 Mute",
                        false),
                    std::make_unique<juce::AudioParameterFloat>(
                        "mic2Volume",
                        "Mic 2 Volume",
                        juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f),
                        1.0f),
                    std::make_unique<juce::AudioParameterFloat>(
                        "mic2Pan",
                        "Mic 2 Pan",
                        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f),
                        0.0f),  // Default to center
                    std::make_unique<juce::AudioParameterBool>(
                        "mic2Solo",
                        "Mic 2 Solo",
                        false),
                    std::make_unique<juce::AudioParameterBool>(
                        "mic2Mute",
                        "Mic 2 Mute",
                        false),
                    std::make_unique<juce::AudioParameterFloat>(
                        "mic3Volume",
                        "Mic 3 Volume",
                        juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f),
                        1.0f),
                    std::make_unique<juce::AudioParameterFloat>(
                        "mic3Pan",
                        "Mic 3 Pan",
                        juce::NormalisableRange<float>(-1.0f, 1.0f, 0.01f),
                        1.0f),  // Default to full right
                    std::make_unique<juce::AudioParameterBool>(
                        "mic3Solo",
                        "Mic 3 Solo",
                        false),
                    std::make_unique<juce::AudioParameterBool>(
                        "mic3Mute",
                        "Mic 3 Mute",
                        false),
                    std::make_unique<juce::AudioParameterFloat>(
                        "masterVolume",
                        "Master Volume",
                        juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f),
                        1.0f)
                }),
      micLevels{0.0f, 0.0f, 0.0f},
      micLevelSmoothed{0.0f, 0.0f, 0.0f},
      levelDecayRate(0.7f)
{
    // Initialize chamber parameters
    chamber.initialize(getSampleRate(), 0.0f, 0.5f);  // Speaker on left wall
    
    // Set up parameter listeners
    parameters.addParameterListener("mediumDensity", this);
    parameters.addParameterListener("wallReflectivity", this);
    parameters.addParameterListener("wallDamping", this);
    
    // Initialize microphone positions
    chamber.setMicrophonePosition(0, 0.75f, 0.25f);  // Top right
    chamber.setMicrophonePosition(1, 0.75f, 0.5f);   // Middle right
    chamber.setMicrophonePosition(2, 0.75f, 0.75f);  // Bottom right
}

RippleatorAudioProcessor::~RippleatorAudioProcessor()
{
    parameters.removeParameterListener("mediumDensity", this);
    parameters.removeParameterListener("wallReflectivity", this);
    parameters.removeParameterListener("wallDamping", this);
}

void RippleatorAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    if (parameterID == "mediumDensity")
        chamber.setMediumDensity(newValue);
    else if (parameterID == "wallReflectivity")
        chamber.setWallReflectivity(newValue);
    else if (parameterID == "wallDamping")
        chamber.setWallDamping(newValue);
}

const juce::String RippleatorAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool RippleatorAudioProcessor::acceptsMidi() const
{
    return false;
}

bool RippleatorAudioProcessor::producesMidi() const
{
    return false;
}

bool RippleatorAudioProcessor::isMidiEffect() const
{
    return false;
}

double RippleatorAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int RippleatorAudioProcessor::getNumPrograms()
{
    return 1;
}

int RippleatorAudioProcessor::getCurrentProgram()
{
    return 0;
}

void RippleatorAudioProcessor::setCurrentProgram(int index)
{
}

const juce::String RippleatorAudioProcessor::getProgramName(int index)
{
    return {};
}

void RippleatorAudioProcessor::changeProgramName(int index, const juce::String& newName)
{
}

void RippleatorAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    chamber.initialize(sampleRate, 0.0f, 0.5f);
    
    // Reset level meters
    for (int i = 0; i < 3; ++i)
    {
        micLevels[i] = 0.0f;
        micLevelSmoothed[i] = 0.0f;
    }
}

void RippleatorAudioProcessor::releaseResources()
{
}

bool RippleatorAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // Only support mono input and stereo output
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo()
        || layouts.getMainInputChannelSet() != juce::AudioChannelSet::mono())
        return false;

    return true;
}

void RippleatorAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                          juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear any output channels that don't contain input data
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    // Get buffer pointers
    auto* channelData = buffer.getWritePointer(0);
    auto numSamples = buffer.getNumSamples();

    // Create a temporary buffer to store the input
    juce::AudioBuffer<float> inputBuffer;
    inputBuffer.makeCopyOf(buffer);
    auto* inputData = inputBuffer.getReadPointer(0);

    // Clear the output buffer
    buffer.clear();

    // Apply level decay to all microphones
    for (int i = 0; i < 3; ++i)
    {
        micLevels[i] *= levelDecayRate; // Apply decay rate
    }

    // Process through chamber
    chamber.processBlock(inputData, numSamples);

    // Mix microphone outputs
    bool anySolo = *parameters.getRawParameterValue("mic1Solo") > 0.5f ||
                   *parameters.getRawParameterValue("mic2Solo") > 0.5f ||
                   *parameters.getRawParameterValue("mic3Solo") > 0.5f;

    // Get microphone parameters
    float mic1Vol = *parameters.getRawParameterValue("mic1Volume");
    float mic2Vol = *parameters.getRawParameterValue("mic2Volume");
    float mic3Vol = *parameters.getRawParameterValue("mic3Volume");
    
    bool mic1Mute = *parameters.getRawParameterValue("mic1Mute") > 0.5f;
    bool mic2Mute = *parameters.getRawParameterValue("mic2Mute") > 0.5f;
    bool mic3Mute = *parameters.getRawParameterValue("mic3Mute") > 0.5f;
    
    bool mic1Solo = *parameters.getRawParameterValue("mic1Solo") > 0.5f;
    bool mic2Solo = *parameters.getRawParameterValue("mic2Solo") > 0.5f;
    bool mic3Solo = *parameters.getRawParameterValue("mic3Solo") > 0.5f;
    
    float mic1Pan = *parameters.getRawParameterValue("mic1Pan");
    float mic2Pan = *parameters.getRawParameterValue("mic2Pan");
    float mic3Pan = *parameters.getRawParameterValue("mic3Pan");

    // Process each sample
    for (int i = 0; i < numSamples; ++i)
    {
        // Get microphone outputs
        float mic1Out = chamber.getMicrophoneOutput(0);
        float mic2Out = chamber.getMicrophoneOutput(1);
        float mic3Out = chamber.getMicrophoneOutput(2);
        
        // Update level meters
        updateMicrophoneLevel(0, mic1Out);
        updateMicrophoneLevel(1, mic2Out);
        updateMicrophoneLevel(2, mic3Out);
        
        // Apply volume, mute, and solo
        bool mic1Active = (!mic1Mute && (!anySolo || mic1Solo));
        bool mic2Active = (!mic2Mute && (!anySolo || mic2Solo));
        bool mic3Active = (!mic3Mute && (!anySolo || mic3Solo));
        
        float leftSample = 0.0f;
        float rightSample = 0.0f;
        
        if (mic1Active)
        {
            // Calculate pan position (pan = -1 is full left, pan = 1 is full right)
            float leftGain = (mic1Pan <= 0.0f) ? 1.0f : (1.0f - mic1Pan);
            float rightGain = (mic1Pan >= 0.0f) ? 1.0f : (1.0f + mic1Pan);
            
            leftSample += mic1Out * mic1Vol * leftGain;
            rightSample += mic1Out * mic1Vol * rightGain;
        }
        
        if (mic2Active)
        {
            // Calculate pan position (pan = -1 is full left, pan = 1 is full right)
            float leftGain = (mic2Pan <= 0.0f) ? 1.0f : (1.0f - mic2Pan);
            float rightGain = (mic2Pan >= 0.0f) ? 1.0f : (1.0f + mic2Pan);
            
            leftSample += mic2Out * mic2Vol * leftGain;
            rightSample += mic2Out * mic2Vol * rightGain;
        }
        
        if (mic3Active)
        {
            // Calculate pan position (pan = -1 is full left, pan = 1 is full right)
            float leftGain = (mic3Pan <= 0.0f) ? 1.0f : (1.0f - mic3Pan);
            float rightGain = (mic3Pan >= 0.0f) ? 1.0f : (1.0f + mic3Pan);
            
            leftSample += mic3Out * mic3Vol * leftGain;
            rightSample += mic3Out * mic3Vol * rightGain;
        }
        
        // Apply master volume
        leftSample *= *parameters.getRawParameterValue("masterVolume");
        rightSample *= *parameters.getRawParameterValue("masterVolume");
        
        // Write to output channels
        buffer.setSample(0, i, leftSample);
        buffer.setSample(1, i, rightSample);
    }
}

void RippleatorAudioProcessor::updateMicrophoneLevel(int micIndex, float level)
{
    if (micIndex >= 0 && micIndex < 3)
    {
        // Take the absolute value of the level
        level = std::abs(level);
        
        // Update the level if the new level is higher
        if (level > micLevels[micIndex])
        {
            micLevels[micIndex] = level;
        }
        // Otherwise, the decay is handled in processBlock
    }
}

float RippleatorAudioProcessor::getMicrophoneLevel(int micIndex) const
{
    if (micIndex >= 0 && micIndex < 3)
    {
        return micLevelSmoothed[micIndex];
    }
    return 0.0f;
}

bool RippleatorAudioProcessor::hasEditor() const
{
    return true;
}

juce::AudioProcessorEditor* RippleatorAudioProcessor::createEditor()
{
    return new RippleatorAudioProcessorEditor(*this);
}

void RippleatorAudioProcessor::getStateInformation(juce::MemoryBlock& destData)
{
    auto state = parameters.copyState();
    std::unique_ptr<juce::XmlElement> xml(state.createXml());
    copyXmlToBinary(*xml, destData);
}

void RippleatorAudioProcessor::setStateInformation(const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes));
    if (xmlState.get() != nullptr)
        if (xmlState->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xmlState));
}

Chamber& RippleatorAudioProcessor::getChamber()
{
    return chamber;
}

juce::AudioProcessorValueTreeState& RippleatorAudioProcessor::getParameters()
{
    return parameters;
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new RippleatorAudioProcessor();
}
