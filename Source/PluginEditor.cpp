#include "PluginProcessor.h"
#include "PluginEditor.h"

NKBFETAudioProcessorEditor::NKBFETAudioProcessorEditor (NKBFETAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    setSize (720, 230);
    setResizable (false, false);

    auto setupKnob = [this](juce::Slider& slider, juce::Label& label, const juce::String& text) {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 16);
        slider.setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::transparentBlack);
        slider.setColour(juce::Slider::rotarySliderOutlineColourId, juce::Colours::transparentBlack);
        slider.setColour(juce::Slider::thumbColourId, juce::Colours::transparentBlack);
        slider.setColour(juce::Slider::textBoxOutlineColourId, juce::Colour(0xff442255));
        slider.setColour(juce::Slider::textBoxTextColourId, juce::Colours::whitesmoke);
        addAndMakeVisible(slider);

        label.setText(text, juce::dontSendNotification);
        label.setJustificationType(juce::Justification::centred);
        label.setFont(juce::FontOptions(11.0f, juce::Font::bold));
        label.setColour(juce::Label::textColourId, juce::Colours::whitesmoke);
        addAndMakeVisible(label);
    };

    setupKnob(inputSlider, inputLabel, "INPUT");
    setupKnob(outputSlider, outputLabel, "OUTPUT");
    setupKnob(attackSlider, attackLabel, "ATTACK");
    setupKnob(releaseSlider, releaseLabel, "RELEASE");
    setupKnob(driveSlider, driveLabel, "DRIVE");

    ratioLabel.setText("RATIO", juce::dontSendNotification);
    ratioLabel.setJustificationType(juce::Justification::centred);
    ratioLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    ratioLabel.setColour(juce::Label::textColourId, juce::Colours::whitesmoke);
    addAndMakeVisible(ratioLabel);

    for (size_t i = 0; i < ratioButtons.size(); ++i)
    {
        auto& btn = ratioButtons[i];
        btn.setLookAndFeel (&squareButtonLaf);
        btn.setButtonText("");
        btn.setRadioGroupId(1001);
        btn.setClickingTogglesState(true);

        btn.onClick = [this, i]() {
            if (auto* param = audioProcessor.apvts.getParameter("RATIO"))
            {
                param->beginChangeGesture();
                param->setValueNotifyingHost(static_cast<float>(i) / 4.0f);
                param->endChangeGesture();
            }
        };

        addAndMakeVisible(btn);
    }

    inputAttach   = std::make_unique<Attachment>(audioProcessor.apvts, "INPUT",   inputSlider);
    outputAttach  = std::make_unique<Attachment>(audioProcessor.apvts, "OUTPUT",  outputSlider);
    attackAttach  = std::make_unique<Attachment>(audioProcessor.apvts, "ATTACK",  attackSlider);
    releaseAttach = std::make_unique<Attachment>(audioProcessor.apvts, "RELEASE", releaseSlider);
    driveAttach   = std::make_unique<Attachment>(audioProcessor.apvts, "DRIVE",   driveSlider);

    updateRatioButtonsFromParam();
    startTimerHz(30);
}

NKBFETAudioProcessorEditor::~NKBFETAudioProcessorEditor()
{
    stopTimer();
    for (auto& btn : ratioButtons)
        btn.setLookAndFeel (nullptr);
}

void NKBFETAudioProcessorEditor::updateRatioButtonsFromParam()
{
    int ratioIdx = static_cast<int>(audioProcessor.apvts.getRawParameterValue("RATIO")->load());
    ratioIdx = juce::jlimit(0, 4, ratioIdx);
    ratioButtons[static_cast<size_t>(ratioIdx)].setToggleState(true, juce::dontSendNotification);
}

void NKBFETAudioProcessorEditor::timerCallback()
{
    float targetGr = audioProcessor.getGainReductionDb();
    displayedGrDb = displayedGrDb * 0.6f + targetGr * 0.4f;

    updateRatioButtonsFromParam();
    repaint();
}

