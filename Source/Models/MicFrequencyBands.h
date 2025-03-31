#pragma once

#include <array>
#include <cmath>
#include <DebugLogger.h>

// Define M_PI if not already defined
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct Biquad {
    double a0, a1, a2, b0, b1, b2;
    double z1, z2; // State variables for Direct Form I implementation
};

struct FrequencyBand
{
    static constexpr float DEFAULT_Q = 1;
    float minFrequency;
    float maxFrequency;
    float centerFrequency;
    float value;
    double gain;
    Biquad biquad;
    void calculateBiquadCoefficients(double sampleRate) {
        centerFrequency = (minFrequency + maxFrequency) / 2.0;

        // Calculate gain in linear scale
        // double gainLinear = (value > 0.0f) ? value : 0.0;

        double gainDB;
        if (value <= 0.0f) {
            gainDB = -96.0;  // Near silence for zero/negative values
        } else if (value < 0.5f) {
            // Map 0.0-0.5 to -96dB to 0dB logarithmically
            gainDB = -96.0 * (1.0 - value/0.5);
        } else {
            // Map 0.5-1.0 to 0dB to +12dB linearly
            gainDB = 24.0 * (value - 0.5);  // Gives +12dB at value=1.0
        }

        double w0 = 2.0 * M_PI * centerFrequency / sampleRate;
        double cosw0 = cos(w0);
        double sinw0 = sin(w0);
        double Q =  4.32;//centerFrequency / (maxFrequency - minFrequency);
        double alpha = sinw0 / (2.0 * Q);



        // For a proper peak filter:
        gain = pow(10, gainDB / 40.0);
        double A = pow(10, gainDB / 40);

        // Standard biquad peak/notch filter coefficients
        biquad.b0 = 1.0 + alpha * A;
        biquad.b1 = -2.0 * cosw0;
        biquad.b2 = 1.0 - alpha * A;
        biquad.a0 = 1.0 + alpha / A;
        biquad.a1 = -2.0 * cosw0;
        biquad.a2 = 1.0 - alpha / A;

        // Normalize by a0
        double oneOverA0 = 1.0 / biquad.a0;
        biquad.b0 *= oneOverA0;
        biquad.b1 *= oneOverA0;
        biquad.b2 *= oneOverA0;
        biquad.a1 *= oneOverA0;
        biquad.a2 *= oneOverA0;
        biquad.a0 = 1.0;  // Now normalized

    }

    float processSample(float sample) {
        // Apply biquad filter
        double y = biquad.b0 * sample + biquad.b1 * biquad.z1 + biquad.b2 * biquad.z2 - biquad.a1 * biquad.z1 - biquad.a2 * biquad.z2;
        biquad.z2 = biquad.z1;
        biquad.z1 = y;
        return static_cast<float>(y);
    }

};

struct MicFrequencyBands
{
    static constexpr int NUM_FREQUENCY_BANDS = 3;
    static constexpr int MIN_FREQUENCY = 20;
    static constexpr int MAX_FREQUENCY = 12000;
    std::array<FrequencyBand, NUM_FREQUENCY_BANDS> bands;
    MicFrequencyBands()
    {
        // Logarithmic increment per band
        float logMin = std::log10(MIN_FREQUENCY);
        float logMax = std::log10(MAX_FREQUENCY);
        float logStep = (logMax - logMin) / NUM_FREQUENCY_BANDS;

        for (int i = 0; i < NUM_FREQUENCY_BANDS; ++i)
        {
            // Calculate min and max frequency for the band
            float bandMin = std::pow(10, logMin + i * logStep);
            float bandMax = std::pow(10, logMin + (i + 1) * logStep);

            // Assign to the band
            bands[i] = {bandMin, bandMax, 1.0f};
        }
    }
    void reset(const float value)
    {
        for (int i = 0; i < NUM_FREQUENCY_BANDS; ++i)
        {
            bands[i].value = value;
        }
    }
    void calculateBiquadCoefficients(double sampleRate)
    {
        for (int i = 0; i < NUM_FREQUENCY_BANDS; ++i)
        {
            bands[i].calculateBiquadCoefficients(sampleRate);
        }
    }
    FrequencyBand getBandForFrequency(float f)
    {
        for (int i = 0; i < NUM_FREQUENCY_BANDS; ++i)
        {
            if (f >= bands[i].minFrequency && f <= bands[i].maxFrequency)
            {
                return bands[i];
            }
        }
        return bands[NUM_FREQUENCY_BANDS - 1];
    }

