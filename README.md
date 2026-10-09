# SonicWaveshaper

SonicWaveshaper is a waveshaper audio plugin with a browser edition. It applies drive, four distortion shapes (Tanh, Soft Cubic, Hard Clip, Foldback), dry/wet mix and output trim.

## Tech stack

- JUCE 8.0.6 (C++17) for the VST3, AU and Standalone plugin
- Waveshaping DSP with 2x oversampling (juce::dsp::Oversampling, half-band polyphase IIR)
- ADAA-1 antiderivative antialiasing in the browser edition
- Web Audio API (ScriptProcessorNode) for the browser edition in index.html

## Repository layout

- index.html: standalone browser edition (SW-74 MK II web app). Open it in Chrome or Safari.
- Source/: JUCE plugin source (PluginProcessor, PluginEditor, WaveshaperDSP.h)
- CMakeLists.txt: JUCE plugin build definition
- build.sh: macOS (Apple Silicon) build script

## Building the plugin

Requirements: CMake 3.22+, a C++17 compiler, and Xcode command line tools on macOS (AU needs macOS).

```
bash build.sh
```

Or manually:

```
git clone --depth 1 --branch 8.0.6 https://github.com/juce-framework/JUCE.git
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --config Release
```

JUCE is pulled in with add_subdirectory, so the JUCE folder must sit next to CMakeLists.txt.

## Running the browser edition

Open index.html in a browser and press POWER for the test tone. Choose a source (Kick, Bass, Lead, Sine, Custom or Live In). Use the Speaker Profile buttons to switch to Laptop Punch, which adds high-pass, presence and harmonic exciter processing. Use headphones with LIVE IN to avoid feedback.

## Status

The browser edition is complete. The VST3 download is disabled because no compiled plugin binary is included yet.
