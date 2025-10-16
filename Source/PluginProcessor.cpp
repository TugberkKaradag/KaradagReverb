/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================

const int combTunings[numCombs] = { 1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617 };
const int allPassTunings[numAllPasses] = { 556, 441, 341, 225 };

//==============================================================================
Comb::Comb() {}
void Comb::setBuffer(float* buf, int size) { buffer = buf; bufSize = size; }
void Comb::mute() { for (int i = 0; i < bufSize; i++) buffer[i] = 0; }

float Comb::process(float input)
{
    float output = buffer[bufIdx];
    filterStore = (output * damp2) + (filterStore * damp1);
    buffer[bufIdx] = input + (filterStore * feedback);
    if (++bufIdx >= bufSize) bufIdx = 0;
    return output;
}

void Comb::setDamp(float value) { damp1 = value; damp2 = 1 - value; }
void Comb::setFeedback(float value) { feedback = value; }

//==============================================================================
AllPass::AllPass() {}
void AllPass::setBuffer(float* buf, int size) { buffer = buf; bufSize = size; }
void AllPass::mute() { for (int i = 0; i < bufSize; i++) buffer[i] = 0; }

float AllPass::process(float input)
{
    float bufOut = buffer[bufIdx];
    float output = -input + bufOut;
    buffer[bufIdx] = input + (bufOut * feedback);
    if (++bufIdx >= bufSize) bufIdx = 0;
    return output;
}

