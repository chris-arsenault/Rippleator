#pragma once
#include <array>
#include "../DebugLogger.h"

struct CircularBuffer
{
    std::array<float, 48000> buffer;
    int writeIndex = 0;
    int readIndex = 0;
    int size = 48000;

    CircularBuffer()
    {
        buffer.fill(0.0f);
    }

    void addSamples(const float* sample, const int numSamples)
    {
        if (numSamples > size) {
          DebugLogger::logWithCategory("ERROR", "CircularBuffer::addSamples: numSamples > size");
        }

        for (int i = 0; i < numSamples; ++i)
        {
            int bufferIndex = (writeIndex + i) % size;
            buffer[bufferIndex] = sample[i];
        }
        writeIndex = (writeIndex + numSamples) % size;
    }

    int getSamples(float* outputBuffer, const int requestedSamples)
    {
        int samplesToRead = 0;
        int projectedWriteIndex = writeIndex;

        if (readIndex > writeIndex)
            projectedWriteIndex = writeIndex + size;

        if (readIndex + requestedSamples > projectedWriteIndex)
            samplesToRead = projectedWriteIndex - readIndex;
        else
            samplesToRead = requestedSamples;

        for (int i = 0; i < samplesToRead; ++i)
        {
            int bufferIndex = (readIndex + i) % size;
            outputBuffer[i] = buffer[bufferIndex];
        }

        readIndex = (readIndex + samplesToRead) % size;

        return samplesToRead;
    }
};