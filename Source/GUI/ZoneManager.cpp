#include "ZoneManager.h"

ZoneManager::ZoneManager(Chamber& c)
    : chamber(c)
{
    addZoneButton = std::make_unique<juce::TextButton>("Add Zone");
    addZoneButton->addListener(this);
    addAndMakeVisible(addZoneButton.get());
}

ZoneManager::~ZoneManager()
{
    addZoneButton->removeListener(this);
    
    // Remove listeners from all controls
    for (auto* slider : densitySliders)
        slider->removeListener(this);
    for (auto* slider : x1Sliders)
        slider->removeListener(this);
    for (auto* slider : y1Sliders)
        slider->removeListener(this);
    for (auto* slider : x2Sliders)
        slider->removeListener(this);
    for (auto* slider : y2Sliders)
        slider->removeListener(this);
    for (auto* button : removeButtons)
        button->removeListener(this);
}

void ZoneManager::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
    
    // Draw border
    g.setColour(juce::Colours::grey);
    g.drawRect(getLocalBounds(), 1);
    
    // Draw title
    g.setColour(juce::Colours::white);
    g.setFont(16.0f);
    g.drawText("Zones", getLocalBounds().removeFromTop(20),
               juce::Justification::centred, true);
}

void ZoneManager::resized()
{
    auto bounds = getLocalBounds();
    
    // Title area
    bounds.removeFromTop(20);
    
    // Add zone button at top
    addZoneButton->setBounds(bounds.removeFromTop(30).reduced(5));
    
    // Space for zone controls
    bounds.removeFromTop(5);
    
    // Layout zone controls
    const int zoneHeight = 180;  // Increased to accommodate position sliders
    for (int i = 0; i < densitySliders.size(); ++i)
    {
        auto zoneArea = bounds.removeFromTop(zoneHeight).reduced(5);
        
        // Remove button at top
        removeButtons[i]->setBounds(zoneArea.removeFromTop(25));
        zoneArea.removeFromTop(5);
        
        // Density slider and label
        auto controlArea = zoneArea.removeFromTop(30);
        densityLabels[i]->setBounds(controlArea.removeFromLeft(80));
        densitySliders[i]->setBounds(controlArea);
        zoneArea.removeFromTop(5);
        
        // Position controls
        auto posLabelArea = zoneArea.removeFromTop(20);
        positionLabels[i]->setBounds(posLabelArea);
        
        // X1 slider
        auto x1Area = zoneArea.removeFromTop(25);
        x1Labels[i]->setBounds(x1Area.removeFromLeft(30));
        x1Sliders[i]->setBounds(x1Area);
        
        // Y1 slider
        auto y1Area = zoneArea.removeFromTop(25);
        y1Labels[i]->setBounds(y1Area.removeFromLeft(30));
        y1Sliders[i]->setBounds(y1Area);
        
        // X2 slider
        auto x2Area = zoneArea.removeFromTop(25);
        x2Labels[i]->setBounds(x2Area.removeFromLeft(30));
        x2Sliders[i]->setBounds(x2Area);
        
        // Y2 slider
        auto y2Area = zoneArea.removeFromTop(25);
        y2Labels[i]->setBounds(y2Area.removeFromLeft(30));
        y2Sliders[i]->setBounds(y2Area);
        
        bounds.removeFromTop(5); // Space between zones
    }
}

void ZoneManager::buttonClicked(juce::Button* button)
{
    if (button == addZoneButton.get())
    {
        addNewZone();
    }
    else
    {
        // Find which remove button was clicked
        for (int i = 0; i < removeButtons.size(); ++i)
        {
            if (button == removeButtons[i])
            {
                removeZone(i);
                break;
            }
        }
    }
}

void ZoneManager::sliderValueChanged(juce::Slider* slider)
{
    // Find which slider was changed
    for (int i = 0; i < densitySliders.size(); ++i)
    {
        if (slider == densitySliders[i])
        {
            updateZoneDensity(i, static_cast<float>(slider->getValue()));
            break;
        }
        else if (slider == x1Sliders[i] || slider == y1Sliders[i] ||
                 slider == x2Sliders[i] || slider == y2Sliders[i])
        {
            updateZoneBounds(i,
                           static_cast<float>(x1Sliders[i]->getValue()),
                           static_cast<float>(y1Sliders[i]->getValue()),
                           static_cast<float>(x2Sliders[i]->getValue()),
                           static_cast<float>(y2Sliders[i]->getValue()));
            break;
        }
    }
}

