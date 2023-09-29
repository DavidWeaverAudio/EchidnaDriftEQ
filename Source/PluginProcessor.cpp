/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

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
    for (int i = 0; i < 10; ++i)
    {
        bandParamNames[i] = {
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
    for (int i = 0; i < 10; ++i)
    {
        auto wa_addListener = [this](const juce::String& paramName) {
            auto* param = parameters.getParameter(paramName);
            jassert(param != nullptr && "Parameter doesn't exist!");
            if (param)
                param->addListener(this);
        };

        wa_addListener(bandParamNames[i].gainCurrent);
        wa_addListener(bandParamNames[i].gainSpeed);
        wa_addListener(bandParamNames[i].gainMin);
        wa_addListener(bandParamNames[i].gainMax);
        wa_addListener(bandParamNames[i].gainDirection);
        wa_addListener(bandParamNames[i].freqCurrent);
        wa_addListener(bandParamNames[i].freqSpeed);
        wa_addListener(bandParamNames[i].freqMin);
        wa_addListener(bandParamNames[i].freqMax);
        wa_addListener(bandParamNames[i].freqDirection);
        wa_addListener(bandParamNames[i].Q);
        wa_addListener(bandParamNames[i].type);
    }
}

EchidnaAudioProcessor::~EchidnaAudioProcessor()
{
    for (int i = 0; i < 10; ++i)
    {
        auto wa_removeListener = [this](const juce::String& paramName) {
            auto* param = parameters.getParameter(paramName);
            jassert(param != nullptr && "Parameter doesn't exist!");

            if (param)
                param->removeListener(this);
        };

        wa_removeListener(bandParamNames[i].gainCurrent);
        wa_removeListener(bandParamNames[i].gainSpeed);
        wa_removeListener(bandParamNames[i].gainMin);
        wa_removeListener(bandParamNames[i].gainMax);
        wa_removeListener(bandParamNames[i].gainDirection);
        wa_removeListener(bandParamNames[i].freqCurrent);
        wa_removeListener(bandParamNames[i].freqSpeed);
        wa_removeListener(bandParamNames[i].freqMin);
        wa_removeListener(bandParamNames[i].freqMax);
        wa_removeListener(bandParamNames[i].freqDirection);
        wa_removeListener(bandParamNames[i].Q);
        wa_removeListener(bandParamNames[i].type);
    }
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
    for (int i = 0; i < 10; ++i)
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

void EchidnaAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    for (int i = 0; i < 10; ++i)
    {
        float currentGain = *parameters.getRawParameterValue(bandParamNames[i].gainCurrent);
        float minGain = *parameters.getRawParameterValue(bandParamNames[i].gainMin);
        float maxGain = *parameters.getRawParameterValue(bandParamNames[i].gainMax);
        float speed = *parameters.getRawParameterValue(bandParamNames[i].gainSpeed);
        
        if ((bands[i].gainDirection > 0 && currentGain >= maxGain) ||
            (bands[i].gainDirection < 0 && currentGain <= minGain))
        {
            bands[i].gainDirection = -bands[i].gainDirection; 
        }

        currentGain += bands[i].gainDirection * speed;
        parameters.getParameter(bandParamNames[i].gainCurrent)->setValueNotifyingHost(currentGain);

        float currentFreq = *parameters.getRawParameterValue(bandParamNames[i].freqCurrent);
        float minFreq = *parameters.getRawParameterValue(bandParamNames[i].freqMin);
        float maxFreq = *parameters.getRawParameterValue(bandParamNames[i].freqMax);
        float freqSpeed = *parameters.getRawParameterValue(bandParamNames[i].freqSpeed);

        if ((bands[i].freqDirection > 0 && currentFreq >= maxFreq) ||
            (bands[i].freqDirection < 0 && currentGain <= minFreq))
        {
            bands[i].freqDirection = -bands[i].freqDirection;
        }

        currentFreq += bands[i].freqDirection * freqSpeed;
        parameters.getParameter(bandParamNames[i].freqCurrent)->setValueNotifyingHost(currentFreq);

        if (bands[i].needsUpdate)
        {
            bands[i].gainCurrent = *parameters.getRawParameterValue(bandParamNames[i].gainCurrent);
            bands[i].gainSpeed = *parameters.getRawParameterValue(bandParamNames[i].gainSpeed);
            bands[i].gainMin = *parameters.getRawParameterValue(bandParamNames[i].gainMin);
            bands[i].gainMax = *parameters.getRawParameterValue(bandParamNames[i].gainMax);
            bands[i].gainDirection = *parameters.getRawParameterValue(bandParamNames[i].gainDirection);
            bands[i].freqCurrent = *parameters.getRawParameterValue(bandParamNames[i].freqCurrent);
            bands[i].freqSpeed = *parameters.getRawParameterValue(bandParamNames[i].freqSpeed);
            bands[i].freqMin = *parameters.getRawParameterValue(bandParamNames[i].freqMin);
            bands[i].freqMax = *parameters.getRawParameterValue(bandParamNames[i].freqMax);
            bands[i].freqDirection = *parameters.getRawParameterValue(bandParamNames[i].freqDirection);
            bands[i].Q = *parameters.getRawParameterValue(bandParamNames[i].Q);
            bands[i].type = (int)*parameters.getRawParameterValue(bandParamNames[i].type);
            bands[i].updateCoefficients(getSampleRate());
        }

        bands[i].filter.process(juce::dsp::ProcessContextReplacing<float>(juce::dsp::AudioBlock<float>(buffer)));
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

void EchidnaAudioProcessor::parameterValueChanged(int parameterIndex, float newValue)
{
    int bandIndex = parameterIndex / 12;  
    if (bandIndex >= 0 && bandIndex < 10)
    {
        bands[bandIndex].needsUpdate = true;
    }
}

void EchidnaAudioProcessor::parameterGestureChanged(int, bool)
{
}

juce::AudioProcessorValueTreeState::ParameterLayout EchidnaAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;
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
    for (int i = 0; i < 10; ++i) 
    {
        params.push_back(std::make_unique<juce::AudioParameterFloat> (bandParamNames[i].gainCurrent, "Band " + juce::String(i) + " Gain", 0.0f, 2.0f, 1.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat> (bandParamNames[i].freqCurrent, "Band " + juce::String(i) + " Frequency", 20.0f, 20000.0f, 1000.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat> (bandParamNames[i].Q, "Band " + juce::String(i) + " Q", 0.1f, 10.0f, 1.0f));
        params.push_back(std::make_unique<juce::AudioParameterChoice>(bandParamNames[i].type, "Band " + juce::String(i) + " Type", juce::StringArray{"Bell", "Low Shelf", "High Shelf", "Low Pass", "High Pass"}, 0));
        params.push_back(std::make_unique<juce::AudioParameterFloat> (bandParamNames[i].gainMin, "Band " + juce::String(i) + " Gain Min", 0.0f, 2.0f, 0.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat> (bandParamNames[i].gainMax, "Band " + juce::String(i) + " Gain Max", 0.0f, 2.0f, 2.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat> (bandParamNames[i].gainSpeed, "Band " + juce::String(i) + " Gain Speed", 0.001f, 0.1f, 0.01f)); 
        params.push_back(std::make_unique<juce::AudioParameterFloat> (bandParamNames[i].freqMin, "Band " + juce::String(i) + " Freq Min", 20.0f, 2000.0f, 200.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat> (bandParamNames[i].freqMax, "Band " + juce::String(i) + " Freq Max", 20.0f, 2000.0f, 200.0f));
        params.push_back(std::make_unique<juce::AudioParameterFloat> (bandParamNames[i].freqSpeed, "Band " + juce::String(i) + "Freq Speed", 0.001f, 0.1f, 0.01f));
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