#include "PluginProcessor.h"
#include "PluginEditor.h"

NKB1176AudioProcessor::NKB1176AudioProcessor()
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

NKB1176AudioProcessor::~NKB1176AudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout NKB1176AudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back(std::make_unique<juce::AudioParameterFloat>("INPUT",   "Input Gain", -20.0f, 20.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("OUTPUT",  "Output Gain", -20.0f, 20.0f, 0.0f));
    params.push_back(std::make_unique<juce::AudioParameterChoice>("RATIO",   "Ratio", juce::StringArray { "4:1", "8:1", "12:1", "20:1", "ALL" }, 0));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("ATTACK",  "Attack (Fast<-Slow)", 1.0f, 7.0f, 4.0f));
    params.push_back(std::make_unique<juce::AudioParameterFloat>("RELEASE", "Release (Fast<-Slow)", 1.0f, 7.0f, 4.0f));

    return { params.begin(), params.end() };
}

const juce::String NKB1176AudioProcessor::getName() const { return JucePlugin_Name; }
bool NKB1176AudioProcessor::acceptsMidi() const { return false; }
bool NKB1176AudioProcessor::producesMidi() const { return false; }
bool NKB1176AudioProcessor::isMidiEffect() const { return false; }
double NKB1176AudioProcessor::getTailLengthSeconds() const { return 0.0; }
int NKB1176AudioProcessor::getNumPrograms() { return 1; }
int NKB1176AudioProcessor::getCurrentProgram() { return 0; }
void NKB1176AudioProcessor::setCurrentProgram (int index) {}
const juce::String NKB1176AudioProcessor::getProgramName (int index) { return {}; }
void NKB1176AudioProcessor::changeProgramName (int index, const juce::String& newName) {}

void NKB1176AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    envelopeState = 0.0f;
    currentGainReductionDb.store(0.0f);
}

void NKB1176AudioProcessor::releaseResources() {}

bool NKB1176AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;
    return true;
}

void NKB1176AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    auto totalNumInputChannels  = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    float inputDb   = apvts.getRawParameterValue("INPUT")->load();
    float outputDb  = apvts.getRawParameterValue("OUTPUT")->load();
    int   ratioIdx  = static_cast<int>(apvts.getRawParameterValue("RATIO")->load());
    float attackVal = apvts.getRawParameterValue("ATTACK")->load();
    float relVal    = apvts.getRawParameterValue("RELEASE")->load();

    float inputLinear  = juce::Decibels::decibelsToGain(inputDb);
    float outputLinear = juce::Decibels::decibelsToGain(outputDb);

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
            
            float saturated = std::tanh(compressed * 1.1f);

            channelData[i] = saturated * outputLinear;
        }
    }

    currentGainReductionDb.store(maxGrThisBlock);
}

bool NKB1176AudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* NKB1176AudioProcessor::createEditor()
{
    return new NKB1176AudioProcessorEditor (*this);
}

void NKB1176AudioProcessor::getStateInformation (juce::MemoryBlock& destData) {}
void NKB1176AudioProcessor::setStateInformation (const void* data, int sizeInBytes) {}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new NKB1176AudioProcessor();
}