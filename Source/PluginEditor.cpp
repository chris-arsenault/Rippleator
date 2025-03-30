#include "PluginEditor.h"

RippleatorAudioProcessorEditor::RippleatorAudioProcessorEditor(RippleatorAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
      tabbedComponent(juce::TabbedButtonBar::TabsAtTop),
      chamberVisualizer(p.getChamber()),
      zoneManager(p.getChamber()),
      visualizationsTab(p.getChamber()),
      tabNameResetCounter(0)
{
    // Set up title
    titleLabel.setText("Rippleator", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);
    
    // Set up tabbed component
    tabbedComponent.addTab("Chamber", juce::Colours::darkgrey, &chamberVisualizer, false);
    tabbedComponent.addTab("Zones", juce::Colours::darkgrey, &zoneManager, false);
    tabbedComponent.addTab("Visualizations", juce::Colours::darkgrey, &visualizationsTab, false);
    tabbedComponent.setCurrentTabIndex(0);
    addAndMakeVisible(tabbedComponent);
    
    // Set up bypass button
    bypassButton.setButtonText("Bypass Processing");
    bypassButton.setToggleState(audioProcessor.isBypassProcessingEnabled(), juce::dontSendNotification);
    bypassButton.onClick = [this] { 
        bool newState = bypassButton.getToggleState();
        audioProcessor.setBypassProcessing(newState);
    };
    addAndMakeVisible(bypassButton);
    
    // Set up chamber parameter controls
    densityLabel.setText("Medium Density", juce::dontSendNotification);
    densityLabel.setFont(juce::Font(14.0f));
    densityLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(densityLabel);
    
    densitySlider.setSliderStyle(juce::Slider::LinearHorizontal);
    densitySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    densitySlider.setRange(0.1, 10.0, 0.1);
    addAndMakeVisible(densitySlider);
    
    reflectivityLabel.setText("Wall Reflectivity", juce::dontSendNotification);
    reflectivityLabel.setFont(juce::Font(14.0f));
    reflectivityLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(reflectivityLabel);
    
    reflectivitySlider.setSliderStyle(juce::Slider::LinearHorizontal);
    reflectivitySlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    reflectivitySlider.setRange(0.0, 1.0, 0.01);
    addAndMakeVisible(reflectivitySlider);
    
    dampingLabel.setText("Wall Damping", juce::dontSendNotification);
    dampingLabel.setFont(juce::Font(14.0f));
    dampingLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(dampingLabel);
    
    dampingSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    dampingSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    dampingSlider.setRange(0.0, 1.0, 0.01);
    addAndMakeVisible(dampingSlider);
    
    outputGainLabel.setText("Output Gain", juce::dontSendNotification);
    outputGainLabel.setFont(juce::Font(14.0f));
    outputGainLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(outputGainLabel);
    
    outputGainSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    outputGainSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
    outputGainSlider.setRange(-20.0, 20.0, 0.1);
    addAndMakeVisible(outputGainSlider);
    
    inputLevelMeter.setRange(-20.0, 0.0, 0.1);
    addAndMakeVisible(inputLevelMeter);
    
    outputLevelMeter.setRange(-20.0, 0.0, 0.1);
    addAndMakeVisible(outputLevelMeter);
    
    // Create parameter attachments
    auto& parameters = audioProcessor.getParameters();
    densityAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        parameters, "mediumDensity", densitySlider));
    reflectivityAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        parameters, "wallReflectivity", reflectivitySlider));
    dampingAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        parameters, "wallDamping", dampingSlider));
    outputGainAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        parameters, "outputGain", outputGainSlider));
    
    // Set up microphone controls
    for (int i = 0; i < 3; ++i)
    {
        auto& mic = micControls[i];
        juce::String prefix = "mic" + juce::String(i + 1);
        
        // Label
        mic.label.setText("Mic " + juce::String(i + 1), juce::dontSendNotification);
        mic.label.setFont(juce::Font(14.0f, juce::Font::bold));
        mic.label.setJustificationType(juce::Justification::centredLeft);
        addAndMakeVisible(mic.label);
        
        // Volume slider
        mic.volumeSlider.setSliderStyle(juce::Slider::LinearHorizontal);
        mic.volumeSlider.setTextBoxStyle(juce::Slider::TextBoxRight, false, 60, 20);
        mic.volumeSlider.setRange(0.0, 2.0, 0.01);
        addAndMakeVisible(mic.volumeSlider);
        
        // Solo button
        mic.soloButton.setButtonText("S");
        mic.soloButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::yellow);
        addAndMakeVisible(mic.soloButton);
        
        // Mute button
        mic.muteButton.setButtonText("M");
        mic.muteButton.setColour(juce::TextButton::buttonOnColourId, juce::Colours::red);
        addAndMakeVisible(mic.muteButton);
        
        // Level meter
        addAndMakeVisible(mic.levelMeter);
        
        // Parameter attachments
        mic.volumeAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
            parameters, prefix + "Volume", mic.volumeSlider));
        mic.soloAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(
            parameters, prefix + "Solo", mic.soloButton));
        mic.muteAttachment.reset(new juce::AudioProcessorValueTreeState::ButtonAttachment(
            parameters, prefix + "Mute", mic.muteButton));
    }
    
    // Enable keyboard focus to receive key events
    setWantsKeyboardFocus(true);
    
    // Start timer for level meter updates
    startTimerHz(30); // Update 30 times per second
    
    // Set window size
    setSize(800, 800);
    
}

