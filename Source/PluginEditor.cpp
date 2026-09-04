#include "PluginProcessor.h"
#include "PluginEditor.h"

#include "Theme.h"

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
    // Usar las reglas de diseño para el fondo
    g.fillAll (ExtasisDesign::bgBase);

    // Dibujar un panel central siguiendo el sistema de márgenes rígidos
    auto bounds = getLocalBounds().reduced (ExtasisDesign::marginLarge);
    
    g.setColour (ExtasisDesign::bgPanel);
    g.fillRoundedRectangle (bounds.toFloat(), ExtasisDesign::panelCornerRadius);

    g.setColour (ExtasisDesign::metalDark);
    g.drawRoundedRectangle (bounds.toFloat(), ExtasisDesign::panelCornerRadius, 2.0f);

    // Textos con tipografía y color del tema
    g.setColour (ExtasisDesign::hudRed);
    g.setFont (ExtasisDesign::getFontTitle());
    g.drawFittedText ("EXTASIS VISION", bounds.removeFromTop(80), juce::Justification::centred, 1);

    g.setColour (ExtasisDesign::metalChrome);
    g.setFont (ExtasisDesign::getFontBody());
    g.drawFittedText ("SYSTEM ONLINE. AWAITING IMAGE INPUT...", bounds, juce::Justification::centred, 1);
}

void ExtasisVisionAudioProcessorEditor::resized()
{
}
