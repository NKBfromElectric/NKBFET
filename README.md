# NKB FET

FET-style Compressor Audio Plugin developed with C++ and JUCE framework.

## Overview
**NKB FET** is an FET-style compressor plugin inspired by the fast response and sound character of the classic UREI 1176. It features ultra-fast attack/release times, a distinct punchy analog character, and a modern, high-precision digital Gain Reduction display.

## Features
- **FET Compressor Emulation**: Ultra-fast attack time and dynamic response.
- **Modern Digital GR Display**: High-visibility digital Gain Reduction meter and dynamic response bar.
- **JUCE Framework**: Lightweight and cross-platform audio processing in C++.
- **Simple & Intuitive Controls**: Streamlined parameters for precise and fast tone shaping.

## Requirements & Environment
- **Framework**: JUCE (v7 / v8)
- **Language**: C++17 / C++20
- **Supported Formats**: VST3 / AU / Standalone

## Project Structure
- `Source/` : C++ source code (Processor, Editor)
- `NKBFET.jucer` : Projucer project file
- `README.md` : Documentation

## How to Build
1. Install [JUCE](https://juce.com/).
2. Open `NKBFET.jucer` using Projucer.
3. Select your IDE exporter (e.g., Visual Studio / Xcode), open the project, and build.

## License
This project is open-source. Feel free to use and inspect the code for educational and development purposes.
