#include "PluginProcessor.h"
#include "PluginEditor.h"

NKBFETAudioProcessor::NKBFETAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
       apvts(*this, nullptr, "Parameters", createParameterLayout())
#endif
{
}

NKBFETAudioProcessor::~NKBFETAudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout NKBFETAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>("INPUT",   "Input Gain", -20.0f, 20.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("OUTPUT",  "Output Gain", -20.0f, 20.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("DRIVE",   "Drive", 0.0f, 24.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterChoice>("RATIO",   "Ratio", juce::StringArray { "4", "8", "12", "20", "ALL" }, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("ATTACK",  "Attack (Fast<-Slow)", 1.0f, 7.0f, 4.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("RELEASE", "Release (Fast<-Slow)", 1.0f, 7.0f, 4.0f));

    return { params.begin(), params.end() };
}

const juce::String NKBFETAudioProcessor::getName() const { return JucePlugin_Name; }
bool NKBFETAudioProcessor::acceptsMidi() const { return false; }
bool NKBFETAudioProcessor::producesMidi() const { return false; }
bool NKBFETAudioProcessor::isMidiEffect() const { return false; }
double NKBFETAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int NKBFETAudioProcessor::getNumPrograms() { return 1; }
int NKBFETAudioProcessor::getCurrentProgram() { return 0; }
void NKBFETAudioProcessor::setCurrentProgram (int index) {}
const juce::String NKBFETAudioProcessor::getProgramName (int index) { return {}; }
void NKBFETAudioProcessor::changeProgramName (int index, const juce::String& newName) {}

void NKBFETAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    envelopeState = 0.0f;
    currentGainReductionDb.store(0.0f);
}

void NKBFETAudioProcessor::releaseResources() {}

bool NKBFETAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return true;
}

void NKBFETAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    float inputDb   = apvts.getRawParameterValue("INPUT")->load();
    float outputDb  = apvts.getRawParameterValue("OUTPUT")->load();
    float drivedB   = apvts.getRawParameterValue("DRIVE")->load();
    int   ratioIdx  = static_cast<int>(apvts.getRawParameterValue("RATIO")->load());
    float attackVal = apvts.getRawParameterValue("ATTACK")->load();
    float relVal    = apvts.getRawParameterValue("RELEASE")->load();

    float inputLinear  = juce::Decibels::decibelsToGain(inputDb);
    float outputLinear = juce::Decibels::decibelsToGain(outputDb);
    float driveGain    = juce::Decibels::decibelsToGain(drivedB);
    float driveCompensate = 1.0f / (1.0f + drivedB * 0.04f);

    float ratio = 4.0f;
    bool isAllButtons = false;
    if (ratioIdx == 1) ratio = 8.0f;
    else if (ratioIdx == 2) ratio = 12.0f;
    else if (ratioIdx == 3) ratio = 20.0f;
    else if (ratioIdx == 4) { ratio = 12.0f; isAllButtons = true; }

    float attackMs = juce::jmap(attackVal, 1.0f, 7.0f, 0.8f, 0.02f);
    float releaseMs = juce::jmap(relVal, 1.0f, 7.0f, 1100.0f, 50.0f);

    float sampleRate = static_cast<float>(getSampleRate());
    if (sampleRate <= 0.0f) sampleRate = 44100.0f;

    float attackCoeff  = std::exp(-1.0f / (sampleRate * (attackMs / 1000.0f)));
    float releaseCoeff = std::exp(-1.0f / (sampleRate * (releaseMs / 1000.0f)));

    float fixedThreshDb = -20.0f;
    float fixedThreshLinear = juce::Decibels::decibelsToGain(fixedThreshDb);

    int numSamples = buffer.getNumSamples();
    float maxGrThisBlock = 0.0f;

    for (int i = 0; i < numSamples; ++i)
    {
        for (int ch = 0; ch < totalNumInputChannels; ++ch)
        {
            float* channelData = buffer.getWritePointer(ch);
            channelData[i] *= inputLinear;
        }

        float envIn = 0.0f;
        for (int ch = 0; ch < totalNumInputChannels; ++ch)
        {
            envIn = std::max(envIn, std::abs(buffer.getSample(ch, i)));
        }

        if (envIn > envelopeState)
            envelopeState = attackCoeff * envelopeState + (1.0f - attackCoeff) * envIn;
        else
            envelopeState = releaseCoeff * envelopeState + (1.0f - releaseCoeff) * envIn;

        float gainGain = 1.0f;
        float gainReductionDb = 0.0f;

        if (envelopeState > fixedThreshLinear && envelopeState > 0.00001f)
        {
            float envDb = juce::Decibels::gainToDecibels(envelopeState);
            float excessDb = envDb - fixedThreshDb;
            gainReductionDb = excessDb * (1.0f - (1.0f / ratio));
            
            if (isAllButtons) gainReductionDb *= 1.4f;
            
            gainGain = juce::Decibels::decibelsToGain(-gainReductionDb);
        }

        maxGrThisBlock = std::max(maxGrThisBlock, gainReductionDb);

        for (int ch = 0; ch < totalNumInputChannels; ++ch)
        {
            float* channelData = buffer.getWritePointer(ch);
            float compressed = channelData[i] * gainGain;
            
            float driven = compressed * driveGain;
            float saturated = std::tanh(driven * 1.1f) * driveCompensate;

            channelData[i] = saturated * outputLinear;
        }
    }

    currentGainReductionDb.store(maxGrThisBlock);
}

bool NKBFETAudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* NKBFETAudioProcessor::createEditor()
{
    return new NKBFETAudioProcessorEditor (*this);
}

void NKBFETAudioProcessor::getStateInformation (juce::MemoryBlock& destData) {}
void NKBFETAudioProcessor::setStateInformation (const void* data, int sizeInBytes) {}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NKBFETAudioProcessor();
}