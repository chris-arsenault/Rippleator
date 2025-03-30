#include "ChamberVisualizer.h"
#include "../PluginProcessor.h"

ChamberVisualizer::ChamberVisualizer(Chamber& chamber)
    : chamber(chamber)
{
    // Initialize color map
    setColorMap(juce::Colours::blue, juce::Colours::red);
    
    // Start timer for regular updates
    startTimer(50);
    
    // Enable mouse events
    setMouseCursor(juce::MouseCursor::PointingHandCursor);
}

ChamberVisualizer::~ChamberVisualizer()
{
    stopTimer();
}

void ChamberVisualizer::paint(juce::Graphics& g)
{
    auto bounds = getLocalBounds().toFloat();
    
    // Draw background
    g.fillAll(juce::Colours::black);
    
    // Draw ray paths
    const auto& rays = chamber.getCachedRays();
    
    // Draw rays with varying colors based on intensity and bounce count
    for (const auto& ray : rays)
    {
        // Skip rays with zero intensity
        if (ray.intensity <= 0.01f)
            continue;
            
        // Calculate ray endpoint based on direction and distance
        float rayLength = 0.1f; // Default length if distance is not set
        if (ray.distance > 0)
            rayLength = ray.distance;
            
        juce::Point<float> rayEnd = ray.origin + ray.direction * rayLength;
        
        // Calculate color based on ray intensity and bounce count
        float intensity = juce::jlimit(0.0f, 1.0f, ray.intensity);
        float hue = 0.6f - (float)ray.bounceCount * 0.1f; // Hue shifts with bounce count
        hue = juce::jlimit(0.0f, 1.0f, hue);
        
        juce::Colour rayColor = juce::Colour::fromHSV(hue, 0.8f, intensity, 0.7f);
        
        // Draw ray line
        g.setColour(rayColor);
        g.drawLine(
            ray.origin.x * bounds.getWidth(), 
            ray.origin.y * bounds.getHeight(),
            rayEnd.x * bounds.getWidth(), 
            rayEnd.y * bounds.getHeight(),
            1.0f + intensity * 2.0f // Line thickness based on intensity
        );
    }
    
    // Draw zone boundaries
    const auto& zones = chamber.getZones();
    
    for (int i = 0; i < zones.size(); ++i)
    {
        const auto& zone = zones[i];
        
        // Convert zone coordinates from 0-1 range to pixel coordinates
        float x = zone->x * bounds.getWidth();
        float y = zone->y * bounds.getHeight();
        float width = zone->width * bounds.getWidth();
        float height = zone->height * bounds.getHeight();
        
        // Use different color for the zone being dragged
        if (currentDragTarget == DragTarget::ZoneCorner && i == draggedZoneIndex)
            g.setColour(juce::Colours::orange.withAlpha(0.7f));
        else
            g.setColour(juce::Colours::red);
            
        // Draw zone boundary rectangle
        g.drawRect(x, y, width, height, 2.0f);
        
        // Draw corner handles
        float handleSize = 8.0f;
        g.fillRect(x - handleSize/2, y - handleSize/2, handleSize, handleSize); // Top-left
        g.fillRect(x + width - handleSize/2, y - handleSize/2, handleSize, handleSize); // Top-right
        g.fillRect(x - handleSize/2, y + height - handleSize/2, handleSize, handleSize); // Bottom-left
        g.fillRect(x + width - handleSize/2, y + height - handleSize/2, handleSize, handleSize); // Bottom-right
        
        // Add a small label showing the zone density
        g.setFont(14.0f);
        juce::String densityText = "D: " + juce::String(zone->density, 1);
        g.setColour(juce::Colours::white);
        g.drawText(densityText, x + 5, y + 5, 50, 20, juce::Justification::left);
    }
    
    // Draw speaker position
    auto speakerPos = chamber.getSpeakerPosition();
    float speakerX = speakerPos.x * bounds.getWidth();
    float speakerY = speakerPos.y * bounds.getHeight();
    
    // Use different color for the speaker being dragged
    if (currentDragTarget == DragTarget::Speaker)
        g.setColour(juce::Colours::orange);
    else
        g.setColour(juce::Colours::yellow);
        
    g.fillEllipse(speakerX - 5.0f, speakerY - 5.0f, 10.0f, 10.0f);
    g.drawText("S", speakerX - 4.0f, speakerY - 15.0f, 10.0f, 10.0f, juce::Justification::centred);
    
    // Draw microphone positions
    for (int i = 0; i < 3; ++i)
    {
        auto micPos = chamber.getMicrophonePosition(i);
        float micX = micPos.x * bounds.getWidth();
        float micY = micPos.y * bounds.getHeight();
        
        // Use different color for the microphone being dragged
        if (currentDragTarget == DragTarget::Microphone && i == draggedMicIndex)
            g.setColour(juce::Colours::orange);
        else
            g.setColour(juce::Colours::green);
            
        g.fillEllipse(micX - 4.0f, micY - 4.0f, 8.0f, 8.0f);
        g.drawText(juce::String(i + 1), micX - 4.0f, micY - 15.0f, 10.0f, 10.0f, juce::Justification::centred);
    }
    
    // Draw chamber walls
    g.setColour(juce::Colours::white);
    g.drawRect(bounds, 2.0f);
}

