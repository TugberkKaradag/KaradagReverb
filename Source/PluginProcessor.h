/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================

const int numCombs = 8;
const int numAllPasses = 4;
const float fixedGain = 0.015f;
const float scaleWet = 3.0f;
const float scaleDry = 2.0f;
const float scaleDamp = 0.4f;
const float scaleRoom = 0.28f;
const float offsetRoom = 0.7f;

//==============================================================================

class Comb
{
public:
    Comb();
    void setBuffer(float* buf, int size);
    float process(float input);
    void mute();
    void setDamp(float value);
    void setFeedback(float value);

private:
    float feedback = 0.0f;
    float filterStore = 0.0f;
    float damp1 = 0.0f;
    float damp2 = 0.0f;
    float* buffer = nullptr;
    int bufSize = 0;
    int bufIdx = 0;
};

class AllPass
{
public:
    AllPass();
    void setBuffer(float* buf, int size);
    float process(float input);
    void mute();

private:
    float feedback = 0.5f;
    float* buffer = nullptr;
    int bufSize = 0;
    int bufIdx = 0;
};

//==============================================================================
class NewProjectAudioProcessor : public juce::AudioProcessor
{
public:
    NewProjectAudioProcessor();
    ~NewProjectAudioProcessor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;
    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int index, const juce::String& newName) override;
    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    void updateParams();

    juce::AudioProcessorValueTreeState apvts;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    
    Comb comb[numCombs * 2]; 
    AllPass allPass[numAllPasses * 2];

    // 
    juce::AudioBuffer<float> combBuffers;
    juce::AudioBuffer<float> allPassBuffers;

    
    float roomSize = 0.5f, damp = 0.5f, wet = 0.33f, dry = 0.7f, width = 1.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NewProjectAudioProcessor)
};