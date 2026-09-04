#pragma once
#include <JuceHeader.h>

class ExtasisVisionAudioProcessor : public juce::AudioProcessor
{
public:
    ExtasisVisionAudioProcessor();
    ~ExtasisVisionAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;
    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    // --- ExtasisVision Engine Variables ---
    void loadImage (const juce::File& file);
    
    juce::Image currentImage;
    bool hasImage = false;
    
    float currentPhase = 0.0f;
    float scanPositionX = 0.0f; 
    float filterState = 0.0f; // Para el Sintetizador RGB
    
    juce::CriticalSection imageLock; // Proteger acceso a la imagen entre hilos

    // --- Audio Processor Value Tree State ---
    juce::AudioProcessorValueTreeState apvts;
    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (ExtasisVisionAudioProcessor)
};
