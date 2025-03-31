#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "DebugLogger.h"

// Define M_PI if not already defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout RippleatorAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "mediumDensity",
        "Medium Density",
        juce::NormalisableRange<float>(0.1f, 10.0f, 0.1f),
        1.0f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "wallReflectivity",
        "Wall Reflectivity",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.5f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "wallDamping",
        "Wall Damping",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        0.2f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "outputGain",
        "Output Gain",
        juce::NormalisableRange<float>(0.0f, 2.0f, 0.01f),
        1.0f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "mic1Volume",
        "Mic 1 Volume",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        1.0f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "mic2Volume",
        "Mic 2 Volume",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        1.0f));
    
    params.push_back(std::make_unique<juce::AudioParameterFloat>(
        "mic3Volume",
        "Mic 3 Volume",
        juce::NormalisableRange<float>(0.0f, 1.0f, 0.01f),
        1.0f));
    
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "mic1Solo",
        "Mic 1 Solo",
        false));
    
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "mic2Solo",
        "Mic 2 Solo",
        false));
    
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "mic3Solo",
        "Mic 3 Solo",
        false));
    
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "mic1Mute",
        "Mic 1 Mute",
        false));
    
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "mic2Mute",
        "Mic 2 Mute",
        false));
    
    params.push_back(std::make_unique<juce::AudioParameterBool>(
        "mic3Mute",
        "Mic 3 Mute",
        false));
    
    return { params.begin(), params.end() };
}

