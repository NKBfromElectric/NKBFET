#include "PluginProcessor.h"
#include "PluginEditor.h"

NKB1176AudioProcessorEditor::NKB1176AudioProcessorEditor (NKB1176AudioProcessor& p)
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

    ratioBox.addItemList(juce::StringArray { "4", "8", "12", "20", "ALL" }, 1);
    ratioBox.setColour(juce::ComboBox::backgroundColourId, juce::Colour(0xff180a20));
    ratioBox.setColour(juce::ComboBox::textColourId, juce::Colours::whitesmoke);
    ratioBox.setColour(juce::ComboBox::outlineColourId, juce::Colour(0xff552266));
    addAndMakeVisible(ratioBox);

    ratioLabel.setText("RATIO", juce::dontSendNotification);
    ratioLabel.setJustificationType(juce::Justification::centred);
    ratioLabel.setFont(juce::FontOptions(11.0f, juce::Font::bold));
    ratioLabel.setColour(juce::Label::textColourId, juce::Colours::whitesmoke);
    addAndMakeVisible(ratioLabel);

    inputAttach   = std::make_unique<Attachment>(audioProcessor.apvts, "INPUT",   inputSlider);
    outputAttach  = std::make_unique<Attachment>(audioProcessor.apvts, "OUTPUT",  outputSlider);
    attackAttach  = std::make_unique<Attachment>(audioProcessor.apvts, "ATTACK",  attackSlider);
    releaseAttach = std::make_unique<Attachment>(audioProcessor.apvts, "RELEASE", releaseSlider);
    ratioAttach   = std::make_unique<ComboBoxAttachment>(audioProcessor.apvts, "RATIO", ratioBox);

    startTimerHz(30);
}

NKB1176AudioProcessorEditor::~NKB1176AudioProcessorEditor()
{
    stopTimer();
}

void NKB1176AudioProcessorEditor::timerCallback()
{
    float targetGr = audioProcessor.getGainReductionDb();
    displayedGrDb = displayedGrDb * 0.6f + targetGr * 0.4f;
    repaint();
}

void NKB1176AudioProcessorEditor::paint (juce::Graphics& g)
{
    // 1. 紫ベースのパネル背景
    juce::Colour purpleTop (0xff6b2680);
    juce::Colour purpleBottom (0xff421552);
    g.setGradientFill (juce::ColourGradient (purpleTop, 0, 0, purpleBottom, 0, (float) getHeight(), false));
    g.fillAll();

    // 左右シャーシ＆ビス
    g.setColour (juce::Colours::black.withAlpha(0.35f));
    g.fillRect (0, 0, 18, getHeight());
    g.fillRect (getWidth() - 18, 0, 18, getHeight());

    g.setColour (juce::Colour(0xff222222));
    g.fillEllipse (5, 18, 8, 8);
    g.fillEllipse (5, getHeight() - 26, 8, 8);
    g.fillEllipse (getWidth() - 13, 18, 8, 8);
    g.fillEllipse (getWidth() - 13, getHeight() - 26, 8, 8);

    // 2. タイトルロゴ（被り防止のため上部かつ少し左寄りに調整）
    g.setColour (juce::Colours::whitesmoke);
    g.setFont (juce::FontOptions(15.0f, juce::Font::bold));
    g.drawText ("NKB 1176", 25, 8, 120, 18, juce::Justification::left, true);
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
        float angle = juce::jmap ((float)sliderPos, (float)minVal, (float)maxVal, -2.2f, 2.2f);

        juce::Point<float> pointerStart (cx + capRadius * std::sin(angle), cy - capRadius * std::cos(angle));
        juce::Point<float> pointerEnd (cx + (bodyRadius - 2.0f) * std::sin(angle), cy - (bodyRadius - 2.0f) * std::cos(angle));

        g.setColour (juce::Colours::white);
        g.drawLine (juce::Line<float>(pointerStart, pointerEnd), isLarge ? 2.5f : 2.0f);
    };

    drawAnalogKnob(inputSlider, true);
    drawAnalogKnob(outputSlider, true);
    drawAnalogKnob(attackSlider, false);
    drawAnalogKnob(releaseSlider, false);

    // 4. アナログVUメーター
    auto vuArea = juce::Rectangle<int>(getWidth() - 175, 30, 145, 95);
    
    g.setColour (juce::Colour(0xff0d0d0d));
    g.fillRect (vuArea.expanded(3));

    juce::Colour vuLight (0xffffe082);
    juce::Colour vuDark (0xffd7ccc8);
    g.setGradientFill (juce::ColourGradient (vuLight, (float)vuArea.getX(), (float)vuArea.getY(),
                                             vuDark, (float)vuArea.getX(), (float)vuArea.getBottom(), false));
    g.fillRect (vuArea);

    g.setColour (juce::Colour(0xff212121));
    g.setFont (juce::FontOptions(12.0f, juce::Font::bold));
    g.drawText ("VU", vuArea.removeFromTop(20), juce::Justification::centred, true);
    g.setFont (juce::FontOptions(9.0f, juce::Font::plain));
    g.drawText ("-20 -10 -7 -5 -3 -1 0 +1 +3", vuArea.removeFromTop(15), juce::Justification::centred, true);

    float maxGrDisplay = 20.0f;
    float normGr = juce::jlimit(0.0f, 1.0f, displayedGrDb / maxGrDisplay);
    float angle = juce::jmap(normGr, 0.0f, 1.0f, 0.45f, -0.45f); 

    juce::Point<float> pivot ((float)(getWidth() - 102), 145.0f);
    float needleLength = 65.0f;
    juce::Point<float> needleEnd (pivot.x + needleLength * std::sin(angle), pivot.y - needleLength * std::cos(angle));

    g.setColour (juce::Colour(0xffd50000));
    g.drawLine (juce::Line<float>(pivot, needleEnd), 2.0f);

    auto grTextRect = juce::Rectangle<int>(getWidth() - 175, 135, 145, 22);
    g.setColour (juce::Colour(0xff0d0d0d));
    g.fillRect (grTextRect);
    g.setColour (juce::Colour(0xffffa500));
    g.setFont (juce::FontOptions(11.0f, juce::Font::bold));
    g.drawText ("GR: -" + juce::String(displayedGrDb, 1) + " dB", grTextRect, juce::Justification::centred, true);
}

void NKB1176AudioProcessorEditor::resized()
{
    // ロゴとぶつからないようY開始位置を42pxに下げて配置
    int startY = 42;

    inputLabel.setBounds(30, startY, 95, 16);
    inputSlider.setBounds(30, startY + 16, 95, 125);

    outputLabel.setBounds(150, startY, 95, 16);
    outputSlider.setBounds(150, startY + 16, 95, 125);

    int midX = 285;
    attackLabel.setBounds(midX, startY, 70, 16);
    attackSlider.setBounds(midX, startY + 16, 70, 68);

    releaseLabel.setBounds(midX, startY + 86, 70, 16);
    releaseSlider.setBounds(midX, startY + 102, 70, 68);

    int ratioX = 390;
    ratioLabel.setBounds(ratioX, startY + 40, 55, 18);
    ratioBox.setBounds(ratioX, startY + 62, 55, 24);
}