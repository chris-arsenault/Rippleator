#include "Chamber.h"
#include "../DebugLogger.h"

// Define M_PI if not already defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Constructor
Chamber::Chamber()
    : speakerX(0.5f),
      speakerY(0.5f),
      initialized(false),
      sampleRate(44100.0),
      currentBlockSize(0),
      currentSampleIndex(0),
      samplesSinceLastFFT(0),
      minSamplesForFFT(0), // Will be calculated in initialize()
      bypassProcessing(false), // Initialize to false by default
      fftSize(1024),
      fftBufferPos(0),
      defaultMediumDensity(1.0f) // Initialize default medium density
{
    DebugLogger::logWithCategory("CHAMBER", "Chamber constructor called");

    // Initialize microphone positions
    micPositions[0] = juce::Point<float>(0.2f, 0.2f);
    micPositions[1] = juce::Point<float>(0.8f, 0.2f);
    micPositions[2] = juce::Point<float>(0.5f, 0.8f);
    
    // Initialize microphone buffers
    for (int i = 0; i < 3; ++i) {
        micBuffers[i].resize(1024, 0.0f);
        micFrequencyResponses[i] = MicFrequencyBands();
        micFFTData[i].resize(FFT_SIZE, std::complex<float>(0.0f, 0.0f));
        previousPhase[i].resize(FFT_SIZE, 0.0f);
    }
    
    // Initialize FFT-related members
    inputBuffer.resize(FFT_SIZE, 0.0f);
    fftWorkspace.resize(FFT_SIZE, std::complex<float>(0.0f, 0.0f));
    fftData.resize(FFT_SIZE * 2, 0.0f); // Double size for real/imaginary parts
    fftOutput.resize(FFT_SIZE, 0.0f);
    windowFunction.resize(FFT_SIZE, 0.0f);
    
    // Create Hann window function
    for (int i = 0; i < FFT_SIZE; ++i) {
        windowFunction[i] = 0.5f * (1.0f - std::cos(2.0f * M_PI * i / (FFT_SIZE - 1)));
    }
    
    // Initialize FFT objects
    fftForward = std::make_unique<juce::dsp::FFT>(std::log2(FFT_SIZE));
    fftInverse = std::make_unique<juce::dsp::FFT>(std::log2(FFT_SIZE));

    rayTracer.initialize(this);
    
    DebugLogger::logWithCategory("CHAMBER", "Chamber constructor completed");
}

Chamber::~Chamber()
{
    // Clean up any resources
}

void Chamber::initialize(double sampleRate, float speakerX, float speakerY)
{
    DebugLogger::logWithCategory("CHAMBER", "Chamber initialize called with sampleRate: " + 
                                 std::to_string(sampleRate) + ", speakerX: " + 
                                 std::to_string(speakerX) + ", speakerY: " + 
                                 std::to_string(speakerY));
    
    this->sampleRate = sampleRate;
    setSpeakerPosition(speakerX, speakerY);
    
    // Calculate minimum samples needed for FFT processing
    // For a good frequency resolution down to 50Hz, we need at least sampleRate / 50 samples
    // But we also want to keep latency low, so we'll use a fraction of that
    minSamplesForFFT = static_cast<int>(sampleRate / 50.0 / 4.0); // 1/4 of the samples needed for 50Hz resolution
    
    // Ensure it's not too small or too large
    minSamplesForFFT = juce::jlimit(256, FFT_SIZE / 2, minSamplesForFFT);
    
    DebugLogger::logWithCategory("CHAMBER", "Minimum samples for FFT set to: " + std::to_string(minSamplesForFFT));
    
    // Reset FFT sample counter
    samplesSinceLastFFT = 0;

    //Recalc rays and store frequency responses
    micFrequencyResponses = rayTracer.updateRayCache();
    
    initialized = true;
    
    DebugLogger::logWithCategory("CHAMBER", "Chamber initialization completed");
}