void ZoneManager::addNewZone()
{
    // Create zone in chamber with default values
    float defaultX = 0.3f;
    float defaultY = 0.3f;
    float defaultWidth = 0.2f;
    float defaultHeight = 0.2f;
    float defaultDensity = 2.0f;
    
    chamber.addZone(defaultX, defaultY, defaultWidth, defaultHeight, defaultDensity);
    int zoneId = removeButtons.size(); // Use the current number of zone controls
    
    // Create controls
    auto removeButton = std::make_unique<juce::TextButton>("Remove Zone " + juce::String(zoneId + 1));
    removeButton->addListener(this);
    addAndMakeVisible(removeButton.get());
    
    auto densityLabel = std::make_unique<juce::Label>();
    densityLabel->setText("Density", juce::dontSendNotification);
    addAndMakeVisible(densityLabel.get());
    
    auto densitySlider = std::make_unique<juce::Slider>();
    densitySlider->setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    densitySlider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 70, 20);
    densitySlider->setRange(0.1, 10.0);
    densitySlider->setValue(1.0);
    densitySlider->addListener(this);
    addAndMakeVisible(densitySlider.get());
    
    auto positionLabel = std::make_unique<juce::Label>();
    positionLabel->setText("Position", juce::dontSendNotification);
    positionLabel->setJustificationType(juce::Justification::centred);
    addAndMakeVisible(positionLabel.get());
    
    // Create position sliders
    auto createPosSlider = [this]() {
        auto slider = std::make_unique<juce::Slider>();
        slider->setSliderStyle(juce::Slider::LinearHorizontal);
        slider->setTextBoxStyle(juce::Slider::TextBoxRight, false, 50, 20);
        slider->setRange(0.0, 1.0);
        slider->addListener(this);
        addAndMakeVisible(slider.get());
        return slider;
    };
    
    auto x1Label = std::make_unique<juce::Label>();
    x1Label->setText("X1", juce::dontSendNotification);
    addAndMakeVisible(x1Label.get());
    auto x1Slider = createPosSlider();
    x1Slider->setValue(0.25);
    
    auto y1Label = std::make_unique<juce::Label>();
    y1Label->setText("Y1", juce::dontSendNotification);
    addAndMakeVisible(y1Label.get());
    auto y1Slider = createPosSlider();
    y1Slider->setValue(0.25);
    
    auto x2Label = std::make_unique<juce::Label>();
    x2Label->setText("X2", juce::dontSendNotification);
    addAndMakeVisible(x2Label.get());
    auto x2Slider = createPosSlider();
    x2Slider->setValue(0.75);
    
    auto y2Label = std::make_unique<juce::Label>();
    y2Label->setText("Y2", juce::dontSendNotification);
    addAndMakeVisible(y2Label.get());
    auto y2Slider = createPosSlider();
    y2Slider->setValue(0.75);
    
    // Store controls
    removeButtons.add(removeButton.release());
    densityLabels.add(densityLabel.release());
    densitySliders.add(densitySlider.release());
    positionLabels.add(positionLabel.release());
    x1Labels.add(x1Label.release());
    x1Sliders.add(x1Slider.release());
    y1Labels.add(y1Label.release());
    y1Sliders.add(y1Slider.release());
    x2Labels.add(x2Label.release());
    x2Sliders.add(x2Slider.release());
    y2Labels.add(y2Label.release());
    y2Sliders.add(y2Slider.release());
    
    resized();
}

void ZoneManager::removeZone(int index)
{
    if (index >= 0 && index < removeButtons.size())
    {
        // Remove zone from chamber
        chamber.removeZone(index);
        
        // Remove controls
        removeButtons.remove(index);
        densityLabels.remove(index);
        densitySliders.remove(index);
        positionLabels.remove(index);
        x1Labels.remove(index);
        x1Sliders.remove(index);
        y1Labels.remove(index);
        y1Sliders.remove(index);
        x2Labels.remove(index);
        x2Sliders.remove(index);
        y2Labels.remove(index);
        y2Sliders.remove(index);
        
        resized();
    }
}

void ZoneManager::updateZoneDensity(int index, float density)
{
    if (index >= 0 && index < densitySliders.size())
    {
        chamber.setZoneDensity(index, density);
    }
}

void ZoneManager::updateZoneBounds(int index, float x1, float y1, float x2, float y2)
{
    if (index >= 0 && index < densitySliders.size())
    {
        // Convert from (x1, y1, x2, y2) to (x, y, width, height)
        float x = x1;
        float y = y1;
        float width = x2 - x1;
        float height = y2 - y1;
        
        chamber.setZoneBounds(index, x, y, width, height);
    }
}
