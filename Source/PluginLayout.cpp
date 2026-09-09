#include "PluginLayout.h"
#include "PluginEditor.h"

void PluginLayout::applyLayout(NKBFETAudioProcessorEditor& editor)
{
    int startY = 42;

    // 1. INPUT & OUTPUT
    editor.inputLabel.setBounds(25, startY, 95, 16);
    editor.inputSlider.setBounds(25, startY + 16, 95, 125);

    editor.outputLabel.setBounds(130, startY, 95, 16);
    editor.outputSlider.setBounds(130, startY + 16, 95, 125);

    // 2. ATTACK & RELEASE
    int midX = 230;
    editor.attackLabel.setBounds(midX, startY - 2, 75, 16);
    editor.attackSlider.setBounds(midX, startY + 14, 75, 72);

    editor.releaseLabel.setBounds(midX, startY + 84, 75, 16);
    editor.releaseSlider.setBounds(midX, startY + 100, 75, 72);

    // 3. DRIVE（視覚的な中心に合わせて 340 -> 320 へ移動）
    int driveX = 320;
    editor.driveLabel.setBounds(driveX, startY + 28, 75, 16);
    editor.driveSlider.setBounds(driveX, startY + 44, 75, 75);

    // 4. RATIO（MC77風 連結四角形ボタン群）
    int buttonX = 450;
    editor.ratioLabel.setBounds(buttonX - 20, startY - 2, 60, 16);

    int btnW = 24;
    int btnH = 24;
    int btnStartY = startY + 18;

    for (size_t i = 0; i < editor.ratioButtons.size(); ++i)
    {
        editor.ratioButtons[i].setBounds(buttonX, btnStartY + static_cast<int>(i) * btnH, btnW, btnH);
    }
}