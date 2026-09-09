# NKB1176

1176-style FET Compressor Audio Plugin developed with C++ and JUCE framework.

## Overview
**NKB1176** は、名機 UREI 1176 の挙動やサウンド特性に着想を得て制作されたFETスタイルのコンプレッサー・プラグインです。高速なアタック/リリースタイムと、特有の押し出し感のあるディストーション・キャラクターを特徴としています。

## Features
- **FET Compressor Emulation**: 高速なアタックとダイナミックなレスポンス
- **JUCE Framework**: C++ / JUCE による軽量かつクロスプラットフォームなオーディオ処理
- **Simple Controls**: 直感的な操作が可能なパラメーター設計

## Requirements & Environment
- **Framework**: JUCE (v7 / v8)
- **Language**: C++17 / C++20
- **Supported Formats**: VST3 / AU / Standalone

## Project Structure
```text
.
├── Source/              # C++ ソースコード (Processor, Editor)
├── NKB1176.jucer        # Projucer プロジェクトファイル
└── README.md            # ドキュメント

## How to Build
1. [JUCE](https://juce.com/) をインストールします。
2. `NKB1176.jucer` を Projucer で開きます。
3. お使いのIDE（Visual Studio / Xcode など）のエクスポート設定を選択し、プロジェクトを開いてビルドを実行します。

## License
This project is open-source. Feel free to use and inspect the code for educational and development purposes.
