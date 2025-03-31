#include "RayTracer.h"
#include "Chamber.h"
#include "Zone.h"
#include "../DebugLogger.h"

// Define M_PI if not already defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// Constructor
RayTracer::RayTracer() :
 	initialized(false),
    raysCacheValid(false),
	isProcessing(false)
{
}

RayTracer::~RayTracer()
{
    // Clean up any resources
}

void RayTracer::initialize(Chamber* parentChamber)
{
    // Update ray cache and frequency responses
    chamber = parentChamber;
    initialized = true;

    DebugLogger::logWithCategory("TRACER", "TRACER initialization completed");
}

// Ray tracing methods
Intersection RayTracer::traceRay(const Ray& ray) const
{
    DebugLogger::logWithCategory("RAY", "Tracing ray");
    const std::vector<std::unique_ptr<Zone>>& zones = chamber->getZones();

    Intersection result;
    result.hit = false;
    result.distance = std::numeric_limits<float>::max();

    // Check intersection with walls
    // Left wall (x = 0)
    if (ray.direction.x < 0)
    {
        float t = -ray.origin.x / ray.direction.x;
        if (t > 0 && t < result.distance)
        {
            float y = ray.origin.y + t * ray.direction.y;
            if (y >= 0 && y <= 1)
            {
                result.hit = true;
                result.distance = t;
                result.point = juce::Point<float>(0, y);
                result.normal = juce::Point<float>(1.0f, 0.0f); // Normal points right (away from left wall)
                result.isWall = true;
                result.wallIndex = 0; // Left wall
                result.zoneId = -1;
            }
        }
    }

    // Right wall (x = 1)
    if (ray.direction.x > 0)
    {
        float t = (1 - ray.origin.x) / ray.direction.x;
        if (t > 0 && t < result.distance)
        {
            float y = ray.origin.y + t * ray.direction.y;
            if (y >= 0 && y <= 1)
            {
                result.hit = true;
                result.distance = t;
                result.point = juce::Point<float>(1, y);
                result.normal = juce::Point<float>(-1.0f, 0.0f); // Normal points left (away from right wall)
                result.isWall = true;
                result.wallIndex = 1; // Right wall
                result.zoneId = -1;
            }
        }
    }

    // Top wall (y = 0)
    if (ray.direction.y < 0)
    {
        float t = -ray.origin.y / ray.direction.y;
        if (t > 0 && t < result.distance)
        {
            float x = ray.origin.x + t * ray.direction.x;
            if (x >= 0 && x <= 1)
            {
                result.hit = true;
                result.distance = t;
                result.point = juce::Point<float>(x, 0);
                result.normal = juce::Point<float>(0.0f, 1.0f); // Normal points down (away from top wall)
                result.isWall = true;
                result.wallIndex = 2; // Top wall
                result.zoneId = -1;
            }
        }
    }

    // Bottom wall (y = 1)
    if (ray.direction.y > 0)
    {
        float t = (1 - ray.origin.y) / ray.direction.y;
        if (t > 0 && t < result.distance)
        {
            float x = ray.origin.x + t * ray.direction.x;
            if (x >= 0 && x <= 1)
            {
                result.hit = true;
                result.distance = t;
                result.point = juce::Point<float>(x, 1);
                result.normal = juce::Point<float>(0.0f, -1.0f); // Normal points up (away from bottom wall)
                result.isWall = true;
                result.wallIndex = 3; // Bottom wall
                result.zoneId = -1;
            }
        }
    }

    // Check intersection with zone boundaries
    for (int i = 0; i < zones.size(); ++i)
    {
        const auto& zone = zones[i];

        // Left boundary (x = zone->x)
        if (ray.direction.x != 0)
        {
            float t = (zone->x - ray.origin.x) / ray.direction.x;
            if (t > 0 && t < result.distance)
            {
                float y = ray.origin.y + t * ray.direction.y;
                if (y >= zone->y && y <= zone->y + zone->height)
                {
                    result.hit = true;
                    result.distance = t;
                    result.point = juce::Point<float>(zone->x, y);
                    result.normal = juce::Point<float>(ray.direction.x > 0 ? -1.0f : 1.0f, 0.0f); // Normal points away from zone boundary
                    result.isWall = false;
                    result.zoneId = i;
                }
            }
        }

        // Right boundary (x = zone->x + zone->width)
        if (ray.direction.x != 0)
        {
            float t = (zone->x + zone->width - ray.origin.x) / ray.direction.x;
            if (t > 0 && t < result.distance)
            {
                float y = ray.origin.y + t * ray.direction.y;
                if (y >= zone->y && y <= zone->y + zone->height)
                {
                    result.hit = true;
                    result.distance = t;
                    result.point = juce::Point<float>(zone->x + zone->width, y);
                    result.normal = juce::Point<float>(ray.direction.x > 0 ? -1.0f : 1.0f, 0.0f); // Normal points away from zone boundary
                    result.isWall = false;
                    result.zoneId = i;
                }
            }
        }

        // Top boundary (y = zone->y)
        if (ray.direction.y != 0)
        {
            float t = (zone->y - ray.origin.y) / ray.direction.y;
            if (t > 0 && t < result.distance)
            {
                float x = ray.origin.x + t * ray.direction.x;
                if (x >= zone->x && x <= zone->x + zone->width)
                {
                    result.hit = true;
                    result.distance = t;
                    result.point = juce::Point<float>(x, zone->y);
                    result.normal = juce::Point<float>(0.0f, ray.direction.y > 0 ? -1.0f : 1.0f); // Normal points away from zone boundary
                    result.isWall = false;
                    result.zoneId = i;
                }
            }
        }

        // Bottom boundary (y = zone->y + zone->height)
        if (ray.direction.y != 0)
        {
            float t = (zone->y + zone->height - ray.origin.y) / ray.direction.y;
            if (t > 0 && t < result.distance)
            {
                float x = ray.origin.x + t * ray.direction.x;
                if (x >= zone->x && x <= zone->x + zone->width)
                {
                    result.hit = true;
                    result.distance = t;
                    result.point = juce::Point<float>(x, zone->y + zone->height);
                    result.normal = juce::Point<float>(0.0f, ray.direction.y > 0 ? -1.0f : 1.0f); // Normal points away from zone boundary
                    result.isWall = false;
                    result.zoneId = i;
                }
            }
        }
    }

    DebugLogger::logWithCategory("RAY", "Ray tracing completed");

    return result;
}

