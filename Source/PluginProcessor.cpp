/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

const std::array<EQBandParameters, 5> EchidnaAudioProcessor::bandParamNames = [] {
    std::array<EQBandParameters, 5> names;
    for (int i = 0; i < 5; ++i)
    {
        names[i] = {
            "BAND" + juce::String(i + 1) + "_GAIN",
            "BAND" + juce::String(i + 1) + "_GAIN_SPEED",
            "BAND" + juce::String(i + 1) + "_GAIN_MIN",
            "BAND" + juce::String(i + 1) + "_GAIN_MAX",
            "BAND" + juce::String(i + 1) + "_GAIN_DIRECTION",
            "BAND" + juce::String(i + 1) + "_FREQ",
            "BAND" + juce::String(i + 1) + "_FREQ_SPEED",
            "BAND" + juce::String(i + 1) + "_FREQ_MIN",
            "BAND" + juce::String(i + 1) + "_FREQ_MAX",
            "BAND" + juce::String(i + 1) + "_FREQ_DIRECTION",
            "BAND" + juce::String(i + 1) + "_Q",
            "BAND" + juce::String(i + 1) + "_TYPE"
        };
    }
    return names;
}();

//==============================================================================
EchidnaAudioProcessor::EchidnaAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
    parameters(*this, nullptr, "PARAMETERS", createParameterLayout())

#endif
{
    
}

EchidnaAudioProcessor::~EchidnaAudioProcessor()
{
}

//==============================================================================
const juce::String EchidnaAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool EchidnaAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool EchidnaAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool EchidnaAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double EchidnaAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int EchidnaAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int EchidnaAudioProcessor::getCurrentProgram()
{
    return 0;
}

void EchidnaAudioProcessor::setCurrentProgram (int index)
{
}

const juce::String EchidnaAudioProcessor::getProgramName (int index)
{
    return {};
}

void EchidnaAudioProcessor::changeProgramName (int index, const juce::String& newName)
{
}

//==============================================================================
void EchidnaAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    
    for (int i = 0; i < 5; ++i)
    {
       bands[i].filter.reset();
    }
    
}

void EchidnaAudioProcessor::releaseResources()
{
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool EchidnaAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
  #else
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void EchidnaAudioProcessor::processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    // Update the EQ bands
    for (int i = 0; i < 5; ++i)
    {
        UpdateBandParameters(i);
    }

    // Process audio data with the updated filter bands
    auto block = juce::dsp::AudioBlock<float>(buffer);
    for (int i = 0; i < 5; ++i)
    {
        for (int j = 0; j < buffer.getNumChannels(); j++)
        {
            auto channelBlock = block.getSingleChannelBlock(j);
            bands[i].filter.process(juce::dsp::ProcessContextReplacing<float>(channelBlock));
        }
    }
}

//==============================================================================
bool EchidnaAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* EchidnaAudioProcessor::createEditor()
{
    return new juce::GenericAudioProcessorEditor(*this);
}

//==============================================================================
void EchidnaAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void EchidnaAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

void EchidnaAudioProcessor::UpdateBandParameters(int bandIndex)
{
    float currentGain = *parameters.getRawParameterValue(bandParamNames[bandIndex].gainCurrent);
    float currentFreq = *parameters.getRawParameterValue(bandParamNames[bandIndex].freqCurrent);
    float minGain = *parameters.getRawParameterValue(bandParamNames[bandIndex].gainMin);
    float maxGain = *parameters.getRawParameterValue(bandParamNames[bandIndex].gainMax);
    float minFreq = *parameters.getRawParameterValue(bandParamNames[bandIndex].freqMin);
    float maxFreq = *parameters.getRawParameterValue(bandParamNames[bandIndex].freqMax);

    if ((bands[bandIndex].gainDirection > 0 && currentGain >= maxGain) ||
        (bands[bandIndex].gainDirection < 0 && currentGain <= minGain))
    {
        bands[bandIndex].gainDirection = -bands[bandIndex].gainDirection;
    }
    if ((bands[bandIndex].freqDirection > 0 && currentFreq >= maxFreq) ||
        (bands[bandIndex].freqDirection < 0 && currentFreq <= minFreq))
    {
        bands[bandIndex].freqDirection = -bands[bandIndex].freqDirection;
    }

    bands[bandIndex].gainCurrent = currentGain;
    bands[bandIndex].freqCurrent = currentFreq;

    bands[bandIndex].updateCoefficients(getSampleRate());
}

