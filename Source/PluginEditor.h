#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "Theme.h"

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

    juce::Slider scanSpeedSlider;
    juce::Slider baseOctaveSlider;
    juce::Slider scanSpeed2Slider;
    juce::Slider baseOctave2Slider;
    juce::Slider delayMixSlider;
    juce::Slider reverbMixSlider;
    
    juce::ComboBox engineModeComboBox;
    juce::ComboBox scaleModeComboBox;

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> scanSpeedAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> baseOctaveAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> scanSpeed2Attachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> baseOctave2Attachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> delayMixAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> reverbMixAttachment;
    
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> engineModeAttachment;
    std::unique_ptr<juce::AudioProcessorValueTreeState::ComboBoxAttachment> scaleModeAttachment;
    
    ExtasisDesign::ExtasisLookAndFeel customLookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExtasisVisionAudioProcessorEditor)
};
