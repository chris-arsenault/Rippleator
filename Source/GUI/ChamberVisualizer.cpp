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
    
    // Get chamber grid
    const auto& grid = chamber.getGrid();
    
    // Calculate cell size
    float cellWidth = bounds.getWidth() / Chamber::GRID_WIDTH;
    float cellHeight = bounds.getHeight() / Chamber::GRID_HEIGHT;
    
    // Draw pressure field
    for (int y = 0; y < Chamber::GRID_HEIGHT; ++y)
    {
        for (int x = 0; x < Chamber::GRID_WIDTH; ++x)
        {
            float pressure = grid[y * Chamber::GRID_WIDTH + x];
            
            // Map pressure to color
            float normalizedPressure = (pressure + 1.0f) * 0.5f; // Map [-1,1] to [0,1]
            normalizedPressure = juce::jlimit(0.0f, 1.0f, normalizedPressure);
            
            g.setColour(colorMap.getColourAtPosition(normalizedPressure));
            
            // Draw cell - fill the entire cell with no gaps
            g.fillRect(x * cellWidth, y * cellHeight, cellWidth + 0.5f, cellHeight + 0.5f);
        }
    }
    
    // Draw zone boundaries
    const auto& zones = chamber.getZones();
    
    for (int i = 0; i < zones.size(); ++i)
    {
        const auto& zone = zones[i];
        
        // Convert zone coordinates from 0-1 range to pixel coordinates
        float x1 = zone->x1 * bounds.getWidth();
        float y1 = zone->y1 * bounds.getHeight();
        float x2 = zone->x2 * bounds.getWidth();
        float y2 = zone->y2 * bounds.getHeight();
        
        // Use different color for the zone being dragged
        if (currentDragTarget == DragTarget::ZoneCorner && i == draggedZoneIndex)
            g.setColour(juce::Colours::orange.withAlpha(0.7f));
        else
            g.setColour(juce::Colours::white.withAlpha(0.7f));
            
        // Draw zone boundary rectangle
        g.drawRect(x1, y1, x2 - x1, y2 - y1, 2.0f);
        
        // Draw corner handles
        float handleSize = 8.0f;
        g.fillRect(x1 - handleSize/2, y1 - handleSize/2, handleSize, handleSize); // Top-left
        g.fillRect(x2 - handleSize/2, y1 - handleSize/2, handleSize, handleSize); // Top-right
        g.fillRect(x1 - handleSize/2, y2 - handleSize/2, handleSize, handleSize); // Bottom-left
        g.fillRect(x2 - handleSize/2, y2 - handleSize/2, handleSize, handleSize); // Bottom-right
        
        // Add a small label showing the zone density
        g.setFont(12.0f);
        juce::String densityText = juce::String(zone->density, 1);
        g.drawText(densityText, x1 + 2, y1 + 2, 30, 15, juce::Justification::left);
    }
    
    // Draw speaker position
    float speakerX = chamber.getSpeakerX() * bounds.getWidth();
    float speakerY = chamber.getSpeakerY() * bounds.getHeight();
    
    // Use different color for the speaker being dragged
    if (currentDragTarget == DragTarget::Speaker)
        g.setColour(juce::Colours::orange);
    else
        g.setColour(juce::Colours::yellow);
        
    g.fillEllipse(speakerX - 5.0f, speakerY - 5.0f, 10.0f, 10.0f);
    g.drawText("S", speakerX - 4.0f, speakerY - 15.0f, 10.0f, 10.0f, juce::Justification::centred);
    
    // Draw microphone positions
    const auto& micPositions = chamber.getMicrophonePositions();
    for (int i = 0; i < 3; ++i)
    {
        // Use different color for the microphone being dragged
        if (currentDragTarget == DragTarget::Microphone && i == draggedMicIndex)
            g.setColour(juce::Colours::orange);
        else
            g.setColour(juce::Colours::green);
            
        float micX = micPositions[i].x * bounds.getWidth();
        float micY = micPositions[i].y * bounds.getHeight();
        g.fillEllipse(micX - 4.0f, micY - 4.0f, 8.0f, 8.0f);
        g.drawText(juce::String(i + 1), micX - 4.0f, micY - 15.0f, 10.0f, 10.0f, juce::Justification::centred);
    }
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
                    float x1 = zones[draggedZoneIndex]->x1;
                    float y1 = zones[draggedZoneIndex]->y1;
                    float x2 = zones[draggedZoneIndex]->x2;
                    float y2 = zones[draggedZoneIndex]->y2;
                    
                    // Update the appropriate corner
                    switch (draggedCorner)
                    {
                        case ZoneCorner::TopLeft:
                            x1 = normX;
                            y1 = normY;
                            break;
                        case ZoneCorner::TopRight:
                            x2 = normX;
                            y1 = normY;
                            break;
                        case ZoneCorner::BottomLeft:
                            x1 = normX;
                            y2 = normY;
                            break;
                        case ZoneCorner::BottomRight:
                            x2 = normX;
                            y2 = normY;
                            break;
                    }
                    
                    // Ensure x1 < x2 and y1 < y2
                    if (x1 > x2) std::swap(x1, x2);
                    if (y1 > y2) std::swap(y1, y2);
                    
                    // Update zone bounds
                    chamber.setZoneBounds(draggedZoneIndex, x1, y1, x2, y2);
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
    const auto& micPositions = chamber.getMicrophonePositions();
    
    for (int i = 0; i < 3; ++i)
    {
        float micX = micPositions[i].x * bounds.getWidth();
        float micY = micPositions[i].y * bounds.getHeight();
        
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
    float speakerX = chamber.getSpeakerX() * bounds.getWidth();
    float speakerY = chamber.getSpeakerY() * bounds.getHeight();
    
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
        float x1 = zone->x1 * bounds.getWidth();
        float y1 = zone->y1 * bounds.getHeight();
        float x2 = zone->x2 * bounds.getWidth();
        float y2 = zone->y2 * bounds.getHeight();
        
        // Check top-left corner
        if (position.x >= x1 - handleSize/2 && position.x <= x1 + handleSize/2 &&
            position.y >= y1 - handleSize/2 && position.y <= y1 + handleSize/2)
        {
            zoneIndex = i;
            corner = ZoneCorner::TopLeft;
            return true;
        }
        
        // Check top-right corner
        if (position.x >= x2 - handleSize/2 && position.x <= x2 + handleSize/2 &&
            position.y >= y1 - handleSize/2 && position.y <= y1 + handleSize/2)
        {
            zoneIndex = i;
            corner = ZoneCorner::TopRight;
            return true;
        }
        
        // Check bottom-left corner
        if (position.x >= x1 - handleSize/2 && position.x <= x1 + handleSize/2 &&
            position.y >= y2 - handleSize/2 && position.y <= y2 + handleSize/2)
        {
            zoneIndex = i;
            corner = ZoneCorner::BottomLeft;
            return true;
        }
        
        // Check bottom-right corner
        if (position.x >= x2 - handleSize/2 && position.x <= x2 + handleSize/2 &&
            position.y >= y2 - handleSize/2 && position.y <= y2 + handleSize/2)
        {
            zoneIndex = i;
            corner = ZoneCorner::BottomRight;
            return true;
        }
    }
    
    return false; // No zone corner at this position
}
