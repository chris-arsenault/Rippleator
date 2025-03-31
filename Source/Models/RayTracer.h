#pragma once

#include <JuceHeader.h>
#include <vector>
#include <array>
#include "MicFrequencyBands.h"

// forward declaration
class Chamber;
/**
 * This class handles the ray tracing methods for the Chamber class
 */

// Ray tracing structures
struct Ray {
    juce::Point<float> origin;
    juce::Point<float> direction;
    float intensity = 1.0f;
    float distance = 0.0f;
    int bounceCount = 0;
    MicFrequencyBands frequencyBands;

    Ray(const juce::Point<float>& origin, const juce::Point<float>& direction)
        : origin(origin), direction(direction) {
        // Initialize frequency bands to 1.0
        frequencyBands = MicFrequencyBands();
        frequencyBands.reset(1.0f);
    }
};

struct Intersection {
    bool hit = false;
    juce::Point<float> point;
    juce::Point<float> normal;
    float distance = 0.0f;
    bool isWall = false;
    int wallIndex = -1;
    int zoneId = -1;
};

class RayTracer
{
public:
    RayTracer();
    ~RayTracer();

    void initialize(Chamber* parentChamber);


    bool RayTracer::isCacheValid() const { return initialized && raysCacheValid && !isProcessing; }
    const std::vector<Ray>& getCachedRays() const { return cachedRays; }
    std::array<MicFrequencyBands, 3>& getMicFrequencyResponses()  { return micFrequencyResponses; }
    void updateRayCache();

private:
    static constexpr int RAYS_PER_REFLECTION = 3;

    bool initialized;
    bool isProcessing;

    // Pointer to chamber to avoid data duplication
    Chamber* chamber;

    // Ray tracing
    bool raysCacheValid;
    std::vector<Ray> cachedRays;

    std::array<MicFrequencyBands, 3> micFrequencyResponses;

    void performFrequencyAnalysis(float input);
    void applyFrequencyEffects();
    void handleWallReflection(int x, int y);
    void handleZoneRefraction(int x, int y);
    void processAudioForMicrophones(const float* input, int numSamples);


    // Ray tracing methods
    Intersection traceRay(const Ray& ray) const;
    float calculateRayContribution(const Ray& ray, const juce::Point<float>& micPosition) const;
    std::vector<Ray> generateReflectionRays(const Ray& ray, const Intersection& intersection) const;
    void updateRayFrequencies(Ray& ray, const Intersection& intersection) const;
    void calculateMicrophoneFrequencyResponses();


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(RayTracer)
};
