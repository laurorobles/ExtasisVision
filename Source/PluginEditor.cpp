#include "PluginProcessor.h"
#include "PluginEditor.h"

#include "Theme.h"

ExtasisVisionAudioProcessorEditor::ExtasisVisionAudioProcessorEditor (ExtasisVisionAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Define the initial window size
    setSize (800, 600);
    startTimerHz (60); // 60 FPS repainting for the scanner
}

ExtasisVisionAudioProcessorEditor::~ExtasisVisionAudioProcessorEditor()
{
}

bool ExtasisVisionAudioProcessorEditor::isInterestedInFileDrag (const juce::StringArray& files)
{
    for (auto file : files)
    {
        if (file.endsWithIgnoreCase (".jpg") || file.endsWithIgnoreCase (".jpeg") || file.endsWithIgnoreCase (".png"))
            return true;
    }
    return false;
}

void ExtasisVisionAudioProcessorEditor::filesDropped (const juce::StringArray& files, int, int)
{
    for (auto file : files)
    {
        if (file.endsWithIgnoreCase (".jpg") || file.endsWithIgnoreCase (".jpeg") || file.endsWithIgnoreCase (".png"))
        {
            audioProcessor.loadImage (juce::File (file));
            break; // Solo cargar la primera imagen compatible
        }
    }
}

void ExtasisVisionAudioProcessorEditor::timerCallback()
{
    if (audioProcessor.hasImage)
        repaint();
}

void ExtasisVisionAudioProcessorEditor::paint (juce::Graphics& g)
{
    // Usar las reglas de diseño para el fondo
    g.fillAll (ExtasisDesign::bgBase);

    // Dibujar un panel central siguiendo el sistema de márgenes rígidos
    auto bounds = getLocalBounds().reduced (ExtasisDesign::marginLarge);
    
    g.setColour (ExtasisDesign::bgPanel);
    g.fillRoundedRectangle (bounds.toFloat(), ExtasisDesign::panelCornerRadius);

    if (audioProcessor.hasImage)
    {
        const juce::ScopedLock sl (audioProcessor.imageLock);
        
        // Dibujar la imagen dentro del panel
        g.drawImage (audioProcessor.currentImage, bounds.toFloat(), juce::RectanglePlacement::centred);
        
        // Dibujar línea de escaneo
        float scanX = bounds.getX() + (audioProcessor.scanPositionX * bounds.getWidth());
        g.setColour (ExtasisDesign::hudRed.withAlpha(0.8f));
        g.drawLine (scanX, bounds.getY(), scanX, bounds.getBottom(), 2.0f);
    }
    else
    {
        g.setColour (ExtasisDesign::metalChrome);
        g.setFont (ExtasisDesign::getFontBody());
        g.drawFittedText ("SYSTEM ONLINE. AWAITING IMAGE INPUT... (DRAG & DROP)", bounds, juce::Justification::centred, 1);
    }

    g.setColour (ExtasisDesign::metalDark);
    g.drawRoundedRectangle (bounds.toFloat(), ExtasisDesign::panelCornerRadius, 2.0f);

    // Textos con tipografía y color del tema
    g.setColour (ExtasisDesign::hudRed);
    g.setFont (ExtasisDesign::getFontTitle());
    g.drawFittedText ("EXTASIS VISION", bounds.removeFromTop(80), juce::Justification::centred, 1);

    // Créditos del desarrollador
    g.setColour (ExtasisDesign::metalDark.brighter(0.5f));
    g.setFont (ExtasisDesign::getFontBody().withHeight(11.0f));
    g.drawFittedText ("coded by @laurorobles", 
                      getLocalBounds().reduced(ExtasisDesign::marginSmall), 
                      juce::Justification::bottomRight, 1);
}

void ExtasisVisionAudioProcessorEditor::resized()
{
}
