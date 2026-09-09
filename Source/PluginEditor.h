#pragma once

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PluginLayout.h"

// MC77風 縦長四角形ボタン LookAndFeel
struct SquareButtonLookAndFeel : public juce::LookAndFeel_V4
{
    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool isMouseOverButton, bool isButtonDown) override
    {
        auto bounds = button.getLocalBounds().toFloat();
        bool isToggled = button.getToggleState();

        if (isToggled)
        {
            g.setColour (juce::Colour (0xff121215));
            g.fillRect (bounds);

            auto innerBounds = bounds.reduced (1.0f);
            g.setColour (juce::Colour (0xff202026));
            g.fillRect (innerBounds);

            g.setColour (juce::Colour (0xffff9900));
            g.fillRect (innerBounds.removeFromTop (3.0f));

            g.setColour (juce::Colours::black.withAlpha (0.8f));
            g.drawRect (bounds, 1.0f);
        }
        else
        {
            g.setGradientFill (juce::ColourGradient (
                juce::Colour (0xff555555), bounds.getX(), bounds.getY(),
                juce::Colour (0xff1a1a1a), bounds.getX(), bounds.getBottom(), false));
            g.fillRect (bounds);

            g.setColour (juce::Colours::white.withAlpha (0.25f));
            g.drawLine (bounds.getX() + 1.0f, bounds.getY() + 1.0f, bounds.getRight() - 1.0f, bounds.getY() + 1.0f, 1.0f);

            g.setColour (juce::Colour (0xff0a0a0a));
            g.drawRect (bounds, 1.0f);
        }
    }
};

class NKBFETAudioProcessorEditor  : public juce::AudioProcessorEditor, private juce::Timer
{
    friend class PluginLayout;

public:
    NKBFETAudioProcessorEditor (NKBFETAudioProcessor&);
    ~NKBFETAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void timerCallback() override;
    void updateRatioButtonsFromParam();

    NKBFETAudioProcessor& audioProcessor;

    SquareButtonLookAndFeel squareButtonLaf;

    juce::Slider inputSlider, outputSlider, attackSlider, releaseSlider, driveSlider;
    juce::Label inputLabel, outputLabel, attackLabel, releaseLabel, driveLabel, ratioLabel;

    std::array<juce::TextButton, 5> ratioButtons;

    using Attachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    std::unique_ptr<Attachment> inputAttach, outputAttach, attackAttach, releaseAttach, driveAttach;

    float displayedGrDb = 0.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NKBFETAudioProcessorEditor)
};