#include "PluginProcessor.h"
#include "PluginEditor.h"

ExtasisVisionAudioProcessorEditor::ExtasisVisionAudioProcessorEditor (ExtasisVisionAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Define the initial window size
    setSize (800, 600);
}

ExtasisVisionAudioProcessorEditor::~ExtasisVisionAudioProcessorEditor()
{
}

void ExtasisVisionAudioProcessorEditor::paint (juce::Graphics& g)
{
    // T-1000 aesthetic background (Dark metallic gray)
    g.fillAll (juce::Colour::fromRGB(25, 25, 28));

    // Futuristic HUD text color (Red or Cyan)
    g.setColour (juce::Colour::fromRGB(255, 60, 60)); // Terminator Red
    g.setFont (30.0f);
    g.drawFittedText ("EXTASIS VISION", getLocalBounds().withSizeKeepingCentre(400, 50).translated(0, -200), juce::Justification::centred, 1);

    g.setColour (juce::Colours::grey);
    g.setFont (15.0f);
    g.drawFittedText ("SYSTEM ONLINE. AWAITING IMAGE INPUT...", getLocalBounds(), juce::Justification::centred, 1);
}

void ExtasisVisionAudioProcessorEditor::resized()
{
}