void Chamber::setSpeakerPosition(float x, float y)
{
    DebugLogger::logWithCategory("CHAMBER", "Setting speaker position to (" + 
                                 std::to_string(x) + ", " + std::to_string(y) + ")");
    
    // Clamp to 0-1 range
    x = juce::jlimit(0.0f, 1.0f, x);
    y = juce::jlimit(0.0f, 1.0f, y);
    
    speakerX = x;
    speakerY = y;

    //Recalc rays and store frequency responses
    micFrequencyResponses = rayTracer.updateRayCache();
}

void Chamber::setMicrophonePosition(int index, float x, float y)
{
    if (index < 0 || index >= 3)
        return;
    
    DebugLogger::logWithCategory("CHAMBER", "Setting microphone " + std::to_string(index) + 
                                 " position to (" + std::to_string(x) + ", " + 
                                 std::to_string(y) + ")");
    
    // Clamp to 0-1 range
    x = juce::jlimit(0.0f, 1.0f, x);
    y = juce::jlimit(0.0f, 1.0f, y);
    
    micPositions[index] = {x, y};

    //Recalc rays and store frequency responses
    micFrequencyResponses = rayTracer.updateRayCache();
}

void Chamber::setBypassProcessing(bool bypass)
{
    bypassProcessing = bypass;
}

void Chamber::processBlock(const float* input, int numSamples)
{
    if (!initialized)
    {
        DebugLogger::logWithCategory("ERROR", "Chamber not initialized before processBlock call");
        return;
    }
    
    if (bypassProcessing)
    {
        // If bypass is enabled, just copy the input to the output
        for (int i = 0; i < 3; ++i) {
            if (micBuffers[i].size() < static_cast<size_t>(numSamples)) {
                micBuffers[i].resize(numSamples, 0.0f);
            }
            std::copy(input, input + numSamples, micBuffers[i].begin());
        }
        return;
    }
    
    // Resize microphone buffers if needed
    for (int i = 0; i < 3; ++i) {
        if (micBuffers[i].size() < static_cast<size_t>(numSamples)) {
            micBuffers[i].resize(numSamples, 0.0f);
        }
        std::fill(micBuffers[i].begin(), micBuffers[i].end(), 0.0f);
    }
    
    // Update current block size
    currentBlockSize = numSamples;
    
    // Process audio for each microphone in one pass
    processAudioForMicrophones(input, numSamples);
}