RippleatorAudioProcessorEditor::~RippleatorAudioProcessorEditor()
{
    stopTimer();
    densityAttachment.reset();
    reflectivityAttachment.reset();
    dampingAttachment.reset();
    outputGainAttachment.reset();
    for (int i = 0; i < 3; ++i)
    {
        auto& mic = micControls[i];
        mic.volumeAttachment.reset();
        mic.soloAttachment.reset();
        mic.muteAttachment.reset();
    }
}

void RippleatorAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
}

void RippleatorAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(10);
    
    // Title at the top
    titleLabel.setBounds(area.removeFromTop(30));
    
    // Add bypass button
    bypassButton.setBounds(area.removeFromTop(30).withSizeKeepingCentre(150, 24));
    
    area.removeFromTop(10); // Add some spacing
    
    // Tabbed component takes most of the space
    tabbedComponent.setBounds(area.removeFromTop(400));
    
    area.removeFromTop(10); // Add some spacing
    
    // Controls at bottom
    auto controlsArea = area;
    
    // First row of controls
    auto row1 = controlsArea.removeFromTop(30);
    densityLabel.setBounds(row1.removeFromLeft(120));
    densitySlider.setBounds(row1.removeFromLeft(200));
    
    // Add spacing
    controlsArea.removeFromTop(5);
    
    // Second row of controls
    auto row2 = controlsArea.removeFromTop(30);
    reflectivityLabel.setBounds(row2.removeFromLeft(120));
    reflectivitySlider.setBounds(row2.removeFromLeft(200));
    
    // Add spacing
    controlsArea.removeFromTop(5);
    
    // Third row of controls
    auto row3 = controlsArea.removeFromTop(30);
    dampingLabel.setBounds(row3.removeFromLeft(120));
    dampingSlider.setBounds(row3.removeFromLeft(200));
    
    // Add spacing
    controlsArea.removeFromTop(5);
    
    // Fourth row of controls
    auto row4 = controlsArea.removeFromTop(30);
    outputGainLabel.setBounds(row4.removeFromLeft(120));
    outputGainSlider.setBounds(row4.removeFromLeft(200));
    
    // Level meters
    auto metersArea = row4.removeFromRight(200);
    inputLevelMeter.setBounds(metersArea.removeFromLeft(90));
    outputLevelMeter.setBounds(metersArea);
    
    // Add spacing
    controlsArea.removeFromTop(5);
    
    // Microphone controls
    for (int i = 0; i < 3; ++i)
    {
        auto& mic = micControls[i];
        auto micArea = controlsArea.removeFromTop(30);
        
        mic.label.setBounds(micArea.removeFromLeft(60));
        
        auto meterWidth = 20;
        auto buttonWidth = 30;
        
        mic.levelMeter.setBounds(micArea.removeFromLeft(meterWidth));
        micArea.removeFromLeft(5); // spacing
        
        mic.soloButton.setBounds(micArea.removeFromRight(buttonWidth));
        micArea.removeFromRight(5); // spacing
        
        mic.muteButton.setBounds(micArea.removeFromRight(buttonWidth));
        micArea.removeFromRight(5); // spacing
        
        mic.volumeSlider.setBounds(micArea);
        
        controlsArea.removeFromTop(5);
    }
}

void RippleatorAudioProcessorEditor::timerCallback()
{
    // Update level meters
    for (int i = 0; i < 3; ++i)
    {
        float level = audioProcessor.getMicrophoneLevel(i);
        micControls[i].levelMeter.setLevel(level);
    }
    
    // Update chamber visualizer
    chamberVisualizer.repaint();
    
    // Update bypass button state (in case it was changed via keyboard shortcut)
    bypassButton.setToggleState(audioProcessor.isBypassProcessingEnabled(), juce::dontSendNotification);
    
    // Check if we need to reset tab names (if the timer was started for that purpose)
    if (tabNameResetCounter > 0)
    {
        tabNameResetCounter--;
        if (tabNameResetCounter == 0)
        {
            // Reset tab names
            tabbedComponent.setTabName(0, "Chamber");
            tabbedComponent.setTabName(1, "Zones");
            tabbedComponent.setTabName(2, "Visualizations");
        }
    }
}

bool RippleatorAudioProcessorEditor::keyPressed(const juce::KeyPress& key)
{
    // Toggle bypass processing with 'B' key
    if (key.getKeyCode() == 'B')
    {
        bool currentBypass = audioProcessor.isBypassProcessingEnabled();
        audioProcessor.setBypassProcessing(!currentBypass);
        
        // Show a message about the current state
        juce::String message = "Processing " + juce::String(!currentBypass ? "enabled" : "bypassed");
        tabbedComponent.setTabName(tabbedComponent.getCurrentTabIndex(), message);
        
        // Set counter to reset tab name after a number of timer callbacks
        // Since timer is running at 30Hz, 60 callbacks = 2 seconds
        tabNameResetCounter = 60;
        
        return true;
    }
    
    return juce::Component::keyPressed(key);
}