//==============================================================================
NewProjectAudioProcessor::NewProjectAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor(BusesProperties()
        .withInput("Input", juce::AudioChannelSet::stereo(), true)
        .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
#endif
    apvts(*this, nullptr, "Parameters", createParameterLayout())
{
    updateParams();
}

NewProjectAudioProcessor::~NewProjectAudioProcessor() {}

//==============================================================================
juce::AudioProcessorValueTreeState::ParameterLayout NewProjectAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    params.push_back(std::make_unique<juce::AudioParameterFloat>("ROOMSIZE", "Room Size", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("DAMPING", "Tone / Damping", 0.0f, 1.0f, 0.5f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("WET", "Wet", 0.0f, 1.0f, 0.33f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("DRY", "Dry", 0.0f, 1.0f, 0.7f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("WIDTH", "Width", 0.0f, 1.0f, 1.0f));
    return { params.begin(), params.end() };
}

void NewProjectAudioProcessor::prepareToPlay(double sampleRate, int samplesPerBlock)
{
    int stereoSpread = 23; 

    int totalCombBufferSize = 0;
    for (int i = 0; i < numCombs; ++i)
    {
        totalCombBufferSize += combTunings[i];
        totalCombBufferSize += combTunings[i] + stereoSpread;
    }
    combBuffers.setSize(1, totalCombBufferSize);
    combBuffers.clear();

    int totalAllPassBufferSize = 0;
    for (int i = 0; i < numAllPasses; ++i)
    {
        totalAllPassBufferSize += allPassTunings[i];
        totalAllPassBufferSize += allPassTunings[i] + stereoSpread;
    }
    allPassBuffers.setSize(1, totalAllPassBufferSize);
    allPassBuffers.clear();

    int combBufferOffset = 0;
    int allPassBufferOffset = 0;
    for (int i = 0; i < numCombs; ++i)
    {
        comb[i].setBuffer(combBuffers.getWritePointer(0, combBufferOffset), combTunings[i]);
        combBufferOffset += combTunings[i];
        comb[i + numCombs].setBuffer(combBuffers.getWritePointer(0, combBufferOffset), combTunings[i] + stereoSpread);
        combBufferOffset += combTunings[i] + stereoSpread;
    }

    for (int i = 0; i < numAllPasses; ++i)
    {
        allPass[i].setBuffer(allPassBuffers.getWritePointer(0, allPassBufferOffset), allPassTunings[i]);
        allPassBufferOffset += allPassTunings[i];
        allPass[i + numAllPasses].setBuffer(allPassBuffers.getWritePointer(0, allPassBufferOffset), allPassTunings[i] + stereoSpread);
        allPassBufferOffset += allPassTunings[i] + stereoSpread;
    }
}

void NewProjectAudioProcessor::releaseResources() {}

bool NewProjectAudioProcessor::isBusesLayoutSupported(const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    if (layouts.getMainInputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return true;
}

void NewProjectAudioProcessor::updateParams()
{
    wet = apvts.getRawParameterValue("WET")->load();
    dry = apvts.getRawParameterValue("DRY")->load();
    roomSize = apvts.getRawParameterValue("ROOMSIZE")->load();
    damp = apvts.getRawParameterValue("DAMPING")->load();
    width = apvts.getRawParameterValue("WIDTH")->load();

    float wet1 = wet * (width / 2.0f + 0.5f);
    float wet2 = wet * ((1.0f - width) / 2.0f);

    float feedback = roomSize * scaleRoom + offsetRoom;
    float dampingValue = damp * scaleDamp;

    for (int i = 0; i < numCombs; ++i)
    {
        comb[i].setFeedback(feedback);
        comb[i + numCombs].setFeedback(feedback);
        comb[i].setDamp(dampingValue);
        comb[i + numCombs].setDamp(dampingValue);
    }
}

void NewProjectAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    updateParams();

    auto* leftChannel = buffer.getWritePointer(0);
    auto* rightChannel = buffer.getWritePointer(1);

    for (int i = 0; i < buffer.getNumSamples(); ++i)
    {
        float input = (leftChannel[i] + rightChannel[i]) * fixedGain;
        float outL = 0, outR = 0;

        for (int j = 0; j < numCombs; ++j)
        {
            outL += comb[j].process(input);
            outR += comb[j + numCombs].process(input);
        }

        for (int j = 0; j < numAllPasses; ++j)
        {
            outL = allPass[j].process(outL);
            outR = allPass[j + numAllPasses].process(outR);
        }

        float wet1 = wet * (width / 2.0f + 0.5f);
        float wet2 = wet * ((1.0f - width) / 2.0f);

        leftChannel[i] = outL * wet1 + outR * wet2 + leftChannel[i] * dry;
        rightChannel[i] = outR * wet1 + outL * wet2 + rightChannel[i] * dry;
    }
}

const juce::String NewProjectAudioProcessor::getName() const { return JucePlugin_Name; }
bool NewProjectAudioProcessor::acceptsMidi() const { return false; }
bool NewProjectAudioProcessor::producesMidi() const { return false; }
bool NewProjectAudioProcessor::isMidiEffect() const { return false; }
double NewProjectAudioProcessor::getTailLengthSeconds() const { return 2.0; }
int NewProjectAudioProcessor::getNumPrograms() { return 1; }
int NewProjectAudioProcessor::getCurrentProgram() { return 0; }
void NewProjectAudioProcessor::setCurrentProgram(int index) {}
const juce::String NewProjectAudioProcessor::getProgramName(int index) { return {}; }
void NewProjectAudioProcessor::changeProgramName(int index, const juce::String& newName) {}
bool NewProjectAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* NewProjectAudioProcessor::createEditor() { return new NewProjectAudioProcessorEditor(*this); }
void NewProjectAudioProcessor::getStateInformation(juce::MemoryBlock& destData) { auto state = apvts.copyState(); std::unique_ptr<juce::XmlElement> xml(state.createXml()); copyXmlToBinary(*xml, destData); }
void NewProjectAudioProcessor::setStateInformation(const void* data, int sizeInBytes) { std::unique_ptr<juce::XmlElement> xmlState(getXmlFromBinary(data, sizeInBytes)); if (xmlState.get() != nullptr) if (xmlState->hasTagName(apvts.state.getType())) apvts.replaceState(juce::ValueTree::fromXml(*xmlState)); }
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new NewProjectAudioProcessor(); }