void Chamber::processAudioForMicrophones(const float* input, int numSamples)
{
    DebugLogger::logWithCategory("CHAMBER", "Processing audio for microphones");
    
    // Copy input to buffer with overlap
    for (int i = 0; i < numSamples; ++i) {
        inputBuffer[fftBufferPos] = input[i];
        fftBufferPos = (fftBufferPos + 1) % FFT_SIZE;
    }
    
    // Increment the sample counter
    samplesSinceLastFFT += numSamples;
    
    // Only process FFT if we've accumulated enough samples since the last processing
    // This ensures we have enough data for proper frequency domain conversion
    bool shouldProcessFFT = samplesSinceLastFFT >= minSamplesForFFT;
    
    if (shouldProcessFFT) {
        DebugLogger::logWithCategory("CHAMBER", "Processing FFT after " + 
                                    std::to_string(samplesSinceLastFFT) + " samples");
        
        // Reset the counter
        samplesSinceLastFFT = 0;
        
        // Process each microphone
        for (int mic = 0; mic < 3; ++mic) {
            // Prepare FFT input buffer with windowing
            for (int i = 0; i < FFT_SIZE; ++i) {
                int bufferIndex = (fftBufferPos - numSamples + i + FFT_SIZE) % FFT_SIZE;
                fftWorkspace[i] = std::complex<float>(inputBuffer[bufferIndex] * windowFunction[i], 0.0f);
            }
            
            // Perform forward FFT
            fftForward->perform(fftWorkspace.data(), fftWorkspace.data(), false);
            
            // Apply frequency-specific attenuation while preserving phase
            for (int i = 0; i < FFT_SIZE / 2; ++i) {
                // Map FFT bin to frequency band
                float frequency = i * (static_cast<float>(sampleRate) / FFT_SIZE);

                // Get attenuation for this frequency band
                float attenuation = micFrequencyResponses[mic].getBandForFrequency(frequency).value;
                
                // Extract magnitude and phase
                float magnitude = std::abs(fftWorkspace[i]);
                float phase = std::arg(fftWorkspace[i]);
                
                // Calculate phase difference from previous block for phase continuity
                float phaseDelta = phase - previousPhase[mic][i];
                
                // Unwrap phase to avoid discontinuities
                while (phaseDelta > M_PI) phaseDelta -= 2.0f * M_PI;
                while (phaseDelta < -M_PI) phaseDelta += 2.0f * M_PI;
                
                // Update phase with smooth transition
                phase = previousPhase[mic][i] + phaseDelta;
                
                // Store current phase for next block
                previousPhase[mic][i] = phase;
                
                // Apply attenuation to magnitude only
                magnitude *= attenuation;
                
                // Convert back to complex using preserved phase
                fftWorkspace[i] = std::polar(magnitude, phase);
                
                // Handle the symmetrical part (except DC and Nyquist)
                if (i > 0 && i < FFT_SIZE / 2) {
                    // For the symmetrical part, we need conjugate symmetry
                    float symmetricalPhase = -phase; // Conjugate phase
                    
                    // Store phase for symmetrical component
                    previousPhase[mic][FFT_SIZE - i] = symmetricalPhase;
                    
                    // Apply attenuation and set with conjugate phase
                    fftWorkspace[FFT_SIZE - i] = std::polar(magnitude, symmetricalPhase);
                }
            }
            
            // Perform inverse FFT to get time-domain signal
            fftInverse->perform(fftWorkspace.data(), fftWorkspace.data(), true);
            
            // Extract real part and apply window function to reduce artifacts
            // Only extract the last numSamples values that correspond to our input block
            for (int i = 0; i < numSamples; ++i) {
                // Calculate the correct index in the FFT output
                int fftIndex = FFT_SIZE - numSamples + i;
                
                // Apply window function and scale
                float sample = fftWorkspace[fftIndex].real() * windowFunction[fftIndex] / FFT_SIZE;
                
                // Apply a gentle soft-clipping to prevent harsh digital distortion
                sample = std::tanh(sample * 0.8f);
                
                // Store the output in the buffer
                micBuffers[mic][i] = sample;
            }
        }
    } else {
        // If we're not processing FFT this time, we still need to fill the output buffers
        // We'll use the previous output values to avoid discontinuities

        // For each microphone, copy the last processed samples
        for (int mic = 0; mic < 3; ++mic) {
            // If we have previous samples, use them
            if (!micBuffers[mic].empty()) {
                // Get the last sample value for smooth transition
                float lastSample = micBuffers[mic].back();

                // Fill the buffer with a smooth decay from the last sample
                for (int i = 0; i < numSamples; ++i) {
                    // Apply a gentle decay to avoid clicks
                    float decayFactor = std::pow(0.99f, static_cast<float>(i));
                    micBuffers[mic][i] = lastSample * decayFactor;
                }
            }
        }
    }
    
    DebugLogger::logWithCategory("CHAMBER", "Audio processing for microphones completed");
}