void EchidnaAudioProcessor::parameterValueChanged(int parameterIndex, float newValue)
{
    /*
    int bandIndex = parameterIndex / 12;  
    if (bandIndex >= 0 && bandIndex < 10)
    {
        bands[bandIndex].needsUpdate = true;
    }
    */
}

void EchidnaAudioProcessor::parameterGestureChanged(int, bool)
{
}

juce::AudioProcessorValueTreeState::ParameterLayout EchidnaAudioProcessor::createParameterLayout()
{
    /*  
        "BAND1GAIN", 
        "BAND1FREQ", 
        "BAND1Q", 
        "BAND1TYPE", 
        "BAND1GAIN_MIN", 
        "BAND1GAIN_MAX",
        "BAND1GAIN_SPEED", 
        "BAND1GAIN_FREQ_MIN",
        "BAND1FREQ_MAX", 
        "BAND1FREQ_SPEED",
        */
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
    
    for (int i = 0; i < 5; ++i) 
    {
        params.push_back(std::make_unique<juce::AudioParameterFloat> (bandParamNames[i].gainCurrent, "Band " + juce::String(i) + " Gain", -10.0f, 10.0f, 0.1f));
        params.push_back(std::make_unique<juce::AudioParameterFloat> (bandParamNames[i].freqCurrent, "Band " + juce::String(i) + " Frequency", 20.0f, 20000.0f, 1000.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat> (bandParamNames[i].Q, "Band " + juce::String(i) + " Q", 0.1f, 10.0f, 1.0f));
        params.push_back(std::make_unique<juce::AudioParameterChoice>(bandParamNames[i].type, "Band " + juce::String(i) + " Type", juce::StringArray{"Bell", "Low Shelf", "High Shelf", "Low Pass", "High Pass"}, 0));
        params.push_back(std::make_unique<juce::AudioParameterFloat> (bandParamNames[i].gainMin, "Band " + juce::String(i) + " Gain Min", 0.0f, 2.0f, 0.1f));
        params.push_back(std::make_unique<juce::AudioParameterFloat> (bandParamNames[i].gainMax, "Band " + juce::String(i) + " Gain Max", 0.0f, 2.0f, 0.1f));
        params.push_back(std::make_unique<juce::AudioParameterFloat> (bandParamNames[i].gainSpeed, "Band " + juce::String(i) + " Gain Speed", 0.001f, 3.0f, 0.01f)); 
        params.push_back(std::make_unique<juce::AudioParameterFloat> (bandParamNames[i].freqMin, "Band " + juce::String(i) + " Freq Min", 20.0f, 2000.0f, 200.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat> (bandParamNames[i].freqMax, "Band " + juce::String(i) + " Freq Max", 20.0f, 2000.0f, 200.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat> (bandParamNames[i].freqSpeed, "Band " + juce::String(i) + "Freq Speed", 0.0001f, 1.0f, 0.001f));
        params.push_back(std::make_unique<juce::AudioParameterFloat> (bandParamNames[i].gainDirection, "Band " + juce::String(i) + "Gain Dir", -1.0f, 1.0f, 0.f));
        params.push_back(std::make_unique<juce::AudioParameterFloat> (bandParamNames[i].freqDirection, "Band " + juce::String(i) + "Freq Dir", -1.0f, 1.0f, 0.f));
    }

    return { params.begin(), params.end() };
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new EchidnaAudioProcessor();
}
