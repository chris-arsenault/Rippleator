#pragma once

#include <JuceHeader.h>
#include "../Models/Chamber.h"

/**
 * Component that manages the creation and modification of zones in the chamber.
 * Provides UI controls for adding, removing, and adjusting zone properties.
 */
class ZoneManager : public juce::Component,
                   public juce::Button::Listener,
                   public juce::Slider::Listener
{
public:
    ZoneManager(Chamber& chamber);
    ~ZoneManager() override;
    
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // Button::Listener
    void buttonClicked(juce::Button* button) override;
    
    // Slider::Listener
    void sliderValueChanged(juce::Slider* slider) override;

private:
    void addNewZone();
    void removeZone(int index);
    void updateZoneDensity(int index, float density);
    void updateZoneBounds(int index, float x1, float y1, float x2, float y2);
    
    Chamber& chamber;
    
    std::unique_ptr<juce::TextButton> addZoneButton;
    
    juce::OwnedArray<juce::TextButton> removeButtons;
    juce::OwnedArray<juce::Label> densityLabels;
    juce::OwnedArray<juce::Slider> densitySliders;
    
    juce::OwnedArray<juce::Label> positionLabels;
    juce::OwnedArray<juce::Label> x1Labels;
    juce::OwnedArray<juce::Slider> x1Sliders;
    juce::OwnedArray<juce::Label> y1Labels;
    juce::OwnedArray<juce::Slider> y1Sliders;
    juce::OwnedArray<juce::Label> x2Labels;
    juce::OwnedArray<juce::Slider> x2Sliders;
    juce::OwnedArray<juce::Label> y2Labels;
    juce::OwnedArray<juce::Slider> y2Sliders;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ZoneManager)
};
