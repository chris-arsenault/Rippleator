#include "Chamber.h"

Chamber::Chamber()
    : initialized(false),
      sampleRate(44100.0),
      speakerX(0.5f),
      speakerY(0.5f),
      mediumDensity(1.0f),
      wallReflectivity(0.5f),
      wallDamping(0.1f),
      nextZoneId(0),
      micPositions{{{0.25f, 0.25f}, {0.5f, 0.5f}, {0.75f, 0.75f}}},
      fftSize(1024),
      fftData(fftSize * 2, 0.0f),
      fftOutput(fftSize * 2, 0.0f),
      windowFunction(fftSize),
      fftBufferPos(0)
{
    // Initialize visualization grid
    grid.resize(GRID_WIDTH * GRID_HEIGHT, 0.0f);
    
    // Initialize grid cells
    cells.resize(GRID_WIDTH * GRID_HEIGHT);
    
    // Initialize FFT objects
    for (int i = 0; i < fftSize; ++i)
    {
        // Hann window
        windowFunction[i] = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * i / (fftSize - 1)));
    }
    
    // Initialize all cells with default properties
    updateCellProperties();
}

Chamber::~Chamber()
{
}

void Chamber::initialize(double newSampleRate, float newSpeakerX, float newSpeakerY)
{
    this->sampleRate = newSampleRate;
    this->speakerX = newSpeakerX;
    this->speakerY = newSpeakerY;
    
    // Initialize the grid cells
    cells.resize(GRID_WIDTH * GRID_HEIGHT);
    grid.resize(GRID_WIDTH * GRID_HEIGHT, 0.0f);
    
    // Initialize FFT objects
    fftSize = 1024;
    fftData.resize(fftSize * 2, 0.0f);
    fftOutput.resize(fftSize * 2, 0.0f);
    
    // Initialize the window function
    windowFunction.resize(fftSize);
    for (int i = 0; i < fftSize; ++i)
    {
        // Hann window
        windowFunction[i] = 0.5f * (1.0f - std::cos(2.0f * juce::MathConstants<float>::pi * i / (fftSize - 1)));
    }
    
    // Default microphone positions
    micPositions[0] = {0.25f, 0.25f};
    micPositions[1] = {0.5f, 0.5f};
    micPositions[2] = {0.75f, 0.75f};
    
    // Initialize all cells with default properties
    updateCellProperties();
    
    initialized = true;
}

void Chamber::setMediumDensity(float density)
{
    mediumDensity = density;
    updateCellProperties();
}

void Chamber::setWallReflectivity(float reflectivity)
{
    wallReflectivity = reflectivity;
    updateCellProperties();
}

void Chamber::setWallDamping(float damping)
{
    wallDamping = damping;
    updateCellProperties();
}

const std::vector<float>& Chamber::getGrid() const
{
    return grid;
}

bool Chamber::isInitialized() const
{
    return initialized;
}

int Chamber::addZone()
{
    auto zone = std::make_unique<Zone>();
    int zoneId = nextZoneId++;
    zones.push_back(std::move(zone));
    updateCellProperties();
    return zoneId;
}

void Chamber::removeZone(int zoneId)
{
    if (zoneId >= 0 && zoneId < zones.size())
    {
        zones.erase(zones.begin() + zoneId);
        updateCellProperties();
    }
}

void Chamber::setZoneDensity(int zoneId, float density)
{
    if (zoneId >= 0 && zoneId < zones.size())
    {
        zones[zoneId]->density = density;
        updateCellProperties();
    }
}

void Chamber::setZoneBounds(int zoneId, float x1, float y1, float x2, float y2)
{
    if (zoneId >= 0 && zoneId < zones.size())
    {
        auto& zone = zones[zoneId];
        zone->x1 = x1;
        zone->y1 = y1;
        zone->x2 = x2;
        zone->y2 = y2;
        updateCellProperties();
    }
}