void ChamberVisualizer::resized()
{
    // Nothing to do here
}

void ChamberVisualizer::timerCallback()
{
    repaint();
}

void ChamberVisualizer::setColorMap(const juce::Colour& from, const juce::Colour& to)
{
    colorMap = juce::ColourGradient(from, 0, 0, to, 1, 1, false);
    colorMap.addColour(0.5, juce::Colours::white); // Add a midpoint for better visualization
}

void ChamberVisualizer::mouseDown(const juce::MouseEvent& e)
{
    auto mousePos = e.position;
    
    // Check if we're clicking on a microphone
    draggedMicIndex = getMicrophoneAtPosition(mousePos);
    if (draggedMicIndex >= 0)
    {
        currentDragTarget = DragTarget::Microphone;
        setMouseCursor(juce::MouseCursor::DraggingHandCursor);
        return;
    }
    
    // Check if we're clicking on the speaker
    if (isSpeakerAtPosition(mousePos))
    {
        currentDragTarget = DragTarget::Speaker;
        setMouseCursor(juce::MouseCursor::DraggingHandCursor);
        return;
    }
    
    // Check if we're clicking on a zone corner
    if (getZoneCornerAtPosition(mousePos, draggedZoneIndex, draggedCorner))
    {
        currentDragTarget = DragTarget::ZoneCorner;
        setMouseCursor(juce::MouseCursor::DraggingHandCursor);
        return;
    }
    
    // Not dragging anything
    currentDragTarget = DragTarget::None;
}

void ChamberVisualizer::mouseDrag(const juce::MouseEvent& e)
{
    auto bounds = getLocalBounds().toFloat();
    auto mousePos = e.position;
    
    // Convert to normalized coordinates (0-1 range)
    float normX = juce::jlimit(0.0f, 1.0f, mousePos.x / bounds.getWidth());
    float normY = juce::jlimit(0.0f, 1.0f, mousePos.y / bounds.getHeight());
    
    switch (currentDragTarget)
    {
        case DragTarget::Microphone:
            if (draggedMicIndex >= 0)
            {
                // Update microphone position
                chamber.setMicrophonePosition(draggedMicIndex, normX, normY);
            }
            break;
            
        case DragTarget::Speaker:
        {
            // Allow speaker to be placed anywhere in the chamber
            chamber.setSpeakerPosition(normX, normY);
            break;
        }
            
        case DragTarget::ZoneCorner:
            if (draggedZoneIndex >= 0)
            {
                const auto& zones = chamber.getZones();
                if (draggedZoneIndex < zones.size())
                {
                    // Get current zone bounds
                    float x = zones[draggedZoneIndex]->x;
                    float y = zones[draggedZoneIndex]->y;
                    float width = zones[draggedZoneIndex]->width;
                    float height = zones[draggedZoneIndex]->height;
                    
                    // Update the appropriate corner
                    switch (draggedCorner)
                    {
                        case ZoneCorner::TopLeft:
                            x = normX;
                            y = normY;
                            break;
                        case ZoneCorner::TopRight:
                            width = normX - x;
                            y = normY;
                            break;
                        case ZoneCorner::BottomLeft:
                            x = normX;
                            height = normY - y;
                            break;
                        case ZoneCorner::BottomRight:
                            width = normX - x;
                            height = normY - y;
                            break;
                    }
                    
                    // Ensure width and height are positive
                    if (width < 0) {
                        x = x + width;
                        width = -width;
                    }
                    if (height < 0) {
                        y = y + height;
                        height = -height;
                    }
                    
                    // Update zone bounds
                    chamber.setZoneBounds(draggedZoneIndex, x, y, width, height);
                }
            }
            break;
            
        case DragTarget::None:
            // Not dragging anything
            break;
    }
    
    // Repaint to show the updated positions
    repaint();
}