    void downwardNormalize()
    {
        float maxResponse = 0.0f;
        for (int band = 0; band < MicFrequencyBands::NUM_FREQUENCY_BANDS; ++band) {
            maxResponse = std::max(maxResponse, bands[band].value);
        }

        if (maxResponse > 1.0f) {
            for (int band = 0; band < MicFrequencyBands::NUM_FREQUENCY_BANDS; ++band) {
                bands[band].value /= maxResponse;
            }
        }
    }

    // Overloaded operator to multiply all band values by a constant
    MicFrequencyBands operator*(float scalar) const
    {
        MicFrequencyBands result = *this;
        for (int i = 0; i < NUM_FREQUENCY_BANDS; ++i)
        {
            result.bands[i].value *= scalar;
        }
        return result;
    }

    // Overloaded operator to add all band values by a constant
    MicFrequencyBands operator+(float scalar) const
    {
        MicFrequencyBands result = *this;
        for (int i = 0; i < NUM_FREQUENCY_BANDS; ++i)
        {
            result.bands[i].value += scalar;
        }
        return result;
    }


    // Overloaded operator to add all band values by a constant
    MicFrequencyBands& operator+=(float scalar)
    {
        for (int i = 0; i < NUM_FREQUENCY_BANDS; ++i)
        {
            bands[i].value += scalar;
        }
        return *this;
    }

    // Overloaded operator to add two FrequencyBands collections
    MicFrequencyBands operator+(const MicFrequencyBands& other) const
    {
        if (bands.size() != other.bands.size())
        {
            throw std::runtime_error("FrequencyBands collections must have the same number of bands.");
        }

        MicFrequencyBands result = *this;
        for (int i = 0; i < NUM_FREQUENCY_BANDS; ++i)
        {
            result.bands[i].value += other.bands[i].value;
        }
        return result;
    }

    // Overloaded operator to add and assign a FrequencyBands object
    MicFrequencyBands& operator+=(const MicFrequencyBands& other)
    {
        if (bands.size() != other.bands.size())
        {
            throw std::runtime_error("FrequencyBands collections must have the same number of bands.");
        }

        for (int i = 0; i < NUM_FREQUENCY_BANDS; ++i)
        {
            bands[i].value += other.bands[i].value;
        }
        return *this;
    }

    MicFrequencyBands& operator=(const MicFrequencyBands& other)
    {
        // Guard against self-assignment
        if (this != &other)
        {
            // Deep copy each band from other

            for (int i = 0; i < NUM_FREQUENCY_BANDS; ++i)
            {
                // Copy basic properties
                bands[i].minFrequency = other.bands[i].minFrequency;
                bands[i].maxFrequency = other.bands[i].maxFrequency;
                bands[i].centerFrequency = other.bands[i].centerFrequency;
                bands[i].value = other.bands[i].value;

                // Deep copy the biquad filter
                bands[i].biquad.b0 = other.bands[i].biquad.b0;
                bands[i].biquad.b1 = other.bands[i].biquad.b1;
                bands[i].biquad.b2 = other.bands[i].biquad.b2;
                bands[i].biquad.a0 = other.bands[i].biquad.a0;
                bands[i].biquad.a1 = other.bands[i].biquad.a1;
                bands[i].biquad.a2 = other.bands[i].biquad.a2;

                // Copy filter state variables
                bands[i].biquad.z1 = other.bands[i].biquad.z1;
                bands[i].biquad.z2 = other.bands[i].biquad.z2;
            }
        }

        return *this;
    }


    std::string toString() const
    {
        std::string result = "Frequency Bands:\n";

        // Iterate through each frequency band
        for (size_t i = 0; i < bands.size(); ++i)
        {
            // Get band data
            const auto& band = bands[i];

            // Format gain in dB
            float gainDb = band.value <= 0.0f ? -100.0f : 20.0f * std::log10(band.value);
            std::string gainString = gainDb <= -100.0f ? "-inf dB" : std::to_string(gainDb) + " dB";

            // Add band header with frequency range and gain
            result += std::to_string(i + 1) + ". " +
                      std::to_string(band.minFrequency) + " - " +
                      std::to_string(band.maxFrequency) + " Hz: " +
                      "Gain: " + gainString;


            result += "\n   Bicubic Filter: Enabled";
            result += "\n   - Alpha: " + std::to_string(band.biquad.a0);
            result += "\n   - Beta: " + std::to_string(band.biquad.b0);


            // Add empty line between bands for better readability
            result += "\n\n";
        }

        return result;
    }

};
