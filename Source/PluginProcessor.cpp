#include "PluginProcessor.h"
#include "PluginEditor.h"

ExtasisVisionAudioProcessor::ExtasisVisionAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

ExtasisVisionAudioProcessor::~ExtasisVisionAudioProcessor() {}

const juce::String ExtasisVisionAudioProcessor::getName() const { return JucePlugin_Name; }
bool ExtasisVisionAudioProcessor::acceptsMidi() const { return true; }
bool ExtasisVisionAudioProcessor::producesMidi() const { return false; }
bool ExtasisVisionAudioProcessor::isMidiEffect() const { return false; }
double ExtasisVisionAudioProcessor::getTailLengthSeconds() const { return 0.0; }

int ExtasisVisionAudioProcessor::getNumPrograms() { return 1; }
int ExtasisVisionAudioProcessor::getCurrentProgram() { return 0; }
void ExtasisVisionAudioProcessor::setCurrentProgram (int index) { juce::ignoreUnused (index); }
const juce::String ExtasisVisionAudioProcessor::getProgramName (int index) { juce::ignoreUnused (index); return {}; }
void ExtasisVisionAudioProcessor::changeProgramName (int index, const juce::String& newName) { juce::ignoreUnused (index, newName); }

void ExtasisVisionAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock) { juce::ignoreUnused (sampleRate, samplesPerBlock); }
void ExtasisVisionAudioProcessor::releaseResources() {}
bool ExtasisVisionAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return true;
  #endif
}

void ExtasisVisionAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ignoreUnused (midiMessages);
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
}

bool ExtasisVisionAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* ExtasisVisionAudioProcessor::createEditor() { return new ExtasisVisionAudioProcessorEditor (*this); }

void ExtasisVisionAudioProcessor::getStateInformation (juce::MemoryBlock& destData) { juce::ignoreUnused (destData); }
void ExtasisVisionAudioProcessor::setStateInformation (const void* data, int sizeInBytes) { juce::ignoreUnused (data, sizeInBytes); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new ExtasisVisionAudioProcessor(); }
