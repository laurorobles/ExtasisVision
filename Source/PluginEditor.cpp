#include "PluginProcessor.h"
#include "PluginEditor.h"

#include "Theme.h"

ExtasisVisionAudioProcessorEditor::ExtasisVisionAudioProcessorEditor (ExtasisVisionAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    // Sliders config
    scanSpeedSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    scanSpeedSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    scanSpeedSlider.setTooltip ("VELOCIDAD DEL ESCANER 1 (HZ)");
    scanSpeedSlider.setLookAndFeel (&customLookAndFeel);
    addAndMakeVisible (scanSpeedSlider);

    baseOctaveSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    baseOctaveSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    baseOctaveSlider.setTooltip ("OCTAVA BASE 1");
    baseOctaveSlider.setLookAndFeel (&customLookAndFeel);
    addAndMakeVisible (baseOctaveSlider);

    scanSpeed2Slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    scanSpeed2Slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    scanSpeed2Slider.setTooltip ("VELOCIDAD DEL ESCANER 2 (HZ)");
    scanSpeed2Slider.setLookAndFeel (&customLookAndFeel);
    addAndMakeVisible (scanSpeed2Slider);

    baseOctave2Slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    baseOctave2Slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    baseOctave2Slider.setTooltip ("OCTAVA BASE 2");
    baseOctave2Slider.setLookAndFeel (&customLookAndFeel);
    addAndMakeVisible (baseOctave2Slider);

    delayMixSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    delayMixSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    delayMixSlider.setTooltip ("CANTIDAD DE ECO (DELAY)");
    delayMixSlider.setLookAndFeel (&customLookAndFeel);
    addAndMakeVisible (delayMixSlider);

    reverbMixSlider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
    reverbMixSlider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
    reverbMixSlider.setTooltip ("ATMÓSFERA (REVERB)");
    reverbMixSlider.setLookAndFeel (&customLookAndFeel);
    addAndMakeVisible (reverbMixSlider);

    // Attachments
    scanSpeedAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "SCAN_SPEED", scanSpeedSlider);
    baseOctaveAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "BASE_OCTAVE", baseOctaveSlider);
    scanSpeed2Attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "SCAN_SPEED_2", scanSpeed2Slider);
    baseOctave2Attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "BASE_OCTAVE_2", baseOctave2Slider);
    delayMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "DELAY_MIX", delayMixSlider);
    reverbMixAttachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.apvts, "REVERB_MIX", reverbMixSlider);

    // ComboBoxes
    engineModeComboBox.addItemList (juce::StringArray{"Escaner Analitico", "Sintetizador RGB"}, 1);
    addAndMakeVisible (engineModeComboBox);
    engineModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, "ENGINE_MODE", engineModeComboBox);

    scaleModeComboBox.addItemList (juce::StringArray{"Libre", "Cromatica", "Pentatonica Menor"}, 1);
    addAndMakeVisible (scaleModeComboBox);
    scaleModeAttachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, "SCALE_MODE", scaleModeComboBox);

    syncMode1ComboBox.addItemList (juce::StringArray{"Free Hz", "1 Bar", "1/2", "1/4", "1/8", "1/16"}, 1);
    addAndMakeVisible (syncMode1ComboBox);
    syncMode1Attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, "SYNC_MODE_1", syncMode1ComboBox);

    syncMode2ComboBox.addItemList (juce::StringArray{"Free Hz", "1 Bar", "1/2", "1/4", "1/8", "1/16"}, 1);
    addAndMakeVisible (syncMode2ComboBox);
    syncMode2Attachment = std::make_unique<juce::AudioProcessorValueTreeState::ComboBoxAttachment>(
        audioProcessor.apvts, "SYNC_MODE_2", syncMode2ComboBox);

    // License UI Setup
    addChildComponent (licenseInput);
    licenseInput.setMultiLine (false);
    licenseInput.setJustification (juce::Justification::centred);
    licenseInput.setTextToShowWhenEmpty ("EXTV-XXXX-XXXX-XXXX-XXXX", juce::Colours::grey);

    addChildComponent (activateButton);
    activateButton.setButtonText ("ACTIVATE LICENSE");
    activateButton.onClick = [this] { checkLicense(); };

    addChildComponent (gumroadLinkBtn);

    if (!audioProcessor.isLicensedCached.load())
    {
        licenseInput.setVisible (true);
        activateButton.setVisible (true);
        gumroadLinkBtn.setVisible (true);
    }

    setSize (800, 600);
    startTimerHz (60); // 60 FPS repainting for the scanner
}

void ExtasisVisionAudioProcessorEditor::checkLicense()
{
    if (LicenseManager::saveLicense (licenseInput.getText()))
    {
        audioProcessor.isLicensedCached.store (true);
        licenseInput.setVisible (false);
        activateButton.setVisible (false);
        gumroadLinkBtn.setVisible (false);
        repaint();
    }
    else
    {
        licenseInput.setText ("");
        licenseInput.setTextToShowWhenEmpty ("INVALID KEY", juce::Colours::red);
    }
}