void Chamber::processBlock(const float* input, int numSamples)
{
    if (!initialized)
        return;

    // Process each sample
    for (int i = 0; i < numSamples; ++i)
    {
        // Apply a simple pre-filter to the input to reduce high-frequency content
        // that could cause aliasing in the simulation
        static float lastInput = 0.0f;
        float filteredInput = 0.7f * input[i] + 0.3f * lastInput;
        lastInput = input[i];
        
        // Perform frequency analysis on the filtered input
        performFrequencyAnalysis(filteredInput);
        
        // Update the grid with the current filtered input sample
        updateGrid(filteredInput);
    }
}

void Chamber::performFrequencyAnalysis(float input)
{
    // Add the input sample to the FFT buffer
    fftData[fftBufferPos] = input;
    fftBufferPos = (fftBufferPos + 1) % fftSize;
    
    // When the buffer is full, perform FFT analysis
    if (fftBufferPos == 0)
    {
        // Prepare the FFT input (real part in even indices, imaginary part in odd indices)
        for (int i = 0; i < fftSize; ++i)
        {
            // Apply a Hanning window to reduce spectral leakage
            fftData[i * 2] = fftData[i] * windowFunction[i];
            fftData[i * 2 + 1] = 0.0f;  // Imaginary part is zero for time domain
        }
        
        // Perform forward FFT
        // forwardFFT.performRealOnlyForwardTransform(fftData.data(), true);
        
        // Copy FFT data to frequency buffer for later use
        fftOutput = fftData;
        
        // Extract frequency band data (simplistic approach - divide spectrum into bands)
        int bandsPerBin = fftSize / (2 * NUM_FREQUENCY_BANDS);
        
        // Calculate speaker position index
        int speakerIdxX = static_cast<int>(speakerX * GRID_WIDTH);
        int speakerIdxY = static_cast<int>(speakerY * GRID_HEIGHT);
        int speakerIdx = speakerIdxY * GRID_WIDTH + speakerIdxX;
        
        // Update frequency data for the speaker cell
        for (int band = 0; band < NUM_FREQUENCY_BANDS; ++band)
        {
            float bandAmplitude = 0.0f;
            float bandPhase = 0.0f;
            
            // Average amplitude and phase across the band
            for (int bin = band * bandsPerBin; bin < (band + 1) * bandsPerBin; ++bin)
            {
                int realIdx = bin * 2;
                int imagIdx = bin * 2 + 1;
                
                float real = fftOutput[realIdx];
                float imag = fftOutput[imagIdx];
                
                float amplitude = std::sqrt(real * real + imag * imag);
                float phase = std::atan2(imag, real);
                
                bandAmplitude += amplitude;
                bandPhase += phase;
            }
            
            bandAmplitude /= bandsPerBin;
            bandPhase /= bandsPerBin;
            
            // Store band data in speaker cell
            cells[speakerIdx].frequencyBands[band] = bandAmplitude;
            cells[speakerIdx].frequencyPhases[band] = bandPhase;
        }
    }
}

