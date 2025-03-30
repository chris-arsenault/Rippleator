#pragma once

#include <JuceHeader.h>
#include <cmath>

/**
 * Utility class with helper functions for physical modeling calculations.
 */
namespace PhysicsHelpers
{
    /**
     * Calculate the speed of sound in a medium based on density.
     * This is a simplified model for the plugin.
     * 
     * @param density The medium density parameter (0.1 to 10.0)
     * @return The relative speed of sound in the medium
     */
    inline float calculateSoundSpeed(float density)
    {
        // In a real fluid, speed of sound is inversely proportional to square root of density
        // For our plugin, we'll use a simplified model
        return 0.1f + (0.05f / std::sqrt(density));
    }
    
    /**
     * Calculate damping factor based on medium density.
     * 
     * @param density The medium density parameter (0.1 to 10.0)
     * @return The damping factor for the simulation
     */
    inline float calculateDamping(float density)
    {
        // Higher density = more damping
        return 0.999f - (0.0005f * density);
    }
    
    /**
     * Calculate the acoustic impedance of a medium based on its density.
     * 
     * @param density The medium density parameter (0.1 to 10.0)
     * @return The acoustic impedance
     */
    inline float calculateAcousticImpedance(float density)
    {
        // Acoustic impedance is proportional to density * speed of sound
        float c = calculateSoundSpeed(density);
        return density * c;
    }
    
    /**
     * Calculate the transmission coefficient for a wave crossing a boundary.
     * 
     * @param z1 Acoustic impedance of first medium
     * @param z2 Acoustic impedance of second medium
     * @param angle Angle of incidence (in radians)
     * @return The transmission coefficient
     */
    inline float calculateTransmissionCoefficient(float z1, float z2, float angle = 0.0f)
    {
        // For normal incidence (angle = 0):
        // T = (2 * z2) / (z1 + z2)
        float cosTheta = std::cos(angle);
        return (2.0f * z2 * cosTheta) / (z1 + z2);
    }
    
    /**
     * Calculate the reflection coefficient at a boundary.
     * 
     * @param z1 Acoustic impedance of first medium
     * @param z2 Acoustic impedance of second medium
     * @param angle Angle of incidence (in radians)
     * @return The reflection coefficient (-1.0 to 1.0)
     */
    inline float calculateReflectionCoefficient(float z1, float z2, float angle = 0.0f)
    {
        // For normal incidence (angle = 0):
        // R = (z2 - z1) / (z2 + z1)
        float cosTheta = std::cos(angle);
        return (z2 * cosTheta - z1) / (z2 * cosTheta + z1);
    }
    
    /**
     * Calculate frequency-dependent attenuation for a wave crossing a boundary.
     * Higher frequencies are attenuated more when crossing into denser mediums.
     * 
     * @param sourceZ Acoustic impedance of source medium
     * @param targetZ Acoustic impedance of target medium
     * @param normalizedFreq Normalized frequency (0.0 to 1.0)
     * @return Attenuation factor for the frequency
     */
    inline float calculateFrequencyAttenuation(float sourceZ, float targetZ, float normalizedFreq)
    {
        // Higher frequencies are attenuated more in denser mediums
        float impedanceRatio = targetZ / sourceZ;
        float freqFactor = 1.0f - (normalizedFreq * 0.5f); // Higher frequencies are attenuated more
        
        // If moving to denser medium (impedanceRatio > 1), attenuate high frequencies more
        if (impedanceRatio > 1.0f) {
            return 1.0f / (1.0f + (impedanceRatio - 1.0f) * normalizedFreq);
        }
        // If moving to less dense medium, preserve frequencies better
        else {
            return freqFactor + (1.0f - freqFactor) * impedanceRatio;
        }
    }
    
    /**
     * Convert between normalized coordinates (0.0-1.0) and grid coordinates.
     * 
     * @param normCoord Normalized coordinate (0.0 to 1.0)
     * @param gridSize The size of the grid in that dimension
     * @return The corresponding grid coordinate
     */
    inline int normalizedToGridCoord(float normCoord, int gridSize)
    {
        int gridCoord = static_cast<int>(normCoord * (gridSize - 1));
        return juce::jlimit(0, gridSize - 1, gridCoord);
    }
    
    /**
     * Calculate the distance between two points in normalized coordinates.
     * 
     * @param x1 First point X coordinate
     * @param y1 First point Y coordinate
     * @param x2 Second point X coordinate
     * @param y2 Second point Y coordinate
     * @return The Euclidean distance between the points
     */
    inline float distance(float x1, float y1, float x2, float y2)
    {
        float dx = x2 - x1;
        float dy = y2 - y1;
        return std::sqrt(dx * dx + dy * dy);
    }
}
