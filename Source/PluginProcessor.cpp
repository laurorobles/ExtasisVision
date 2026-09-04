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

    // Calcular parámetros una vez por bloque para ahorrar CPU
    int xPos = juce::jlimit (0, width - 1, (int)(scanPositionX * width));
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
    
    // Mapeo Y -> Frecuencia (Abajo es grave, Arriba es agudo)
    float normalizedY = 1.0f - ((float)brightestY / (float)height); 
    float minFreq = 50.0f;
    float maxFreq = 2000.0f;
    float currentFreq = minFreq + (maxFreq - minFreq) * normalizedY;
    float phaseDelta = (currentFreq * juce::MathConstants<float>::twoPi) / sampleRate;

    // Gate: Si el píxel es muy oscuro, hacer silencio
    float targetAmplitude = (maxBrightness > 0.05f) ? (maxBrightness * 0.3f) : 0.0f;

    int numSamples = buffer.getNumSamples();
    int totalNumOutputChannels = getTotalNumOutputChannels();
    
    for (int i = 0; i < numSamples; ++i)
    {
        // Oscilador Senoidal
        float sampleValue = std::sin (currentPhase) * targetAmplitude;
        
        for (int channel = 0; channel < totalNumOutputChannels; ++channel)
            buffer.addSample (channel, i, sampleValue);
            
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