void Chamber::updateGrid(float input)
{
    // Wave equation simulation with improved physics
    const float dt = 1.0f / static_cast<float>(sampleRate);
    const float dx = 1.0f / static_cast<float>(GRID_WIDTH);
    
    // Apply input at speaker position - recalculate the index each time to ensure it's current
    int speakerIdxX = static_cast<int>(speakerX * GRID_WIDTH);
    int speakerIdxY = static_cast<int>(speakerY * GRID_HEIGHT);
    int speakerIdx = speakerIdxY * GRID_WIDTH + speakerIdxX;
    
    // Scale the input to prevent clipping and ensure it's properly connected to the audio input
    // Use a gentler scaling and apply a soft-clipping to prevent harsh digital artifacts
    float scaledInput = std::tanh(input * 10.0f) * 15.0f;
    
    // Add input at the speaker position
    cells[speakerIdx].pressure += scaledInput;
    
    // Apply frequency-dependent effects
    applyFrequencyEffects();
    
    // First pass: Update velocity field based on pressure gradient
    for (int y = 1; y < GRID_HEIGHT - 1; ++y)
    {
        for (int x = 1; x < GRID_WIDTH - 1; ++x)
        {
            const int idx = y * GRID_WIDTH + x;
            auto& cell = cells[idx];
            
            if (cell.isWall)
                continue;
            
            // Get neighboring cells
            const auto& cellLeft = cells[idx - 1];
            const auto& cellRight = cells[idx + 1];
            const auto& cellUp = cells[idx - GRID_WIDTH];
            const auto& cellDown = cells[idx + GRID_WIDTH];
            
            // Calculate pressure gradients
            float pressureGradientX = (cellRight.pressure - cellLeft.pressure) / (2.0f * dx);
            float pressureGradientY = (cellDown.pressure - cellUp.pressure) / (2.0f * dx);
            
            // Update velocity based on pressure gradient
            cell.velocityX = cell.velocityX * (1.0f - cell.damping * 0.1f) - 
                           (dt / (cell.density * dx)) * pressureGradientX;
            
            cell.velocityY = cell.velocityY * (1.0f - cell.damping * 0.1f) - 
                           (dt / (cell.density * dx)) * pressureGradientY;
            
            // Handle wall reflections
            if (x == 1 || x == GRID_WIDTH - 2 || y == 1 || y == GRID_HEIGHT - 2)
            {
                handleWallReflection(x, y);
            }
            
            // Handle zone boundaries (refraction)
            if (cell.isZoneBoundary)
            {
                handleZoneRefraction(x, y);
            }
        }
    }
    
    // Second pass: Update pressure based on velocity divergence
    for (int y = 1; y < GRID_HEIGHT - 1; ++y)
    {
        for (int x = 1; x < GRID_WIDTH - 1; ++x)
        {
            const int idx = y * GRID_WIDTH + x;
            auto& cell = cells[idx];
            
            if (cell.isWall)
                continue;
            
            // Get neighboring cells
            const auto& cellLeft = cells[idx - 1];
            const auto& cellRight = cells[idx + 1];
            const auto& cellUp = cells[idx - GRID_WIDTH];
            const auto& cellDown = cells[idx + GRID_WIDTH];
            
            // Calculate velocity divergence
            float velocityDivergence = (cellRight.velocityX - cellLeft.velocityX) / (2.0f * dx) +
                                     (cellDown.velocityY - cellUp.velocityY) / (2.0f * dx);
            
            // Update pressure based on velocity divergence and local sound speed
            cell.pressure = cell.pressure * (0.9999f - cell.damping * 0.01f) - 
                         cell.density * cell.soundSpeed * cell.soundSpeed * dt * velocityDivergence;
            
            // Apply additional damping at boundaries
            if (x == 1 || x == GRID_WIDTH - 2 || y == 1 || y == GRID_HEIGHT - 2)
            {
                cell.pressure *= (1.0f - wallDamping * 0.1f);
            }
        }
    }
    
    // Special handling for the speaker position to ensure sound propagates properly
    // Re-add the input at the speaker position after the update
    if (speakerIdxX > 0 && speakerIdxX < GRID_WIDTH - 1 && 
        speakerIdxY > 0 && speakerIdxY < GRID_HEIGHT - 1) {
        cells[speakerIdx].pressure += scaledInput * 0.5f;
    }
    
    // Copy pressure field to grid for visualization
    for (int i = 0; i < GRID_WIDTH * GRID_HEIGHT; ++i) {
        grid[i] = cells[i].pressure;
    }
}

void Chamber::handleWallReflection(int x, int y)
{
    const int idx = y * GRID_WIDTH + x;
    auto& cell = cells[idx];
    
    // Apply wall reflection based on wall reflectivity
    if (x == 1) // Left wall
    {
        cell.velocityX = -cell.velocityX * wallReflectivity;
    }
    else if (x == GRID_WIDTH - 2) // Right wall
    {
        cell.velocityX = -cell.velocityX * wallReflectivity;
    }
    
    if (y == 1) // Top wall
    {
        cell.velocityY = -cell.velocityY * wallReflectivity;
    }
    else if (y == GRID_HEIGHT - 2) // Bottom wall
    {
        cell.velocityY = -cell.velocityY * wallReflectivity;
    }
    
    // Apply additional damping at walls
    cell.pressure *= (1.0f - wallDamping * 0.1f);
}

