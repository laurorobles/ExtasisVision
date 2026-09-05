#include "PluginProcessor.h"
#include "PluginEditor.h"

juce::AudioProcessorValueTreeState::ParameterLayout ExtasisVisionAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "SCAN_SPEED", "Velocidad de Escaneo 1", 0.01f, 1.0f, 0.1f));
        
    params.push_back (std::make_unique<juce::AudioParameterInt> (
        "BASE_OCTAVE", "Octava Base 1", -2, 2, 0));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "SCAN_SPEED_2", "Velocidad de Escaneo 2", 0.01f, 1.0f, 0.2f));
        
    params.push_back (std::make_unique<juce::AudioParameterInt> (
        "BASE_OCTAVE_2", "Octava Base 2", -2, 2, -1));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        "SYNC_MODE_1", "Modo Sincronia 1", 
        juce::StringArray{"Free (Hz)", "1 Bar", "1/2", "1/4", "1/8", "1/16"}, 0));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        "SYNC_MODE_2", "Modo Sincronia 2", 
        juce::StringArray{"Free (Hz)", "1 Bar", "1/2", "1/4", "1/8", "1/16"}, 0));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        "ENGINE_MODE", "Motor", juce::StringArray{"Escaner Analitico", "Sintetizador RGB"}, 0));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        "SCALE_MODE", "Escala", juce::StringArray{"Libre", "Cromatica", "Pentatonica Menor"}, 0));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "DELAY_MIX", "Echo", 0.0f, 1.0f, 0.3f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        "REVERB_MIX", "Espacio", 0.0f, 1.0f, 0.4f));

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
    isLicensedCached = LicenseManager::isLicensed();
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
        scanPositionX[0] = 0.0f;
        scanPositionX[1] = 0.0f;
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
    currentPhase[0] = 0.0f;
    currentPhase[1] = 0.0f;
    filterState[0] = 0.0f;
    filterState[1] = 0.0f;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate = sampleRate;
    spec.maximumBlockSize = samplesPerBlock;
    spec.numChannels = getTotalNumOutputChannels();
    if (spec.numChannels == 0) spec.numChannels = 2; // Seguro anticaídas

    reverb.prepare (spec);
    reverbParams.roomSize = 0.8f;
    reverbParams.damping = 0.5f;
    reverbParams.width = 1.0f;
    
    delayLine.prepare (spec);
    delayLine.setMaximumDelayInSamples (sampleRate * 2.0); // Hasta 2 segundos max
    delayLine.setDelay (sampleRate * 0.35f); // 350ms delay
    
    maxSamplesForDemo = (uint64_t)(10.0 * 60.0 * sampleRate); // 10 minutos
    samplesProcessed = 0;
    isExpired = false;
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
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());
        
    if (!isLicensedCached.load())
    {
        samplesProcessed += buffer.getNumSamples();
        if (samplesProcessed >= maxSamplesForDemo)
        {
            isExpired.store (true);
            buffer.clear();
            return;
        }
    }

    if (!hasImage)
    {
        buffer.clear();
        return;
    }

    const juce::ScopedLock sl (imageLock);
    
    int width = currentImage.getWidth();
    int height = currentImage.getHeight();
    float sampleRate = (float) getSampleRate();
    if (sampleRate <= 0) sampleRate = 44100.0f;

    // Leer Parámetros
    int engineMode = static_cast<int> (apvts.getRawParameterValue ("ENGINE_MODE")->load());
    int scaleMode = static_cast<int> (apvts.getRawParameterValue ("SCALE_MODE")->load());
    float delayMix = apvts.getRawParameterValue ("DELAY_MIX")->load();
    float reverbMix = apvts.getRawParameterValue ("REVERB_MIX")->load();
    
    float scanSpeeds[2] = {
        apvts.getRawParameterValue ("SCAN_SPEED")->load(),
        apvts.getRawParameterValue ("SCAN_SPEED_2")->load()
    };
    int octaveShifts[2] = {
        static_cast<int> (apvts.getRawParameterValue ("BASE_OCTAVE")->load()),
        static_cast<int> (apvts.getRawParameterValue ("BASE_OCTAVE_2")->load())
    };
    int syncModes[2] = {
        static_cast<int> (apvts.getRawParameterValue ("SYNC_MODE_1")->load()),
        static_cast<int> (apvts.getRawParameterValue ("SYNC_MODE_2")->load())
    };

    // --- Obtener datos del DAW (BPM, PPQ) ---
    double bpm = 120.0;
    double ppqPosition = 0.0;
    bool isPlaying = false;
    
    if (auto* playHead = getPlayHead())
    {
        if (auto positionInfo = playHead->getPosition())
        {
            if (positionInfo->getBpm().hasValue())
                bpm = *positionInfo->getBpm();
            if (positionInfo->getPpqPosition().hasValue())
                ppqPosition = *positionInfo->getPpqPosition();
            isPlaying = positionInfo->getIsPlaying();
        }
    }

    double beatDivisions[6] = {0.0, 4.0, 2.0, 1.0, 0.5, 0.25};

    // Función Lambda para Cuantizar Frecuencia a Escala
    auto quantizeFrequency = [](float freq, int mode, int octave) -> float {
        float midiNote = 69.0f + 12.0f * std::log2 (freq / 440.0f);
        int note = (int) std::round (midiNote);
        if (mode == 2) // Pentatónica Menor
        {
            int octaveBase = (note / 12) * 12;
            int pitchClass = note % 12;
            int pentatonicNotes[] = {0, 0, 3, 3, 3, 5, 5, 7, 7, 7, 10, 10};
            note = octaveBase + pentatonicNotes[pitchClass];
        }
        note += (octave * 12);
        return 440.0f * std::pow (2.0f, (note - 69.0f) / 12.0f);
    };

    float phaseDeltas[2] = {0.0f, 0.0f};
    float targetAmplitudes[2] = {0.0f, 0.0f};
    float filterCoeffs[2] = {1.0f, 1.0f};

    float minFreq = 50.0f;
    float maxFreq = 2000.0f;

    for (int v = 0; v < 2; ++v)
    {
        if (syncModes[v] > 0 && isPlaying) 
        {
            double beatsPerCycle = beatDivisions[syncModes[v]];
            double phaseInCycle = std::fmod (ppqPosition, beatsPerCycle) / beatsPerCycle;
            scanPositionX[v] = (float) phaseInCycle;
        }

        int xPos = juce::jlimit (0, width - 1, (int)(scanPositionX[v] * width));
        float currentFreq = minFreq;

        if (engineMode == 0) // Escáner Analítico
        {
            float maxBrightness = 0.0f;
            int brightestY = height / 2;
            for (int y = 0; y < height; ++y)
            {
                float brightness = currentImage.getPixelAt (xPos, y).getBrightness();
                if (brightness > maxBrightness) { maxBrightness = brightness; brightestY = y; }
            }
            float normalizedY = 1.0f - ((float)brightestY / (float)height); 
            float rawFreq = minFreq + (maxFreq - minFreq) * normalizedY;
            currentFreq = (scaleMode == 0) ? (rawFreq * std::pow(2.0f, (float)octaveShifts[v])) : quantizeFrequency(rawFreq, scaleMode, octaveShifts[v]);
            targetAmplitudes[v] = (maxBrightness > 0.05f) ? (maxBrightness * 0.3f) : 0.0f;
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

            float rawFreq = minFreq + (maxFreq - minFreq) * avgG; 
            currentFreq = (scaleMode == 0) ? (rawFreq * std::pow(2.0f, (float)octaveShifts[v])) : quantizeFrequency(rawFreq, scaleMode, octaveShifts[v]);
            targetAmplitudes[v] = avgB * 0.4f; 
            filterCoeffs[v] = juce::jlimit(0.01f, 1.0f, avgR * 1.5f);
        }

        phaseDeltas[v] = (currentFreq * juce::MathConstants<float>::twoPi) / sampleRate;
    }

    int numSamples = buffer.getNumSamples();
    
    for (int i = 0; i < numSamples; ++i)
    {
        float synthOutputSum = 0.0f;
        for (int v = 0; v < 2; ++v)
        {
            float sampleValue = std::sin (currentPhase[v]) * targetAmplitudes[v];
            filterState[v] += filterCoeffs[v] * (sampleValue - filterState[v]);
            float voiceOutput = (engineMode == 1) ? filterState[v] : sampleValue;
            synthOutputSum += voiceOutput;

            currentPhase[v] += phaseDeltas[v];
            if (currentPhase[v] >= juce::MathConstants<float>::twoPi) currentPhase[v] -= juce::MathConstants<float>::twoPi;
                
            if (syncModes[v] == 0 || !isPlaying)
            {
                scanPositionX[v] += (scanSpeeds[v] / sampleRate);
                if (scanPositionX[v] > 1.0f) scanPositionX[v] -= 1.0f;
            }
        }

        // Prevención de Clipping Suave (Soft Clipping)
        synthOutputSum = std::tanh (synthOutputSum);

        // Procesar Delay
        float delayedSample = delayLine.popSample (0); 
        float delayFeedback = 0.4f;
        delayLine.pushSample (0, synthOutputSum + delayedSample * delayFeedback); 
        
        float finalSample = synthOutputSum + delayedSample * delayMix;

        for (int channel = 0; channel < totalNumOutputChannels; ++channel)
            buffer.addSample (channel, i, finalSample);
    }

    // Procesar Reverb al final del bloque
    reverbParams.wetLevel = reverbMix;
    reverbParams.dryLevel = 1.0f - (reverbMix * 0.5f);
    reverb.setParameters (reverbParams);

    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> context (block);
    reverb.process (context);
}

bool ExtasisVisionAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* ExtasisVisionAudioProcessor::createEditor() { return new ExtasisVisionAudioProcessorEditor (*this); }

void ExtasisVisionAudioProcessor::getStateInformation (juce::MemoryBlock& destData) { juce::ignoreUnused (destData); }
void ExtasisVisionAudioProcessor::setStateInformation (const void* data, int sizeInBytes) { juce::ignoreUnused (data, sizeInBytes); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new ExtasisVisionAudioProcessor(); }
