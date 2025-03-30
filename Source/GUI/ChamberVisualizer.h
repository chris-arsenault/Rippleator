#pragma once

#include <JuceHeader.h>
#include "../Models/Chamber.h"

/**
 * Component that visualizes the chamber's wave propagation in real-time.
 * Shows the pressure field as a color map, along with speaker and microphone positions.
 */
class ChamberVisualizer : public juce::Component,
                         public juce::Timer
{
public:
    ChamberVisualizer(Chamber& chamber);
    ~ChamberVisualizer() override;
    
    void paint(juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    
    /**
     * Set the color map for the visualization
     * @param from The starting color of the gradient
     * @param to The ending color of the gradient
     */
    void setColorMap(const juce::Colour& from, const juce::Colour& to);
    
    void startTimer(int intervalMs = 50) { juce::Timer::startTimer(intervalMs); }
    void stopTimer() { juce::Timer::stopTimer(); }
    
    // Mouse event handlers for dragging elements
    void mouseDown(const juce::MouseEvent& e) override;
    void mouseDrag(const juce::MouseEvent& e) override;
    void mouseUp(const juce::MouseEvent& e) override;

private:
    Chamber& chamber;
    juce::ColourGradient colorMap;
    
    // Dragging state
    enum class DragTarget {
        None,
        Microphone,
        Speaker,
        ZoneCorner
    };
    
    DragTarget currentDragTarget = DragTarget::None;
    int draggedMicIndex = -1;
    int draggedZoneIndex = -1;
    enum class ZoneCorner {
        TopLeft,
        TopRight,
        BottomLeft,
        BottomRight
    };
    ZoneCorner draggedCorner = ZoneCorner::TopLeft;
    
    // Helper methods to check if a point is near a draggable element
    int getMicrophoneAtPosition(const juce::Point<float>& position);
    bool isSpeakerAtPosition(const juce::Point<float>& position);
    bool getZoneCornerAtPosition(const juce::Point<float>& position, int& zoneIndex, ZoneCorner& corner);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChamberVisualizer)
};
