#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================
class NewProjectAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    NewProjectAudioProcessorEditor(NewProjectAudioProcessor&);
    ~NewProjectAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    NewProjectAudioProcessor& audioProcessor;

    juce::Slider wetSlider, drySlider, roomSizeSlider, dampingSlider, widthSlider;
    juce::Label wetLabel, dryLabel, roomSizeLabel, dampingLabel, widthLabel;

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<SliderAttachment> wetAttachment, dryAttachment, roomSizeAttachment, dampingAttachment, widthAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NewProjectAudioProcessorEditor)
};