ExtasisVisionAudioProcessorEditor::~ExtasisVisionAudioProcessorEditor()
{
    scanSpeedSlider.setLookAndFeel (nullptr);
    baseOctaveSlider.setLookAndFeel (nullptr);
    scanSpeed2Slider.setLookAndFeel (nullptr);
    baseOctave2Slider.setLookAndFeel (nullptr);
    delayMixSlider.setLookAndFeel (nullptr);
    reverbMixSlider.setLookAndFeel (nullptr);
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
    
    // Textos de los 3 Módulos en el Control Deck
    int moduleWidth = footerBounds.getWidth() / 3;
    
    auto mod1 = footerBounds.removeFromLeft(moduleWidth);
    auto mod2 = footerBounds.removeFromLeft(moduleWidth);
    auto mod3 = footerBounds;

    // Subdividir módulos
    auto m1Speed = mod1.removeFromLeft(mod1.getWidth() / 2).removeFromBottom(25);
    auto m1Oct = mod1.removeFromBottom(25);
    auto m2Speed = mod2.removeFromLeft(mod2.getWidth() / 2).removeFromBottom(25);
    auto m2Oct = mod2.removeFromBottom(25);
    auto m3Del = mod3.removeFromLeft(mod3.getWidth() / 2).removeFromBottom(25);
    auto m3Rev = mod3.removeFromBottom(25);

    g.setFont (12.0f);
    g.drawFittedText ("SPEED 1", m1Speed, juce::Justification::centred, 1);
    g.drawFittedText ("OCTAVE 1", m1Oct, juce::Justification::centred, 1);
    g.drawFittedText ("SPEED 2", m2Speed, juce::Justification::centred, 1);
    g.drawFittedText ("OCTAVE 2", m2Oct, juce::Justification::centred, 1);
    g.drawFittedText ("ECHO", m3Del, juce::Justification::centred, 1);
    g.drawFittedText ("SPACE", m3Rev, juce::Justification::centred, 1);

    // Panel Principal (Imagen)
    bounds.reduce (0, ExtasisDesign::marginMedium);
    g.setColour (ExtasisDesign::bgPanel);
    g.fillRoundedRectangle (bounds.toFloat(), 5.0f);

    if (audioProcessor.hasImage)
    {
        g.drawImage (audioProcessor.currentImage, bounds.toFloat(),
                     juce::RectanglePlacement::stretchToFit);

        // Escáner 1 (Rojo)
        g.setColour (ExtasisDesign::hudRed);
        float scanX1 = bounds.getX() + (audioProcessor.scanPositionX[0] * bounds.getWidth());
        g.drawLine (scanX1, bounds.getY(), scanX1, bounds.getBottom(), 2.0f);
        
        // Escáner 2 (Cian)
        g.setColour (juce::Colours::cyan);
        float scanX2 = bounds.getX() + (audioProcessor.scanPositionX[1] * bounds.getWidth());
        g.drawLine (scanX2, bounds.getY(), scanX2, bounds.getBottom(), 2.0f);
    }
    else
    {
        g.setColour (ExtasisDesign::metalChrome);
        g.setFont (ExtasisDesign::getFontBody());
        g.drawFittedText ("SYSTEM ONLINE. AWAITING IMAGE INPUT... (DRAG & DROP)", bounds, juce::Justification::centred, 1);
    }

    g.setColour (ExtasisDesign::metalDark);
    g.drawRoundedRectangle (bounds.toFloat(), ExtasisDesign::panelCornerRadius, 2.0f);

    if (!audioProcessor.isLicensedCached.load())
    {
        g.setColour (juce::Colours::black.withAlpha(0.8f));
        g.fillRect (getLocalBounds());
        
        g.setColour (ExtasisDesign::hudRed);
        g.setFont (24.0f);
        g.drawText ("LICENSE REQUIRED", getLocalBounds().withSizeKeepingCentre(400, 200).withY(getLocalBounds().getCentreY() - 80), juce::Justification::centred);
    }

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
    engineModeComboBox.setBounds (combosBounds.removeFromLeft(120).reduced(2));
    scaleModeComboBox.setBounds (combosBounds.removeFromLeft(120).reduced(2));
    syncMode1ComboBox.setBounds (combosBounds.removeFromLeft(90).reduced(2));
    syncMode2ComboBox.setBounds (combosBounds.removeFromLeft(90).reduced(2));

    auto footerBounds = bounds.removeFromBottom(120);
    
    int moduleWidth = footerBounds.getWidth() / 3;
    auto mod1 = footerBounds.removeFromLeft(moduleWidth);
    auto mod2 = footerBounds.removeFromLeft(moduleWidth);
    auto mod3 = footerBounds;

    // Dejar espacio para las etiquetas en el fondo (25px)
    mod1.removeFromBottom(25);
    mod2.removeFromBottom(25);
    mod3.removeFromBottom(25);

    auto kSpeed1 = mod1.removeFromLeft(mod1.getWidth() / 2);
    auto kOct1 = mod1;
    auto kSpeed2 = mod2.removeFromLeft(mod2.getWidth() / 2);
    auto kOct2 = mod2;
    auto kDel = mod3.removeFromLeft(mod3.getWidth() / 2);
    auto kRev = mod3;

    scanSpeedSlider.setBounds (kSpeed1.reduced(10));
    baseOctaveSlider.setBounds (kOct1.reduced(10));
    scanSpeed2Slider.setBounds (kSpeed2.reduced(10));
    baseOctave2Slider.setBounds (kOct2.reduced(10));
    delayMixSlider.setBounds (kDel.reduced(10));
    reverbMixSlider.setBounds (kRev.reduced(10));

    if (!audioProcessor.isLicensedCached.load())
    {
        auto modalArea = getLocalBounds().withSizeKeepingCentre(400, 200);
        licenseInput.setBounds (modalArea.removeFromTop(40).reduced(0, 5));
        activateButton.setBounds (modalArea.removeFromTop(40).reduced(20, 5));
        gumroadLinkBtn.setBounds (modalArea.removeFromTop(40).reduced(20, 5));
    }
}