//==============================================================================
RippleatorAudioProcessor::RippleatorAudioProcessor()
    : AudioProcessor(BusesProperties()
                     .withInput("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
      parameters(*this, nullptr, juce::Identifier("Rippleator"), createParameterLayout()),
      levelDecayRate(0.9f),
      phase440Hz(0.0f),
      phase880Hz(0.0f),
      phase1760Hz(0.0f),
      bypassProcessing(false),
      micLevels{0.0f, 0.0f, 0.0f},
      micLevelSmoothed{0.0f, 0.0f, 0.0f},
      microphoneEnabled{true, true, true}
{
    // Initialize debug logger
    DebugLogger::initialize();
    DebugLogger::logWithCategory("INIT", "RippleatorAudioProcessor constructor start");
    
    // Initialize chamber parameters
    try {
        DebugLogger::logWithCategory("INIT", "Initializing chamber with sample rate: " + std::to_string(getSampleRate()));
        chamber.initialize(0.0f, 0.5f);  // Speaker on left wall
        DebugLogger::logWithCategory("INIT", "Chamber initialized successfully");
    }
    catch (const std::exception& e) {
        DebugLogger::logWithCategory("ERROR", "Exception during chamber initialization: " + std::string(e.what()));
    }
    catch (...) {
        DebugLogger::logWithCategory("ERROR", "Unknown exception during chamber initialization");
    }
    
    // Set up parameter listeners
    DebugLogger::logWithCategory("INIT", "Setting up parameter listeners");
    parameters.addParameterListener("mediumDensity", this);
    parameters.addParameterListener("wallReflectivity", this);
    parameters.addParameterListener("wallDamping", this);
    
    // Initialize microphone positions
    DebugLogger::logWithCategory("INIT", "Setting microphone positions");
    chamber.setMicrophonePosition(0, 0.75f, 0.25f);  // Top right
    chamber.setMicrophonePosition(1, 0.75f, 0.5f);   // Middle right
    chamber.setMicrophonePosition(2, 0.75f, 0.75f);  // Bottom right
    
    DebugLogger::logWithCategory("INIT", "RippleatorAudioProcessor constructor completed");
}

RippleatorAudioProcessor::~RippleatorAudioProcessor()
{
    parameters.removeParameterListener("mediumDensity", this);
    parameters.removeParameterListener("wallReflectivity", this);
    parameters.removeParameterListener("wallDamping", this);
}

void RippleatorAudioProcessor::parameterChanged(const juce::String& parameterID, float newValue)
{
    // Handle parameter changes
    juce::Logger::writeToLog("Parameter changed: " + parameterID + " = " + juce::String(newValue));
    
    // Update chamber properties based on parameter changes
    if (parameterID == "mediumDensity")
    {
        chamber.setDefaultMediumDensity(newValue);
    }
    
    // If we need to add zone-specific properties, we can use the Chamber's zone management methods:
    // For example: chamber.setZoneProperty(zoneIndex, newValue);
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
    DebugLogger::logWithCategory("AUDIO", "prepareToPlay called with sampleRate: " + std::to_string(sampleRate) + 
                            ", samplesPerBlock: " + std::to_string(samplesPerBlock));
    
    try {
        chamber.initialize(0.0f, 0.5f);
        DebugLogger::logWithCategory("AUDIO", "Chamber reinitialized in prepareToPlay");
        
        // Set the default medium density from the parameter
        float mediumDensity = *parameters.getRawParameterValue("mediumDensity");
        chamber.setDefaultMediumDensity(mediumDensity);
        DebugLogger::logWithCategory("AUDIO", "Medium density set to: " + std::to_string(mediumDensity));
        
        // Reset level meters
        for (int i = 0; i < 3; ++i)
        {
            micLevels[i] = 0.0f;
            micLevelSmoothed[i] = 0.0f;
        }
        DebugLogger::logWithCategory("AUDIO", "Level meters reset");
    }
    catch (const std::exception& e) {
        DebugLogger::logWithCategory("ERROR", "Exception in prepareToPlay: " + std::string(e.what()));
    }
    catch (...) {
        DebugLogger::logWithCategory("ERROR", "Unknown exception in prepareToPlay");
    }
}

void RippleatorAudioProcessor::releaseResources()
{
}

bool RippleatorAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    // Only support mono input and stereo output
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo()
        || layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

void RippleatorAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer,
                                          juce::MidiBuffer& midiMessages)
{
    static bool firstProcessBlock = true;
    if (firstProcessBlock) {
        DebugLogger::logWithCategory("AUDIO", "First processBlock call");
        firstProcessBlock = false;
    }
    
    juce::ScopedNoDenormals noDenormals;
    chamber.setSampleRate(getSampleRate());
    
    try {
        // Only log occasionally to avoid filling the log file
        static int processBlockCounter = 0;
        if (processBlockCounter++ % 100 == 0) {
            DebugLogger::logWithCategory("AUDIO", "processBlock called (iteration " + std::to_string(processBlockCounter) + ")");
        }
        
        auto numSamples = buffer.getNumSamples();
        auto sampleRate = getSampleRate();

        // Clear the output buffer
        buffer.clear();

        // Create a buffer for test tones
        juce::AudioBuffer<float> testToneBuffer(1, numSamples);
        float* testToneData = testToneBuffer.getWritePointer(0);

        // Generate test tones (440Hz, 880Hz, and 1760Hz)
        for (int i = 0; i < numSamples; ++i)
        {
            testToneData[i] =  2.0f * (phase440Hz - std::floor(phase440Hz + 0.5f));
            // Update phases
            phase440Hz += 440.0 / sampleRate;
            if (phase440Hz >= 1.0) phase440Hz -= 1.0;

        }

        // Apply level decay to all microphones
        for (int i = 0; i < 3; ++i)
        {
            micLevels[i] *= levelDecayRate; // Apply decay rate
        }

        // Process through chamber using test tones instead of input data
        if (!bypassProcessing)
        {
            // Make sure bypass is disabled
            chamber.setBypassProcessing(false);
        }
        else
        {
            // When bypass is enabled, just use the test tone directly
            // This will allow us to hear the unprocessed test tone
            chamber.setBypassProcessing(true);
        }
        chamber.processBlock(testToneData, numSamples);

        // Mix microphone outputs
        bool anySolo = *parameters.getRawParameterValue("mic1Solo") > 0.5f ||
                       *parameters.getRawParameterValue("mic2Solo") > 0.5f ||
                       *parameters.getRawParameterValue("mic3Solo") > 0.5f;

        // Get microphone parameters
        float mic1Vol = *parameters.getRawParameterValue("mic1Volume");
        float mic2Vol = *parameters.getRawParameterValue("mic2Volume");
        float mic3Vol = *parameters.getRawParameterValue("mic3Volume");
        bool mic1Solo = *parameters.getRawParameterValue("mic1Solo") > 0.5f;
        bool mic2Solo = *parameters.getRawParameterValue("mic2Solo") > 0.5f;
        bool mic3Solo = *parameters.getRawParameterValue("mic3Solo") > 0.5f;
        bool mic1Mute = *parameters.getRawParameterValue("mic1Mute") > 0.5f;
        bool mic2Mute = *parameters.getRawParameterValue("mic2Mute") > 0.5f;
        bool mic3Mute = *parameters.getRawParameterValue("mic3Mute") > 0.5f;

        // Get output buffer pointers
        float* leftChannel = buffer.getWritePointer(0);
        float* rightChannel = buffer.getWritePointer(1);

        // Create temporary buffers for microphone outputs
        juce::AudioBuffer<float> mic1Buffer(1, numSamples);
        juce::AudioBuffer<float> mic2Buffer(1, numSamples);
        juce::AudioBuffer<float> mic3Buffer(1, numSamples);
        
        float* mic1Data = mic1Buffer.getWritePointer(0);
        float* mic2Data = mic2Buffer.getWritePointer(0);
        float* mic3Data = mic3Buffer.getWritePointer(0);

        chamber.getMicrophoneOutputBlock(0, mic1Data, numSamples);
        chamber.getMicrophoneOutputBlock(1, mic2Data, numSamples);
        chamber.getMicrophoneOutputBlock(2, mic3Data, numSamples);


        // Update level meters with peak values
        for (int i = 0; i < numSamples; ++i) {
            updateMicrophoneLevel(0, std::abs(mic1Data[i]));
            updateMicrophoneLevel(1, std::abs(mic2Data[i]));
            updateMicrophoneLevel(2, std::abs(mic3Data[i]));
        }

        // Process each sample in the block
        for (int i = 0; i < numSamples; ++i)
        {
            // Get microphone outputs
            float mic1Output = mic1Data[i];
            float mic2Output = mic2Data[i];
            float mic3Output = mic3Data[i];
            
            // Apply volume
            mic1Output *= mic1Vol;
            mic2Output *= mic2Vol;
            mic3Output *= mic3Vol;
            
            // Apply solo/mute
            if (anySolo)
            {
                // Solo mode
                if (!mic1Solo) mic1Output = 0.0f;
                if (!mic2Solo) mic2Output = 0.0f;
                if (!mic3Solo) mic3Output = 0.0f;
            }
            else
            {
                // Mute mode
                if (mic1Mute) mic1Output = 0.0f;
                if (mic2Mute) mic2Output = 0.0f;
                if (mic3Mute) mic3Output = 0.0f;
            }
            
            // Mix to stereo (simple panning)
            leftChannel[i] = 0.7f * mic1Output + 0.5f * mic2Output + 0.3f * mic3Output;
            rightChannel[i] = 0.3f * mic1Output + 0.5f * mic2Output + 0.7f * mic3Output;
        }

        // Apply output gain
        float outputGain = *parameters.getRawParameterValue("outputGain");
        buffer.applyGain(outputGain);
        
        if (processBlockCounter % 1000 == 0) {
            DebugLogger::logWithCategory("AUDIO", "processBlock completed successfully");
        }
    }
    catch (const std::exception& e) {
        DebugLogger::logWithCategory("ERROR", "Exception in processBlock: " + std::string(e.what()));
    }
    catch (...) {
        DebugLogger::logWithCategory("ERROR", "Unknown exception in processBlock");
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
        
        // Apply smoothing for the meter display
        const float smoothingFactor = 0.7f;
        micLevelSmoothed[micIndex] = smoothingFactor * micLevelSmoothed[micIndex] + 
                                    (1.0f - smoothingFactor) * micLevels[micIndex];
    }
}

void RippleatorAudioProcessor::setMicrophonePosition(int index, float x, float y)
{
    chamber.setMicrophonePosition(index, x, y);
}

void RippleatorAudioProcessor::setMicrophoneEnabled(int index, bool enabled)
{
    if (index >= 0 && index < 3)
    {
        microphoneEnabled[index] = enabled;
    }
}

void RippleatorAudioProcessor::setBypassProcessing(bool bypass)
{
    bypassProcessing = bypass;
    chamber.setBypassProcessing(bypass);
}

bool RippleatorAudioProcessor::isBypassProcessingEnabled() const
{
    return bypassProcessing;
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
