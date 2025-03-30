#pragma once

#include <JuceHeader.h>

/**
 * Represents a zone in the chamber with its own density properties.
 * Each zone is defined by its rectangular bounds (x1,y1) to (x2,y2)
 * and a density value that affects wave propagation within the zone.
 */
struct Zone
{
    float x1;        // Left boundary (0-1)
    float y1;        // Top boundary (0-1)
    float x2;        // Right boundary (0-1)
    float y2;        // Bottom boundary (0-1)
    float density;   // Density of the medium in this zone
};
