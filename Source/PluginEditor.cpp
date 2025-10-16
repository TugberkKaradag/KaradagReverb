#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
NewProjectAudioProcessorEditor::NewProjectAudioProcessorEditor(NewProjectAudioProcessor& p)
    : AudioProcessorEditor(&p), audioProcessor(p)
{
    
    auto setupSlider = [&](juce::Slider& slider, juce::Label& label, const juce::String& text)
        {
            slider.setSliderStyle(juce::Slider::SliderStyle::RotaryHorizontalVerticalDrag);
            slider.setTextBoxStyle(juce::Slider::TextBoxBelow, true, 80, 20);
            addAndMakeVisible(slider);

            label.setText(text, juce::dontSendNotification);
            label.setJustificationType(juce::Justification::centred);
            addAndMakeVisible(label);
        };

   
    setupSlider(roomSizeSlider, roomSizeLabel, "Room Size");
    setupSlider(dampingSlider, dampingLabel, "Tone / Damping");
    setupSlider(wetSlider, wetLabel, "Wet");
    setupSlider(drySlider, dryLabel, "Dry");
    setupSlider(widthSlider, widthLabel, "Width");

    
    roomSizeAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "ROOMSIZE", roomSizeSlider);
    dampingAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "DAMPING", dampingSlider);
    wetAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "WET", wetSlider);
    dryAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "DRY", drySlider);
    widthAttachment = std::make_unique<SliderAttachment>(audioProcessor.apvts, "WIDTH", widthSlider);

    setSize(600, 220); 
}

NewProjectAudioProcessorEditor::~NewProjectAudioProcessorEditor()
{
}

//==============================================================================
void NewProjectAudioProcessorEditor::paint(juce::Graphics& g)
{

    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

    g.setColour(juce::Colours::whitesmoke.withAlpha(0.7f)); 

    g.setFont(juce::Font("Segoe UI", 26.0f, juce::Font::bold));


    auto titleArea = getLocalBounds().removeFromTop(50);


    g.drawText("Karadag Reverb", titleArea, juce::Justification::centred, true);

}

void NewProjectAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds();

    bounds.removeFromTop(50);

    auto componentWidth = bounds.getWidth() / 5;

    auto createSlice = [&](juce::Label& label, juce::Slider& slider)
        {
            auto slice = bounds.removeFromLeft(componentWidth);
            label.setBounds(slice.removeFromTop(20));
            slider.setBounds(slice.reduced(10));
        };

    createSlice(roomSizeLabel, roomSizeSlider);
    createSlice(dampingLabel, dampingSlider);
    createSlice(wetLabel, wetSlider);
    createSlice(dryLabel, drySlider);
    createSlice(widthLabel, widthSlider);
}