void ChamberVisualizer::mouseUp(const juce::MouseEvent& e)
{
    if (currentDragTarget != DragTarget::None)
    {
        currentDragTarget = DragTarget::None;
        draggedMicIndex = -1;
        draggedZoneIndex = -1;
        setMouseCursor(juce::MouseCursor::PointingHandCursor);
    }
}

int ChamberVisualizer::getMicrophoneAtPosition(const juce::Point<float>& position)
{
    auto bounds = getLocalBounds().toFloat();
    
    for (int i = 0; i < 3; ++i)
    {
        auto micPos = chamber.getMicrophonePosition(i);
        float micX = micPos.x * bounds.getWidth();
        float micY = micPos.y * bounds.getHeight();
        
        // Check if position is within the microphone circle (radius 8)
        float distance = std::sqrt(std::pow(position.x - micX, 2) + std::pow(position.y - micY, 2));
        if (distance <= 8.0f)
        {
            return i;
        }
    }
    
    return -1; // No microphone at this position
}

bool ChamberVisualizer::isSpeakerAtPosition(const juce::Point<float>& position)
{
    auto bounds = getLocalBounds().toFloat();
    auto speakerPos = chamber.getSpeakerPosition();
    float speakerX = speakerPos.x * bounds.getWidth();
    float speakerY = speakerPos.y * bounds.getHeight();
    
    // Check if position is within the speaker circle (radius 10)
    float distance = std::sqrt(std::pow(position.x - speakerX, 2) + std::pow(position.y - speakerY, 2));
    return distance <= 10.0f;
}

bool ChamberVisualizer::getZoneCornerAtPosition(const juce::Point<float>& position, int& zoneIndex, ZoneCorner& corner)
{
    auto bounds = getLocalBounds().toFloat();
    const auto& zones = chamber.getZones();
    
    // Handle size for zone corners
    float handleSize = 8.0f;
    
    for (int i = 0; i < zones.size(); ++i)
    {
        const auto& zone = zones[i];
        
        // Convert zone coordinates from 0-1 range to pixel coordinates
        float x = zone->x * bounds.getWidth();
        float y = zone->y * bounds.getHeight();
        float width = zone->width * bounds.getWidth();
        float height = zone->height * bounds.getHeight();
        
        // Check top-left corner
        if (position.x >= x - handleSize/2 && position.x <= x + handleSize/2 &&
            position.y >= y - handleSize/2 && position.y <= y + handleSize/2)
        {
            zoneIndex = i;
            corner = ZoneCorner::TopLeft;
            return true;
        }
        
        // Check top-right corner
        if (position.x >= x + width - handleSize/2 && position.x <= x + width + handleSize/2 &&
            position.y >= y - handleSize/2 && position.y <= y + handleSize/2)
        {
            zoneIndex = i;
            corner = ZoneCorner::TopRight;
            return true;
        }
        
        // Check bottom-left corner
        if (position.x >= x - handleSize/2 && position.x <= x + handleSize/2 &&
            position.y >= y + height - handleSize/2 && position.y <= y + height + handleSize/2)
        {
            zoneIndex = i;
            corner = ZoneCorner::BottomLeft;
            return true;
        }
        
        // Check bottom-right corner
        if (position.x >= x + width - handleSize/2 && position.x <= x + width + handleSize/2 &&
            position.y >= y + height - handleSize/2 && position.y <= y + height + handleSize/2)
        {
            zoneIndex = i;
            corner = ZoneCorner::BottomRight;
            return true;
        }
    }
    
    return false; // No zone corner at this position
}