float RayTracer::calculateRayContribution(const Ray& ray, const juce::Point<float>& micPosition) const
{
    DebugLogger::logWithCategory("RAY", "Calculating ray contribution");

    // Calculate vector from ray position to microphone
    juce::Point<float> rayToMic(micPosition.x - ray.origin.x, micPosition.y - ray.origin.y);
    float distanceToMic = rayToMic.getDistanceFrom(juce::Point<float>(0.0f, 0.0f));

    // Skip if too far
    if (distanceToMic > 1.0f)
        return 0.0f;

    // Normalize
    if (distanceToMic > 0.0f) {
        rayToMic.x /= distanceToMic;
        rayToMic.y /= distanceToMic;
    }

    // Calculate dot product to determine if ray is pointing toward microphone
    float dotProduct = ray.direction.x * rayToMic.x + ray.direction.y * rayToMic.y;

    // Skip rays pointing away from microphone
    if (dotProduct < 0.0f)
        return 0.0f;

    // Calculate angle between ray direction and direction to microphone
    float angle = std::acos(dotProduct);

    // Calculate angle factor (higher for rays pointing directly at microphone)
    // Use a more focused beam pattern with cosine raised to power 4
    float angleFactor = std::pow(std::cos(angle), 4.0f);

    // Calculate distance attenuation (inverse square law)
    float distanceAttenuation = 1.0f / (1.0f + distanceToMic * distanceToMic * 10.0f);

    // Calculate contribution based on ray intensity, angle factor, and distance attenuation
    float contribution = ray.intensity * angleFactor * distanceAttenuation;

    // Apply bounce attenuation (rays with more bounces contribute less)
    contribution *= std::pow(0.8f, ray.bounceCount);

    DebugLogger::logWithCategory("RAY", "Ray contribution calculated");

    return contribution;
}

