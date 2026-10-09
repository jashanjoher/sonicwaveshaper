#!/bin/bash
# Builds SonicWaveshaper on macOS (Apple Silicon). Run from the project folder: bash build.sh
set -e
cd "$(dirname "$0")"
if [ ! -d JUCE ]; then
  git clone --depth 1 --branch 8.0.6 https://github.com/juce-framework/JUCE.git
fi
cmake -B build -G Xcode -DCMAKE_OSX_ARCHITECTURES=arm64
cmake --build build --config Release
