#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"

class NKB1176AudioProcessorEditor  : public juce::AudioProcessorEditor, private juce::Timer
{
public:
    NKB1176AudioProcessorEditor (NKB1176AudioProcessor&);
    ~NKB1176AudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;

    NKB1176AudioProcessor& audioProcessor;

    juce::Slider inputSlider, outputSlider, attackSlider, releaseSlider;
    juce::Label inputLabel, outputLabel, attackLabel, releaseLabel, ratioLabel;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboBoxAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    juce::ComboBox ratioBox;
    std::unique_ptr<Attachment> inputAttach, outputAttach, attackAttach, releaseAttach;
    std::unique_ptr<ComboBoxAttachment> ratioAttach;

    float displayedGrDb = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NKB1176AudioProcessorEditor)
};