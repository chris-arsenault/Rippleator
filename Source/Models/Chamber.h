#pragma once

#include <JuceHeader.h>
#include <vector>
#include <array>
#include <atomic>
#include <memory>
#include "Zone.h"
#include "../Utils/PhysicsHelpers.h"

/**
 * Chamber class that simulates a 2D rectangular chamber filled with multiple fluid/gas zones.
 * This class implements the physical modeling of sound wave propagation through
 * different mediums, with configurable speaker and microphone positions.
 */

// Define MicPosition struct at global scope
struct MicPosition {
    float x, y;
};

// Define GridCell struct to store cell properties
struct GridCell {
    float pressure;          // Current pressure
    float velocityX;         // X component of velocity
    float velocityY;         // Y component of velocity
    float density;           // Local medium density
    float impedance;         // Acoustic impedance
    float soundSpeed;        // Speed of sound in this cell
    float damping;           // Damping factor for this cell
    bool isWall;             // Whether this cell is a wall boundary
    bool isZoneBoundary;     // Whether this cell is at a zone boundary
    int zoneId;              // ID of the zone this cell belongs to (-1 for default medium)
    
    // Frequency domain data
    std::vector<float> frequencyBands;    // Amplitude of different frequency bands
    std::vector<float> frequencyPhases;   // Phase of different frequency bands
};

class Chamber
{
public:
    static constexpr int GRID_WIDTH = 100;
    static constexpr int GRID_HEIGHT = 100;
    static constexpr int FFT_SIZE = 1024;    // Size of FFT for frequency analysis
    static constexpr int NUM_FREQUENCY_BANDS = 8;  // Number of frequency bands to analyze
    
    Chamber();
    ~Chamber();
    
    void initialize(double sampleRate, float speakerX, float speakerY);
    void processBlock(const float* input, int numSamples);
    void setMicrophonePosition(int index, float x, float y);
    float getMicrophoneOutput(int index) const;
    
    // Grid access
    const std::vector<float>& getGrid() const;
    
    // Parameter setters
    void setMediumDensity(float density);
    void setWallReflectivity(float reflectivity);
    void setWallDamping(float damping);
    
    // Zone management
    int addZone();
    void removeZone(int zoneId);
    void setZoneDensity(int zoneId, float density);
    void setZoneBounds(int zoneId, float x1, float y1, float x2, float y2);
    const std::vector<std::unique_ptr<Zone>>& getZones() const { return zones; }
    
    // Microphone management
    const std::array<MicPosition, 3>& getMicrophonePositions() const { return micPositions; }
    
    // Speaker position
    float getSpeakerX() const { return speakerX; }
    float getSpeakerY() const { return speakerY; }
    void setSpeakerPosition(float x, float y);
    
    bool isInitialized() const;

private:
    void updateGrid(float input);
    float sampleGrid(float x, float y) const;
    void updateCellProperties();
    void performFrequencyAnalysis(float input);
    void applyFrequencyEffects();
    bool isCellAtZoneBoundary(int x, int y) const;
    void handleWallReflection(int x, int y);
    void handleZoneRefraction(int x, int y);
    
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
    
    // Grid data
    std::vector<float> grid;                // Visualization grid (pressure values)
    std::vector<GridCell> cells;            // Grid cells with all properties
    
    // Audio processing
    std::vector<float> fftInputBuffer;      // Buffer for FFT input
    std::vector<float> fftTimeBuffer;       // Time domain buffer
    std::vector<float> fftFrequencyBuffer;  // Frequency domain buffer
    
    // Zones
    std::vector<std::unique_ptr<Zone>> zones;
    int nextZoneId;
    
    // Microphone positions (up to 3)
    std::array<MicPosition, 3> micPositions;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Chamber)
};
