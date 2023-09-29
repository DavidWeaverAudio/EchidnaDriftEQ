/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/

struct EQBandParameters
{
    juce::String gainCurrent;
    juce::String gainDirection;
    juce::String gainMin;
    juce::String gainMax;
    juce::String gainSpeed;
    juce::String freqCurrent;
    juce::String freqDirection;
    juce::String freqMin;
    juce::String freqMax;
    juce::String freqSpeed;
    juce::String Q;
    juce::String type;
};
struct EQBand
{
    juce::dsp::IIR::Filter<float> filter;
    bool needsUpdate = true;

    float gainCurrent = 1.0f;
    float gainSpeed = 0.0f;
    float gainMin = 0.0f;
    float gainMax = 0.0f;
    float gainDirection = 1.0f;
    float freqCurrent = 1000.0f;
    float freqSpeed = 0.0f;
    float freqMin = 0.0f;
    float freqMax = 0.0f;
    float freqDirection = 1.0f;
    float Q = 1.0f;
    int type = 0; 

    void updateCoefficients(double sampleRate)
    {
        switch (type)
        {
        case 0: 
            filter.coefficients = juce::dsp::IIR::Coefficients<float>::makePeakFilter(sampleRate, freqCurrent, Q, gainCurrent);
            break;
        case 1:
            filter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowShelf(sampleRate, freqCurrent, Q, gainCurrent);
            break;
        case 2:
            filter.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighShelf(sampleRate, freqCurrent, Q, gainCurrent);
            break;
        case 3: 
            filter.coefficients = juce::dsp::IIR::Coefficients<float>::makeLowPass(sampleRate, freqCurrent, Q);
            break;
        case 4:
            filter.coefficients = juce::dsp::IIR::Coefficients<float>::makeHighPass(sampleRate, freqCurrent, Q);
            break;
        }

        needsUpdate = false;
    }
};

class EchidnaAudioProcessor  : public juce::AudioProcessor, public juce::AudioProcessorParameter::Listener
                            #if JucePlugin_Enable_ARA
                             , public juce::AudioProcessorARAExtension
                            #endif
{
public:
    //==============================================================================
    EchidnaAudioProcessor();
    ~EchidnaAudioProcessor() override;
    void parameterValueChanged(int parameterIndex, float newValue);
    void parameterGestureChanged(int, bool);

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState& getValueTreeState() { return parameters; }
private:
    EQBand bands[10];
    EQBandParameters bandParamNames[10];
    
    juce::AudioProcessorValueTreeState parameters;
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (EchidnaAudioProcessor)
};