void NKBFETAudioProcessorEditor::paint (juce::Graphics& g)
{
    // 1. 紫ベースのパネル背景
    juce::Colour purpleTop (0xff6b2680);
    juce::Colour purpleBottom (0xff421552);
    g.setGradientFill (juce::ColourGradient (purpleTop, 0.0f, 0.0f, purpleBottom, 0.0f, static_cast<float>(getHeight()), false));
    g.fillAll();

    // 左右シャーシ＆ビス
    g.setColour (juce::Colours::black.withAlpha(0.35f));
    g.fillRect (0, 0, 18, getHeight());
    g.fillRect (getWidth() - 18, 0, 18, getHeight());

    g.setColour (juce::Colour(0xff222222));
    g.fillEllipse (5.0f, 18.0f, 8.0f, 8.0f);
    g.fillEllipse (5.0f, static_cast<float>(getHeight() - 26), 8.0f, 8.0f);
    g.fillEllipse (static_cast<float>(getWidth() - 13), 18.0f, 8.0f, 8.0f);
    g.fillEllipse (static_cast<float>(getWidth() - 13), static_cast<float>(getHeight() - 26), 8.0f, 8.0f);

    // 2. タイトルロゴ
    g.setColour (juce::Colours::whitesmoke);
    g.setFont (juce::FontOptions(15.0f, juce::Font::bold));
    g.drawText ("NKB FET", 25, 8, 120, 18, juce::Justification::left, true);
    g.setFont (juce::FontOptions(9.0f, juce::Font::plain));
    g.drawText ("LIMITING AMPLIFIER", 25, 24, 120, 12, juce::Justification::left, true);

    // 3. アナログ風ノブ描画
    auto drawAnalogKnob = [&g](juce::Slider& slider, bool isLarge) {
        auto bounds = slider.getBounds().toFloat();
        float width = bounds.getWidth();
        float height = bounds.getHeight() - 16.0f;
        float size = std::min(width, height) - 6.0f;
        
        float cx = bounds.getX() + width * 0.5f;
        float cy = bounds.getY() + height * 0.5f;
        float radius = size * 0.5f;

        g.setColour (juce::Colours::black.withAlpha(0.5f));
        g.fillEllipse (cx - radius + 2.0f, cy - radius + 3.0f, size, size);

        g.setGradientFill (juce::ColourGradient (juce::Colour(0xff666666), cx, cy - radius,
                                                 juce::Colour(0xff111111), cx, cy + radius, false));
        g.fillEllipse (cx - radius, cy - radius, size, size);

        float bodySize = size - (isLarge ? 6.0f : 4.0f);
        float bodyRadius = bodySize * 0.5f;
        g.setGradientFill (juce::ColourGradient (juce::Colour(0xff333333), cx, cy - bodyRadius,
                                                 juce::Colour(0xff0d0d0d), cx, cy + bodyRadius, false));
        g.fillEllipse (cx - bodyRadius, cy - bodyRadius, bodySize, bodySize);

        float capSize = bodySize * 0.45f;
        float capRadius = capSize * 0.5f;
        g.setGradientFill (juce::ColourGradient (juce::Colour(0xffefefef), cx - capRadius, cy - capRadius,
                                                 juce::Colour(0xff777777), cx + capRadius, cy + capRadius, false));
        g.fillEllipse (cx - capRadius, cy - capRadius, capSize, capSize);

        double sliderPos = slider.getValue();
        double minVal = slider.getMinimum();
        double maxVal = slider.getMaximum();
        float angle = juce::jmap (static_cast<float>(sliderPos), static_cast<float>(minVal), static_cast<float>(maxVal), -2.2f, 2.2f);

        juce::Point<float> pointerStart (cx + capRadius * std::sin(angle), cy - capRadius * std::cos(angle));
        juce::Point<float> pointerEnd (cx + (bodyRadius - 2.0f) * std::sin(angle), cy - (bodyRadius - 2.0f) * std::cos(angle));

        g.setColour (juce::Colours::white);
        g.drawLine (juce::Line<float>(pointerStart, pointerEnd), isLarge ? 2.5f : 2.0f);
    };

    drawAnalogKnob(inputSlider, true);
    drawAnalogKnob(outputSlider, true);
    drawAnalogKnob(attackSlider, false);
    drawAnalogKnob(releaseSlider, false);
    drawAnalogKnob(driveSlider, false);

    // 4. MC77風 RATIOの左側シルク印刷テキスト
    g.setColour (juce::Colours::whitesmoke);
    g.setFont (juce::FontOptions(10.0f, juce::Font::bold));

    const juce::StringArray ratioTextLabels { "4", "8", "12", "20", "ALL" };
    int buttonX = 450;
    int btnH = 24;
    int btnStartY = 42 + 18;

    for (size_t i = 0; i < ratioButtons.size(); ++i)
    {
        int textY = btnStartY + static_cast<int>(i) * btnH;
        g.drawText (ratioTextLabels[static_cast<int>(i)], buttonX - 32, textY, 26, btnH, juce::Justification::right, false);
    }

    // 5. デジタルGRインジケーター（Y=56）
    auto displayArea = juce::Rectangle<int>(getWidth() - 190, 56, 160, 110);
    
    g.setColour (juce::Colour(0xff121212));
    g.fillRect (displayArea);
    g.setColour (juce::Colour(0xff442255));
    g.drawRect (displayArea, 2.0f);

    g.setColour (juce::Colour(0xff888888));
    g.setFont (juce::FontOptions(10.0f, juce::Font::bold));
    g.drawText ("GAIN REDUCTION", displayArea.getX(), displayArea.getY() + 6, displayArea.getWidth(), 14, juce::Justification::centred, true);

    g.setColour (juce::Colour(0xffffa500));
    g.setFont (juce::FontOptions(22.0f, juce::Font::bold));
    juce::String grText = "-" + juce::String(displayedGrDb, 1) + " dB";
    g.drawText (grText, displayArea.getX(), displayArea.getY() + 24, displayArea.getWidth(), 30, juce::Justification::centred, true);

    auto barArea = juce::Rectangle<float>(static_cast<float>(displayArea.getX()) + 12.0f, static_cast<float>(displayArea.getY()) + 60.0f, static_cast<float>(displayArea.getWidth()) - 24.0f, 12.0f);
    g.setColour (juce::Colour(0xff222222));
    g.fillRect (barArea);

    float maxGrDisplay = 20.0f;
    float normGr = juce::jlimit(0.0f, 1.0f, displayedGrDb / maxGrDisplay);
    float fillWidth = barArea.getWidth() * normGr;

    if (fillWidth > 0.0f)
    {
        auto fillRect = juce::Rectangle<float>(barArea.getX(), barArea.getY(), fillWidth, barArea.getHeight());
        g.setColour (juce::Colour(0xffff5500));
        g.fillRect (fillRect);
    }

    g.setColour (juce::Colour(0xff777777));
    g.setFont (juce::FontOptions(9.0f, juce::Font::plain));
    g.drawText ("0", static_cast<int>(barArea.getX()) - 5, static_cast<int>(barArea.getBottom()) + 2, 20, 12, juce::Justification::left, false);
    g.drawText ("10", static_cast<int>(barArea.getCentreX()) - 10, static_cast<int>(barArea.getBottom()) + 2, 20, 12, juce::Justification::centred, false);
    g.drawText ("20", static_cast<int>(barArea.getRight()) - 15, static_cast<int>(barArea.getBottom()) + 2, 20, 12, juce::Justification::right, false);
}

void NKBFETAudioProcessorEditor::resized()
{
    PluginLayout::applyLayout(*this);
}