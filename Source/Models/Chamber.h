#pragma once

#include <JuceHeader.h>
#include <vector>
#include <memory>
#include <array>
#include "Zone.h"
#include "RayTracer.h"

/**
 * Chamber class that simulates a 2D rectangular chamber filled with multiple fluid/gas zones.
 * This class implements the physical modeling of sound wave propagation through
 * different mediums, with configurable speaker and microphone positions.
 */

class Chamber
{
public:
    static constexpr int FFT_SIZE = 1024;    // Size of FFT for frequency analysis
    
    Chamber();
    ~Chamber();
    
    void initialize(double sampleRate, float speakerX, float speakerY);
    void processBlock(const float* input, int numSamples);
    void setMicrophonePosition(int index, float x, float y);
    float getMicrophoneOutput(int index) const;
    void getMicrophoneOutputBlock(int micIndex, float* outputBuffer, int numSamples) const;
    
    // Parameter setters
    void setMediumDensity(float density);
    void setWallReflectivity(float reflectivity);
    void setWallDamping(float damping);
    
    // Zone management
    int addZone(float x1, float y1, float x2, float y2, float density);
    void removeZone(int zoneId);
    void setZoneProperty(int index, float density);
    void setZoneDensity(int zoneId, float density);
    void setZoneBounds(int zoneId, float x1, float y1, float x2, float y2);
    const std::vector<std::unique_ptr<Zone>>& getZones() const;
    
    // Microphone management
    const std::array<juce::Point<float>, 3>& getMicrophonePositions() const { return micPositions; }
    
    // Speaker position
    float getSpeakerX() const { return speakerX; }
    float getSpeakerY() const { return speakerY; }
    void setSpeakerPosition(float x, float y);
    juce::Point<float> getSpeakerPosition() const;

    const std::vector<Ray>& getCachedRays() const { return rayTracer.getCachedRays(); }
    bool isInitialized() const;
    void setDefaultMediumDensity(float density);
    float getDefaultMediumDensity() const;
    juce::Point<float> getMicrophonePosition(int index) const;
    
    // Getter for microphone frequency responses (for visualization)
    const std::array<MicFrequencyBands, 3>& getMicFrequencyResponses() const { return micFrequencyResponses; }
    
    // Getter for microphone output buffer (for visualization)
    const std::array<std::vector<float>, 3>& getMicBuffers() const { return micBuffers; }

    void setBypassProcessing(bool bypass);

private:

    void processAudioForMicrophones(const float* input, int numSamples);
    
    // Ray tracing
    float defaultMediumDensity;
    RayTracer rayTracer;
    
    // Microphone frequency responses
    std::array<MicFrequencyBands, 3> micFrequencyResponses;
    
    // FFT data for each microphone
    std::array<std::vector<std::complex<float>>, 3> micFFTData;
    
    // Previous phase information for phase continuity
    std::array<std::vector<float>, 3> previousPhase;
    
    // Microphone output buffers
    std::array<std::vector<float>, 3> micBuffers;
    
    // Current block size and sample index
    int currentBlockSize;
    int currentSampleIndex;
    
    // FFT processing control
    int samplesSinceLastFFT;
    int minSamplesForFFT;
    
    // Debug control
    bool bypassProcessing; // When true, input is copied directly to output without processing
    
    // FFT objects
    std::unique_ptr<juce::dsp::FFT> fftForward;
    std::unique_ptr<juce::dsp::FFT> fftInverse;
    
    // Input buffer for FFT processing
    std::vector<float> inputBuffer;
    std::vector<std::complex<float>> fftWorkspace;
    
    bool initialized;
    double sampleRate;
    float speakerX;
    float speakerY;
    
    // Chamber parameters
    float mediumDensity;
    float wallReflectivity;
    float wallDamping;
    
    // FFT processing
    int fftSize;
    std::vector<float> fftData;
    std::vector<float> fftOutput;
    std::vector<float> windowFunction;
    int fftBufferPos;

    // Audio processing
    std::vector<float> fftInputBuffer;      // Buffer for FFT input
    std::vector<float> fftTimeBuffer;       // Time domain buffer
    std::vector<float> fftFrequencyBuffer;  // Frequency domain buffer
    
    // Zones
    std::vector<std::unique_ptr<Zone>> zones;
    int nextZoneId;
    
    // Microphone positions (up to 3)
    std::array<juce::Point<float>, 3> micPositions;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Chamber)
};