void Chamber::handleZoneRefraction(int x, int y)
{
    const int idx = y * GRID_WIDTH + x;
    auto& cell = cells[idx];
    
    // Check neighboring cells to find zone boundaries
    int neighbors[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    
    for (auto& neighbor : neighbors)
    {
        int nx = x + neighbor[0];
        int ny = y + neighbor[1];
        
        if (nx >= 0 && nx < GRID_WIDTH && ny >= 0 && ny < GRID_HEIGHT)
        {
            int nIdx = ny * GRID_WIDTH + nx;
            auto& neighborCell = cells[nIdx];
            
            if (neighborCell.zoneId != cell.zoneId)
            {
                // Calculate impedance ratio
                float z1 = cell.impedance;
                float z2 = neighborCell.impedance;
                
                // Calculate angle of incidence (simplified to normal incidence)
                float angle = 0.0f;
                
                // Apply reflection and transmission at boundary
                float reflectionCoeff = PhysicsHelpers::calculateReflectionCoefficient(z1, z2, angle);
                float transmissionCoeff = PhysicsHelpers::calculateTransmissionCoefficient(z1, z2, angle);
                
                // Apply reflection to velocity components
                if (neighbor[0] != 0) // X boundary
                {
                    float reflectedVelocity = cell.velocityX * reflectionCoeff;
                    float transmittedVelocity = cell.velocityX * transmissionCoeff;
                    
                    cell.velocityX = reflectedVelocity;
                    neighborCell.velocityX = transmittedVelocity;
                }
                else if (neighbor[1] != 0) // Y boundary
                {
                    float reflectedVelocity = cell.velocityY * reflectionCoeff;
                    float transmittedVelocity = cell.velocityY * transmissionCoeff;
                    
                    cell.velocityY = reflectedVelocity;
                    neighborCell.velocityY = transmittedVelocity;
                }
                
                // Apply frequency-dependent attenuation
                for (int band = 0; band < NUM_FREQUENCY_BANDS; ++band)
                {
                    float normalizedFreq = static_cast<float>(band) / NUM_FREQUENCY_BANDS;
                    float attenuation = PhysicsHelpers::calculateFrequencyAttenuation(z1, z2, normalizedFreq);
                    
                    neighborCell.frequencyBands[band] = cell.frequencyBands[band] * attenuation;
                    neighborCell.frequencyPhases[band] = cell.frequencyPhases[band];
                }
            }
        }
    }
}

void Chamber::applyFrequencyEffects()
{
    // Apply frequency-dependent effects to each cell
    for (int y = 1; y < GRID_HEIGHT - 1; ++y)
    {
        for (int x = 1; x < GRID_WIDTH - 1; ++x)
        {
            int idx = y * GRID_WIDTH + x;
            auto& cell = cells[idx];
            
            if (cell.isWall)
                continue;
            
            // Apply frequency-dependent attenuation
            for (int band = 0; band < NUM_FREQUENCY_BANDS; ++band)
            {
                // Higher frequencies attenuate more in denser media
                float frequencyFactor = (band + 1.0f) / NUM_FREQUENCY_BANDS;
                float attenuationFactor = 1.0f - (cell.density * 0.01f * frequencyFactor);
                
                // Ensure attenuation factor is within reasonable bounds
                attenuationFactor = std::max(0.9f, attenuationFactor);
                
                // Apply attenuation
                cell.frequencyBands[band] *= attenuationFactor;
            }
            
            // Reconstruct pressure from frequency bands
            // This is a simplified approach - in a real implementation, you'd use inverse FFT
            float pressureContribution = 0.0f;
            for (int band = 0; band < NUM_FREQUENCY_BANDS; ++band)
            {
                pressureContribution += cell.frequencyBands[band] * 0.1f;
            }
            
            // Add frequency contribution to pressure (small amount to avoid overwhelming the simulation)
            cell.pressure += pressureContribution * 0.01f;
        }
    }
}

void Chamber::updateCellProperties()
{
    // Set default properties for all cells
    for (int y = 0; y < GRID_HEIGHT; ++y)
    {
        for (int x = 0; x < GRID_WIDTH; ++x)
        {
            int idx = y * GRID_WIDTH + x;
            auto& cell = cells[idx];
            
            // Reset cell properties
            cell.pressure = 0.0f;
            cell.velocityX = 0.0f;
            cell.velocityY = 0.0f;
            cell.isWall = (x == 0 || x == GRID_WIDTH - 1 || y == 0 || y == GRID_HEIGHT - 1);
            cell.isZoneBoundary = false;
            cell.zoneId = -1;
            
            // Default medium properties
            cell.density = mediumDensity;
            cell.soundSpeed = PhysicsHelpers::calculateSoundSpeed(mediumDensity);
            cell.impedance = PhysicsHelpers::calculateAcousticImpedance(mediumDensity);
            cell.damping = PhysicsHelpers::calculateDamping(mediumDensity) * 0.1f; // Reduce damping
            
            // Initialize frequency domain data
            cell.frequencyBands.resize(NUM_FREQUENCY_BANDS, 0.0f);
            cell.frequencyPhases.resize(NUM_FREQUENCY_BANDS, 0.0f);
        }
    }
    
    // Apply zone properties
    for (size_t zoneIndex = 0; zoneIndex < zones.size(); ++zoneIndex)
    {
        const auto& zone = zones[zoneIndex];
        
        // Convert normalized coordinates to grid indices
        int zoneX1 = static_cast<int>(zone->x1 * GRID_WIDTH);
        int zoneY1 = static_cast<int>(zone->y1 * GRID_HEIGHT);
        int zoneX2 = static_cast<int>(zone->x2 * GRID_WIDTH);
        int zoneY2 = static_cast<int>(zone->y2 * GRID_HEIGHT);
        
        // Ensure indices are within grid bounds
        zoneX1 = std::max(1, std::min(zoneX1, GRID_WIDTH - 2));
        zoneY1 = std::max(1, std::min(zoneY1, GRID_HEIGHT - 2));
        zoneX2 = std::max(1, std::min(zoneX2, GRID_WIDTH - 2));
        zoneY2 = std::max(1, std::min(zoneY2, GRID_HEIGHT - 2));
        
        // Ensure x1 <= x2 and y1 <= y2
        if (zoneX1 > zoneX2) std::swap(zoneX1, zoneX2);
        if (zoneY1 > zoneY2) std::swap(zoneY1, zoneY2);
        
        // Set properties for cells within the zone
        for (int cellY = zoneY1; cellY <= zoneY2; ++cellY)
        {
            for (int cellX = zoneX1; cellX <= zoneX2; ++cellX)
            {
                int idx = cellY * GRID_WIDTH + cellX;
                auto& cell = cells[idx];
                
                // Set zone properties
                cell.density = zone->density;
                cell.soundSpeed = PhysicsHelpers::calculateSoundSpeed(zone->density);
                cell.impedance = PhysicsHelpers::calculateAcousticImpedance(zone->density);
                cell.damping = PhysicsHelpers::calculateDamping(zone->density) * 0.1f; // Reduce damping
                cell.zoneId = static_cast<int>(zoneIndex);
                
                // Check if this cell is at a zone boundary
                if (cellX == zoneX1 || cellX == zoneX2 || cellY == zoneY1 || cellY == zoneY2)
                {
                    cell.isZoneBoundary = true;
                }
            }
        }
    }
}

void Chamber::setMicrophonePosition(int index, float x, float y)
{
    if (index >= 0 && index < 3)
    {
        micPositions[index] = {x, y};
    }
}

void Chamber::setSpeakerPosition(float x, float y)
{
    // Store old position
    int oldSpeakerIdxX = static_cast<int>(speakerX * GRID_WIDTH);
    int oldSpeakerIdxY = static_cast<int>(speakerY * GRID_HEIGHT);
    int oldSpeakerIdx = oldSpeakerIdxY * GRID_WIDTH + oldSpeakerIdxX;
    
    // Clamp values to 0-1 range
    speakerX = juce::jlimit(0.0f, 1.0f, x);
    speakerY = juce::jlimit(0.0f, 1.0f, y);
    
    // Calculate new speaker position
    int newSpeakerIdxX = static_cast<int>(speakerX * GRID_WIDTH);
    int newSpeakerIdxY = static_cast<int>(speakerY * GRID_HEIGHT);
    int newSpeakerIdx = newSpeakerIdxY * GRID_WIDTH + newSpeakerIdxX;
    
    // Only clear the old speaker position to stop sound from continuing there
    if (oldSpeakerIdx >= 0 && oldSpeakerIdx < GRID_WIDTH * GRID_HEIGHT && oldSpeakerIdx != newSpeakerIdx) {
        // Clear a small area around the old speaker position
        for (int dy = -3; dy <= 3; dy++) {
            for (int dx = -3; dx <= 3; dx++) {
                int y = oldSpeakerIdxY + dy;
                int x = oldSpeakerIdxX + dx;
                if (x >= 0 && x < GRID_WIDTH && y >= 0 && y < GRID_HEIGHT) {
                    int idx = y * GRID_WIDTH + x;
                    cells[idx].pressure = 0.0f;
                    cells[idx].velocityX = 0.0f;
                    cells[idx].velocityY = 0.0f;
                }
            }
        }
    }
}

float Chamber::getMicrophoneOutput(int index) const
{
    if (index >= 0 && index < 3)
    {
        const auto& pos = micPositions[index];
        
        // Get the raw pressure at the microphone position
        float pressure = sampleGrid(pos.x, pos.y);
        
        // Apply a simple smoothing filter to reduce digital artifacts
        // This simulates the frequency response of a real microphone
        static float lastSamples[3][4] = {{0.0f}};
        
        // Simple 4-point moving average filter
        float filteredOutput = (pressure + lastSamples[index][0] + lastSamples[index][1] + lastSamples[index][2]) * 0.25f;
        
        // Update the filter buffer
        lastSamples[index][2] = lastSamples[index][1];
        lastSamples[index][1] = lastSamples[index][0];
        lastSamples[index][0] = pressure;
        
        // Apply a gentle soft-clipping to prevent harsh digital distortion
        filteredOutput = std::tanh(filteredOutput * 0.8f);
        
        return filteredOutput;
    }
    return 0.0f;
}

float Chamber::sampleGrid(float x, float y) const
{
    // Bilinear interpolation of pressure field
    float gx = x * (GRID_WIDTH - 1);
    float gy = y * (GRID_HEIGHT - 1);
    
    int x0 = static_cast<int>(gx);
    int y0 = static_cast<int>(gy);
    int x1 = x0 + 1;
    int y1 = y0 + 1;
    
    // Clamp to valid grid indices
    x0 = std::max(0, std::min(x0, GRID_WIDTH - 1));
    x1 = std::max(0, std::min(x1, GRID_WIDTH - 1));
    y0 = std::max(0, std::min(y0, GRID_HEIGHT - 1));
    y1 = std::max(0, std::min(y1, GRID_HEIGHT - 1));
    
    float fx = gx - x0;
    float fy = gy - y0;
    
    float v00 = cells[y0 * GRID_WIDTH + x0].pressure;
    float v10 = cells[y0 * GRID_WIDTH + x1].pressure;
    float v01 = cells[y1 * GRID_WIDTH + x0].pressure;
    float v11 = cells[y1 * GRID_WIDTH + x1].pressure;
    
    float v0 = v00 * (1 - fx) + v10 * fx;
    float v1 = v01 * (1 - fx) + v11 * fx;
    
    return v0 * (1 - fy) + v1 * fy;
}

bool Chamber::isCellAtZoneBoundary(int x, int y) const
{
    if (x <= 0 || x >= GRID_WIDTH - 1 || y <= 0 || y >= GRID_HEIGHT - 1)
        return false;
        
    int idx = y * GRID_WIDTH + x;
    int zoneId = cells[idx].zoneId;
    
    // Check neighboring cells
    int neighbors[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    
    for (auto& neighbor : neighbors)
    {
        int nx = x + neighbor[0];
        int ny = y + neighbor[1];
        
        if (nx >= 0 && nx < GRID_WIDTH && ny >= 0 && ny < GRID_HEIGHT)
        {
            int nIdx = ny * GRID_WIDTH + nx;
            if (cells[nIdx].zoneId != zoneId)
                return true;
        }
    }
    
    return false;
}
