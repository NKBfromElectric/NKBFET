#pragma once

#include <JuceHeader.h>

class NKB1176AudioProcessor  : public juce::AudioProcessor
{
public:
    NKB1176AudioProcessor();
    ~NKB1176AudioProcessor() override;

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

    float getGainReductionDb() const { return currentGainReductionDb.load(); }

    juce::AudioProcessorValueTreeState apvts;

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    float envelopeState = 0.0f;
    std::atomic<float> currentGainReductionDb { 0.0f };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NKB1176AudioProcessor)
};