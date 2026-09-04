#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

class ExtasisVisionAudioProcessorEditor : public juce::AudioProcessorEditor,
                                          public juce::FileDragAndDropTarget,
                                          public juce::Timer
{
public:
    ExtasisVisionAudioProcessorEditor (ExtasisVisionAudioProcessor&);
    ~ExtasisVisionAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    bool isInterestedInFileDrag (const juce::StringArray& files) override;
    void filesDropped (const juce::StringArray& files, int x, int y) override;
    void timerCallback() override;

private:
    ExtasisVisionAudioProcessor& audioProcessor;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExtasisVisionAudioProcessorEditor)
};