void Chamber::getMicrophoneOutputBlock(int micIndex, float* outputBuffer, int numSamples) const
{
    if (micIndex < 0 || micIndex >= 3 || !outputBuffer)
        return;
    
    DebugLogger::logWithCategory("CHAMBER", "Getting microphone output block for microphone " + std::to_string(micIndex));
    
    // Copy the pre-calculated microphone output directly from the buffer
    // This is more efficient than calling getMicrophoneOutput for each sample
    const int samplesToCopy = std::min(numSamples, currentBlockSize);
    if (samplesToCopy > 0 && micBuffers[micIndex].size() >= static_cast<size_t>(samplesToCopy)) {
        std::copy(micBuffers[micIndex].begin(), 
                 micBuffers[micIndex].begin() + samplesToCopy, 
                 outputBuffer);
    } else {
        // Zero out the buffer if we don't have valid data
        std::fill(outputBuffer, outputBuffer + numSamples, 0.0f);
    }
    
    DebugLogger::logWithCategory("CHAMBER", "Microphone output block retrieved");
}

juce::Point<float> Chamber::getSpeakerPosition() const
{
    return juce::Point<float>(speakerX, speakerY);
}

juce::Point<float> Chamber::getMicrophonePosition(int index) const
{
    if (index >= 0 && index < 3)
        return micPositions[index];
    
    // Return a default position if index is out of range
    return {0.5f, 0.5f};
}

int Chamber::addZone(float x, float y, float width, float height, float density)
{
    DebugLogger::logWithCategory("CHAMBER", "Adding zone at (" + std::to_string(x) + ", " + std::to_string(y) + 
                                 ") with width " + std::to_string(width) + ", height " + std::to_string(height) + 
                                 ", and density " + std::to_string(density));
    
    auto zone = std::make_unique<Zone>();
    zone->x = x;
    zone->y = y;
    zone->width = width;
    zone->height = height;
    zone->density = density;
    
    zones.push_back(std::move(zone));

    //Recalc rays and store frequency responses
    micFrequencyResponses = rayTracer.updateRayCache();

    DebugLogger::logWithCategory("CHAMBER", "Zone added");
    
    return zones.size() - 1;
}

void Chamber::removeZone(int index)
{
    if (index >= 0 && index < zones.size())
    {
        DebugLogger::logWithCategory("CHAMBER", "Removing zone at index " + std::to_string(index));
        
        zones.erase(zones.begin() + index);

        //Recalc rays and store frequency responses
        micFrequencyResponses = rayTracer.updateRayCache();
    }
}

void Chamber::setZoneDensity(int index, float density)
{
    if (index >= 0 && index < zones.size())
    {
        DebugLogger::logWithCategory("CHAMBER", "Setting zone density at index " + std::to_string(index) + 
                                     " to " + std::to_string(density));
        
        zones[index]->density = density;

        //Recalc rays and store frequency responses
        micFrequencyResponses = rayTracer.updateRayCache();
    }
}

void Chamber::setZoneBounds(int index, float x, float y, float width, float height)
{
    if (index >= 0 && index < zones.size())
    {
        DebugLogger::logWithCategory("CHAMBER", "Setting zone bounds at index " + std::to_string(index) + 
                                     " to (" + std::to_string(x) + ", " + std::to_string(y) + 
                                     ") with width " + std::to_string(width) + ", height " + std::to_string(height));
        
        zones[index]->x = juce::jlimit(0.0f, 1.0f, x);
        zones[index]->y = juce::jlimit(0.0f, 1.0f, y);
        zones[index]->width = juce::jlimit(0.0f, 1.0f - zones[index]->x, width);
        zones[index]->height = juce::jlimit(0.0f, 1.0f - zones[index]->y, height);

        //Recalc rays and store frequency responses
        micFrequencyResponses = rayTracer.updateRayCache();
    }
}

const std::vector<std::unique_ptr<Zone>>& Chamber::getZones() const
{
    return zones;
}


bool Chamber::isInitialized() const
{
    return initialized;
}

void Chamber::setDefaultMediumDensity(float density)
{
    DebugLogger::logWithCategory("CHAMBER", "Setting default medium density to " + std::to_string(density));
    defaultMediumDensity = density;

    //Recalc rays and store frequency responses
    micFrequencyResponses = rayTracer.updateRayCache();
}

float Chamber::getDefaultMediumDensity() const
{
    return defaultMediumDensity;
}