std::vector<Ray> RayTracer::generateReflectionRays(const Ray& ray, const Intersection& intersection) const
{
    DebugLogger::logWithCategory("RAY", "Generating reflection rays");

    std::vector<Ray> reflectionRays;

    // Calculate reflection direction
    juce::Point<float> incidentDir = ray.direction;
    juce::Point<float> normal = intersection.normal;

    // Calculate reflection direction using the formula R = I - 2(I·N)N
    float dot = incidentDir.x * normal.x + incidentDir.y * normal.y;
    juce::Point<float> reflectionDir = {
        incidentDir.x - 2.0f * dot * normal.x,
        incidentDir.y - 2.0f * dot * normal.y
    };

    // Create reflection ray
    Ray reflectionRay(intersection.point, reflectionDir);

    // Reduce intensity based on reflection properties
    reflectionRay.intensity = ray.intensity * 0.7f; // Reduce intensity with each reflection

    // Increase bounce count
    reflectionRay.bounceCount = ray.bounceCount + 1;

    DebugLogger::logWithCategory("RAY", "Copying Frequency Bands");
    // Copy frequency bands
    reflectionRay.frequencyBands = ray.frequencyBands;
    DebugLogger::logWithCategory("RAY", "Copied Frequency Bands");

    // Update frequency bands based on the intersection
    updateRayFrequencies(reflectionRay, intersection);

    reflectionRays.push_back(reflectionRay);

    // Generate additional scattered rays for more realistic sound propagation
    // Use if constexpr to avoid the warning
    if constexpr (RAYS_PER_REFLECTION > 1)
    {
        for (int i = 1; i < RAYS_PER_REFLECTION; ++i)
        {
            // Add some randomness to the reflection direction
            float angle = (static_cast<float>(i) / RAYS_PER_REFLECTION) * juce::MathConstants<float>::pi * 0.5f;
            float scatterX = reflectionDir.x * std::cos(angle) - reflectionDir.y * std::sin(angle);
            float scatterY = reflectionDir.x * std::sin(angle) + reflectionDir.y * std::cos(angle);

            juce::Point<float> scatterDir = {scatterX, scatterY};

            // Normalize the direction
            float length = std::sqrt(scatterDir.x * scatterDir.x + scatterDir.y * scatterDir.y);
            if (length > 0.0f)
            {
                scatterDir.x /= length;
                scatterDir.y /= length;
            }

            // Create scattered ray
            Ray scatteredRay(intersection.point, scatterDir);

            // Reduce intensity for scattered rays
            scatteredRay.intensity = reflectionRay.intensity * 0.5f;

            // Increase bounce count
            scatteredRay.bounceCount = ray.bounceCount + 1;

            // Copy frequency bands
            scatteredRay.frequencyBands = reflectionRay.frequencyBands;

            reflectionRays.push_back(scatteredRay);
        }
    }

    DebugLogger::logWithCategory("RAY", "Reflection rays generated");

    return reflectionRays;
}

void RayTracer::updateRayFrequencies(Ray& ray, const Intersection& intersection) const
{
    DebugLogger::logWithCategory("RAY", "Updating ray frequencies");
    const std::vector<std::unique_ptr<Zone>>& zones = chamber->getZones();

    if (!intersection.hit)
        return;

    if (intersection.isWall)
    {
        // Wall reflections
        // Walls absorb high frequencies more than low frequencies
        for (int i = 0; i < ray.frequencyBands.bands.size(); ++i)
        {
            float freq = 100.0f * std::pow(2.0f, i); // Approximate frequency for this band
            float absorptionFactor = 0.1f + 0.05f * std::log10(freq / 100.0f); // Higher frequencies absorb more
            ray.frequencyBands.bands[i].value *= (1.0f - absorptionFactor);
        }
    }
    else if (intersection.zoneId >= 0 && intersection.zoneId < zones.size())
    {
        // Zone boundary crossings
        // Only proceed if we have a valid zone
        if (zones[intersection.zoneId])
        {
            float zoneDensity = zones[intersection.zoneId]->density;

            // Calculate impedance ratio
            float z1 = chamber->getDefaultMediumDensity(); // Use the default medium density from the class member
            float z2 = zoneDensity; // Zone impedance

            // Calculate transmission coefficient for each frequency band
            for (int i = 0; i < ray.frequencyBands.bands.size(); ++i)
            {
                // Frequency-dependent transmission coefficient
                // Higher frequencies are more affected by density changes
                float freq = 100.0f * std::pow(2.0f, i); // Approximate frequency for this band
                float freqFactor = 0.5f + 0.5f * std::log10(freq / 100.0f) / 3.0f;
                float densityDiff = std::abs(z2 - z1) * freqFactor;

                // Transmission coefficient (simplified model)
                float T = 1.0f - densityDiff / (z1 + z2);
                T = juce::jlimit(0.1f, 1.0f, T); // Limit to reasonable range

                ray.frequencyBands.bands[i].value *= T;
            }
        }
    }

    // Reduce intensity based on distance traveled
    float distanceFactor = 1.0f / (1.0f + intersection.distance * 0.1f);
    ray.intensity *= distanceFactor;

    // Apply additional attenuation for each bounce
    ray.intensity *= 0.8f;

    DebugLogger::logWithCategory("RAY", "Ray frequencies updated");
}

