#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class ExtasisVisionAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    ExtasisVisionAudioProcessorEditor (ExtasisVisionAudioProcessor&);
    ~ExtasisVisionAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    ExtasisVisionAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExtasisVisionAudioProcessorEditor)
};
