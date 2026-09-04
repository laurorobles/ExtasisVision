#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout ExtasisVisionAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "SCAN_SPEED", "Velocidad de Escaneo", 0.01f, 1.0f, 0.1f));
        
    params.push_back (std::make_unique<juce::AudioParameterInt> (
        "BASE_OCTAVE", "Octava Base", -2, 2, 0));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        "ENGINE_MODE", "Motor", juce::StringArray{"Escaner Analitico", "Sintetizador RGB"}, 0));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        "SCALE_MODE", "Escala", juce::StringArray{"Libre", "Cromatica", "Pentatonica Menor"}, 0));

    return { params.begin(), params.end() };
}

ExtasisVisionAudioProcessor::ExtasisVisionAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
       apvts (*this, nullptr, "Parameters", createParameterLayout())
#endif
{
}

ExtasisVisionAudioProcessor::~ExtasisVisionAudioProcessor() {}

void ExtasisVisionAudioProcessor::loadImage (const juce::File& file)
{
    auto newImage = juce::ImageFileFormat::loadFrom (file);
    if (newImage.isValid())
    {
        const juce::ScopedLock sl (imageLock);
        currentImage = newImage;
        hasImage = true;
        scanPositionX = 0.0f; // Resetear escáner
    }
}

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

void ExtasisVisionAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock) 
{ 
    juce::ignoreUnused (sampleRate, samplesPerBlock); 
    currentPhase = 0.0f;
    filterState = 0.0f;
}
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
    buffer.clear();

    const juce::ScopedLock sl (imageLock);
    
    if (! hasImage || currentImage.isNull())
        return;

    int width = currentImage.getWidth();
    int height = currentImage.getHeight();
    float sampleRate = (float) getSampleRate();
    if (sampleRate <= 0) sampleRate = 44100.0f;

    // Leer Parámetros
    int engineMode = static_cast<int> (apvts.getRawParameterValue ("ENGINE_MODE")->load());
    int scaleMode = static_cast<int> (apvts.getRawParameterValue ("SCALE_MODE")->load());
    int octaveShift = static_cast<int> (apvts.getRawParameterValue ("BASE_OCTAVE")->load());
    float scanSpeed = apvts.getRawParameterValue ("SCAN_SPEED")->load();

    int xPos = juce::jlimit (0, width - 1, (int)(scanPositionX * width));
    
    float minFreq = 50.0f;
    float maxFreq = 2000.0f;
    float currentFreq = minFreq;
    float targetAmplitude = 0.0f;
    float filterCoeff = 1.0f; // 1.0 = Abierto

    // Función Lambda para Cuantizar Frecuencia a Escala
    auto quantizeFrequency = [](float freq, int mode, int octave) -> float {
        float midiNote = 69.0f + 12.0f * std::log2 (freq / 440.0f);
        int note = (int) std::round (midiNote);
        if (mode == 2) // Pentatónica Menor (0, 3, 5, 7, 10)
        {
            int octaveBase = (note / 12) * 12;
            int pitchClass = note % 12;
            int pentatonicNotes[] = {0, 0, 3, 3, 3, 5, 5, 7, 7, 7, 10, 10};
            note = octaveBase + pentatonicNotes[pitchClass];
        }
        note += (octave * 12);
        return 440.0f * std::pow (2.0f, (note - 69.0f) / 12.0f);
    };

    if (engineMode == 0) // Escáner Analítico
    {
        float maxBrightness = 0.0f;
        int brightestY = height / 2;
        
        for (int y = 0; y < height; ++y)
        {
            float brightness = currentImage.getPixelAt (xPos, y).getBrightness();
            if (brightness > maxBrightness)
            {
                maxBrightness = brightness;
                brightestY = y;
            }
        }
        
        float normalizedY = 1.0f - ((float)brightestY / (float)height); 
        float rawFreq = minFreq + (maxFreq - minFreq) * normalizedY;
        
        currentFreq = (scaleMode == 0) ? (rawFreq * std::pow(2.0f, (float)octaveShift)) : quantizeFrequency(rawFreq, scaleMode, octaveShift);
        targetAmplitude = (maxBrightness > 0.05f) ? (maxBrightness * 0.3f) : 0.0f;
    }
    else // Sintetizador RGB
    {
        float sumR = 0.0f, sumG = 0.0f, sumB = 0.0f;
        for (int y = 0; y < height; ++y)
        {
            auto color = currentImage.getPixelAt (xPos, y);
            sumR += color.getFloatRed();
            sumG += color.getFloatGreen();
            sumB += color.getFloatBlue();
        }
        float avgR = sumR / height;
        float avgG = sumG / height;
        float avgB = sumB / height;

        float rawFreq = minFreq + (maxFreq - minFreq) * avgG; // Verde = Pitch
        currentFreq = (scaleMode == 0) ? (rawFreq * std::pow(2.0f, (float)octaveShift)) : quantizeFrequency(rawFreq, scaleMode, octaveShift);
        
        targetAmplitude = avgB * 0.4f; // Azul = Volumen
        filterCoeff = juce::jlimit(0.01f, 1.0f, avgR * 1.5f); // Rojo = Filtro Lowpass
    }

    float phaseDelta = (currentFreq * juce::MathConstants<float>::twoPi) / sampleRate;

    int numSamples = buffer.getNumSamples();
    int totalNumOutputChannels = getTotalNumOutputChannels();
    
    for (int i = 0; i < numSamples; ++i)
    {
        float sampleValue = std::sin (currentPhase) * targetAmplitude;
        
        // Aplicar Filtro One-Pole si estamos en Modo RGB
        filterState += filterCoeff * (sampleValue - filterState);
        float finalSample = (engineMode == 1) ? filterState : sampleValue;

        for (int channel = 0; channel < totalNumOutputChannels; ++channel)
            buffer.addSample (channel, i, finalSample);
            
        currentPhase += phaseDelta;
        if (currentPhase >= juce::MathConstants<float>::twoPi)
            currentPhase -= juce::MathConstants<float>::twoPi;
            
        scanPositionX += (scanSpeed / sampleRate);
        if (scanPositionX > 1.0f)
            scanPositionX -= 1.0f; // Ciclar la imagen
    }
}

bool ExtasisVisionAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* ExtasisVisionAudioProcessor::createEditor() { return new ExtasisVisionAudioProcessorEditor (*this); }

void ExtasisVisionAudioProcessor::getStateInformation (juce::MemoryBlock& destData) { juce::ignoreUnused (destData); }
void ExtasisVisionAudioProcessor::setStateInformation (const void* data, int sizeInBytes) { juce::ignoreUnused (data, sizeInBytes); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new ExtasisVisionAudioProcessor(); }
