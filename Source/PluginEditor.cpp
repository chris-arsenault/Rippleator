#include "PluginEditor.h"

RippleatorAudioProcessorEditor::RippleatorAudioProcessorEditor(RippleatorAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p),
      chamberVisualizer(p.getChamber()),
      zoneManager(p.getChamber())
{
    // Set up title
    titleLabel.setText("Rippleator", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);
    
    // Set up chamber visualizer
    addAndMakeVisible(chamberVisualizer);
    
    // Set up zone manager
    addAndMakeVisible(zoneManager);
    
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
    
    // Create parameter attachments
    auto& parameters = audioProcessor.getParameters();
    densityAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        parameters, "mediumDensity", densitySlider));
    reflectivityAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        parameters, "wallReflectivity", reflectivitySlider));
    dampingAttachment.reset(new juce::AudioProcessorValueTreeState::SliderAttachment(
        parameters, "wallDamping", dampingSlider));
    
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
    
    // Set window size
    setSize(800, 600);
    
    // Start timer for level meter updates
    startTimerHz(30); // Update 30 times per second
}

RippleatorAudioProcessorEditor::~RippleatorAudioProcessorEditor()
{
    stopTimer();
}

void RippleatorAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkgrey);
}

void RippleatorAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(10);
    
    // Title at top
    titleLabel.setBounds(area.removeFromTop(30));
    
    // Chamber visualizer takes up most of the space
    auto chamberArea = area.removeFromTop(area.getHeight() * 2/3);
    chamberVisualizer.setBounds(chamberArea.removeFromLeft(chamberArea.getWidth() * 2/3));
    
    // Zone manager next to chamber visualizer
    zoneManager.setBounds(chamberArea);
    
    // Add spacing
    area.removeFromTop(10);
    
    // Chamber parameter controls
    auto controlHeight = 24;
    auto controlSpacing = 5;
    
    auto densityArea = area.removeFromTop(controlHeight);
    densityLabel.setBounds(densityArea.removeFromLeft(120));
    densitySlider.setBounds(densityArea);
    
    area.removeFromTop(controlSpacing);
    
    auto reflectivityArea = area.removeFromTop(controlHeight);
    reflectivityLabel.setBounds(reflectivityArea.removeFromLeft(120));
    reflectivitySlider.setBounds(reflectivityArea);
    
    area.removeFromTop(controlSpacing);
    
    auto dampingArea = area.removeFromTop(controlHeight);
    dampingLabel.setBounds(dampingArea.removeFromLeft(120));
    dampingSlider.setBounds(dampingArea);
    
    area.removeFromTop(10);
    
    // Microphone controls
    for (int i = 0; i < 3; ++i)
    {
        auto& mic = micControls[i];
        auto micArea = area.removeFromTop(controlHeight);
        
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
        
        area.removeFromTop(controlSpacing);
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
}
