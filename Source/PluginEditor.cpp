#include "PluginProcessor.h"
#include "PluginEditor.h"

#include "Theme.h"

ExtasisVisionAudioProcessorEditor::ExtasisVisionAudioProcessorEditor (ExtasisVisionAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Sliders config
    scanSpeedSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    scanSpeedSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    scanSpeedSlider.setTooltip ("VELOCIDAD DEL ESCANER ÓPTICO (HZ)");
    scanSpeedSlider.setLookAndFeel (&customLookAndFeel);
    addAndMakeVisible (scanSpeedSlider);

    baseOctaveSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    baseOctaveSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    baseOctaveSlider.setTooltip ("TRANSPOSICIÓN BASE (OCTAVAS)");
    baseOctaveSlider.setLookAndFeel (&customLookAndFeel);
    addAndMakeVisible (baseOctaveSlider);

    // Attachments
    scanSpeedAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "SCAN_SPEED", scanSpeedSlider);
    baseOctaveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "BASE_OCTAVE", baseOctaveSlider);

    // ComboBoxes
    engineModeComboBox.addItemList (juce::StringArray{"Escaner Analitico", "Sintetizador RGB"}, 1);
    addAndMakeVisible (engineModeComboBox);
    engineModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, "ENGINE_MODE", engineModeComboBox);

    scaleModeComboBox.addItemList (juce::StringArray{"Libre", "Cromatica", "Pentatonica Menor"}, 1);
    addAndMakeVisible (scaleModeComboBox);
    scaleModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, "SCALE_MODE", scaleModeComboBox);

    setSize (800, 600);
    startTimerHz (60); // 60 FPS repainting for the scanner
}

ExtasisVisionAudioProcessorEditor::~ExtasisVisionAudioProcessorEditor()
{
    scanSpeedSlider.setLookAndFeel (nullptr);
    baseOctaveSlider.setLookAndFeel (nullptr);
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
    g.fillAll (ExtasisDesign::bgBase);
    auto bounds = getLocalBounds().reduced (ExtasisDesign::marginLarge);
    
    // Header (Título)
    auto headerBounds = bounds.removeFromTop(60);
    g.setColour (ExtasisDesign::hudRed);
    g.setFont (ExtasisDesign::getFontTitle());
    g.drawFittedText ("EXTASIS VISION", headerBounds.withWidth(250), juce::Justification::centredLeft, 1);

    // Controles (Footer)
    auto footerBounds = bounds.removeFromBottom(120);
    g.setColour (ExtasisDesign::metalChrome);
    g.setFont (ExtasisDesign::getFontBody());
    
    // Textos debajo de los knobs (dividimos el footer en dos)
    auto speedLabelArea = footerBounds.removeFromLeft(footerBounds.getWidth() / 2).removeFromBottom(30);
    auto octaveLabelArea = footerBounds.removeFromBottom(30);
    g.drawFittedText ("SCAN SPEED", speedLabelArea, juce::Justification::centred, 1);
    g.drawFittedText ("BASE OCTAVE", octaveLabelArea, juce::Justification::centred, 1);

    // Panel Principal (Imagen)
    bounds.reduce (0, ExtasisDesign::marginMedium);
    g.setColour (ExtasisDesign::bgPanel);
    g.fillRoundedRectangle (bounds.toFloat(), ExtasisDesign::panelCornerRadius);

    if (audioProcessor.hasImage)
    {
        const juce::ScopedLock sl (audioProcessor.imageLock);
        g.drawImage (audioProcessor.currentImage, bounds.toFloat(), juce::RectanglePlacement::centred);
        
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

    // Créditos del desarrollador
    g.setColour (ExtasisDesign::metalDark.brighter(0.5f));
    g.setFont (ExtasisDesign::getFontBody().withHeight(11.0f));
    g.drawFittedText ("coded by @laurorobles", 
                      getLocalBounds().reduced(ExtasisDesign::marginSmall), 
                      juce::Justification::bottomRight, 1);
}

void ExtasisVisionAudioProcessorEditor::resized()
{
    auto bounds = getLocalBounds().reduced (ExtasisDesign::marginLarge);
    auto headerBounds = bounds.removeFromTop(60); 
    
    // Header comboboxes
    headerBounds.removeFromLeft(200); // Espacio para el título "EXTASIS VISION"
    auto combosBounds = headerBounds.reduced(0, 15);
    engineModeComboBox.setBounds (combosBounds.removeFromLeft(150).reduced(5));
    scaleModeComboBox.setBounds (combosBounds.removeFromLeft(150).reduced(5));

    auto footerBounds = bounds.removeFromBottom(120);
    
    auto leftKnob = footerBounds.removeFromLeft(footerBounds.getWidth() / 2);
    auto rightKnob = footerBounds;
    
    // Dejar espacio para las etiquetas de abajo
    leftKnob.removeFromBottom(30);
    rightKnob.removeFromBottom(30);

    scanSpeedSlider.setBounds (leftKnob.reduced(10));
    baseOctaveSlider.setBounds (rightKnob.reduced(10));
}
