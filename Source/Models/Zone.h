#pragma once

#include <JuceHeader.h>

/**
 * Represents a zone in the chamber with its own density properties.
 * Each zone is defined by its position (x,y), dimensions (width,height)
 * and a density value that affects wave propagation within the zone.
 */
struct Zone
{
    float x;          // Left position (0-1)
    float y;          // Top position (0-1)
    float width;      // Width (0-1)
    float height;     // Height (0-1)
    float density;    // Density of the medium in this zone
};
