#pragma once

#include <array>
#include <cmath>

struct FrequencyBand
{
    float minFrequency;
    float maxFrequency;
    float value;
};

struct MicFrequencyBands
{
    static constexpr int NUM_FREQUENCY_BANDS = 24;
    static constexpr int MIN_FREQUENCY = 20;
    static constexpr int MAX_FREQUENCY = 30000;
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
    void reset()
    {
        for (int i = 0; i < NUM_FREQUENCY_BANDS; ++i)
        {
            bands[i].value = 0.0f;
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

    // Overloaded operator to add two FrequencyBands collections
    MicFrequencyBands& operator=(const MicFrequencyBands& other)
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
};