void  RayTracer::updateRayCache()
{
    if (isProcessing)
    {
        DebugLogger::logWithCategory("TRACER", "Skipping ray cache update because it's already being processed");
        return;
    }
    DebugLogger::logWithCategory("TRACER", "Updating ray cache");
    isProcessing = true;

    // Clear the existing cache
    cachedRays.clear();
    std::array<juce::Point<float>, 3> micPositions = chamber->getMicrophonePositions();
    float speakerX = chamber->getSpeakerX();
    float speakerY = chamber->getSpeakerY();

    // Create primary ray from speaker to each microphone
    for (int micIdx = 0; micIdx < 3; ++micIdx)
    {
        const auto [x, y] = micPositions[micIdx];

        // Calculate direction from speaker to microphone
        juce::Point<float> direction(x - speakerX, y - speakerY);

        // Normalize direction
        float length = direction.getDistanceFromOrigin();
        if (length > 0.0f)
        {
            direction /= length;
        }

        // Create primary ray
        Ray primaryRay(juce::Point<float>(speakerX, speakerY), direction);
        primaryRay.intensity = 1.0f; // Full intensity for direct ray
        primaryRay.distance = length;

        // Add primary ray to cache
        cachedRays.push_back(primaryRay);

        // Trace primary ray and generate reflections
        std::vector<Ray> raysToProcess = {primaryRay};

        // Limit the number of reflections to prevent infinite loops
        const int MAX_REFLECTIONS = 100;
        int reflectionCount = 0;

        while (!raysToProcess.empty() && reflectionCount < MAX_REFLECTIONS)
        {
            Ray currentRay = raysToProcess.back();
            raysToProcess.pop_back();

            // Trace ray to find intersection
            Intersection intersection = traceRay(currentRay);

            if (intersection.hit)
            {
                // Generate reflection rays
                std::vector<Ray> reflections = generateReflectionRays(currentRay, intersection);

                // Add reflections to rays to process
                for (auto& reflection : reflections)
                {
                    // Only add if intensity is significant
                    if (reflection.intensity > 0.01f)
                    {
                        raysToProcess.push_back(reflection);
                        cachedRays.push_back(reflection);
                    }
                }

                reflectionCount++;
            }
        }
    }

    raysCacheValid = true;
    DebugLogger::logWithCategory("TRACER", "Ray cache updated");

    calculateMicrophoneFrequencyResponses();
}

void RayTracer::calculateMicrophoneFrequencyResponses()
{
    DebugLogger::logWithCategory("TRACER", "Updating microphone frequency responses");

    std::array<juce::Point<float>, 3> micPositions = chamber->getMicrophonePositions();
    float speakerX = chamber->getSpeakerX();
    float speakerY = chamber->getSpeakerY();
    DebugLogger::logWithCategory("TRACER", "Init microphone frequency responses");

    // Pre-calculate all ray contributions to each microphone
    for (int mic = 0; mic < 3; ++mic) {

        DebugLogger::logWithCategory("TRACER", "Processing microphone " + std::to_string(mic) + "");
        juce::Point<float> micPosition = micPositions[mic];

        // Reset frequency response for this microphone
        micFrequencyResponses[mic].reset(0.0f);

        // Direct ray from speaker to microphone
        juce::Point<float> speakerPosition(speakerX, speakerY);
        Ray directRay(speakerPosition, micPosition - speakerPosition);
        directRay.distance = directRay.direction.getDistanceFromOrigin();

        // Normalize direction
        if (directRay.distance > 0.0f) {
            directRay.direction /= directRay.distance;
        }

        // Check if there's a direct path
        Intersection directIntersection = traceRay(directRay);

        // If the direct ray doesn't hit anything before reaching the microphone
        // or if it hits exactly at the microphone position
        if (!directIntersection.hit ||
            std::abs(directIntersection.distance - directRay.distance) < 0.001f) {
            // Direct contribution with distance attenuation
            float attenuation = 1.0f / (1.0f + directRay.distance * 5.0f);

            // Apply direct contribution to all frequency bands
            micFrequencyResponses[mic] += attenuation;
        }

        // Add contributions from all cached rays to specific frequency bands
        for (const auto& ray : cachedRays) {
            if (ray.intensity > 0.01f) { // Skip rays with negligible intensity
                // Calculate contribution for this ray
                float baseContribution = calculateRayContribution(ray, micPosition);

                if (baseContribution > 0.0f) {
                    micFrequencyResponses[mic]  += ray.frequencyBands * baseContribution;
                }
            }
        }

        // Normalize frequency responses to avoid excessive gain
        micFrequencyResponses[mic].downwardNormalize();
        micFrequencyResponses[mic].bands[0].value = 0;
        micFrequencyResponses[mic].bands[1].value = 0;
        micFrequencyResponses[mic].bands[2].value = 0;
        micFrequencyResponses[mic].calculateBiquadCoefficients(chamber->getSampleRate());
    }

    isProcessing = false;
    DebugLogger::logWithCategory("TRACER", "Microphone frequency responses updated");
   // DebugLogger::logWithCategory("TRACER", "Frequency response coefficients calculated: " + micFrequencyResponses[1].toString());
}
//processBlock called (iteration 1)