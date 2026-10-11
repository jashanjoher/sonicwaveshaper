#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>

#include <array>
#include <memory>
#include <vector>

#include "NAM/get_dsp.h"
#include "WaveshaperDSP.h"

namespace ParamIDs
{
    inline constexpr const char* drive  = "drive";
    inline constexpr const char* shape  = "shape";
    inline constexpr const char* mix    = "mix";
    inline constexpr const char* output = "output";
}

class SonicWaveshaperAudioProcessor : public juce::AudioProcessor
{
public:
    SonicWaveshaperAudioProcessor();
    ~SonicWaveshaperAudioProcessor() override = default;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return "SonicWaveshaper"; }
    bool acceptsMidi() const override { return false; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts;

    // Loads a user-selected .nam file. If it is missing, empty, or invalid, the embedded default is restored
    // and the returned Result carries the reason.
    juce::Result loadCustomNamModel (const juce::File& file);

    // Name shown in the UI: "Default: WaveNet A1" or the loaded file's name.
    juce::String getActiveModelName() const { return activeModelName; }

private:
    using NamPair = std::array<std::unique_ptr<nam::DSP>, 2>;

    static juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    bool loadEmbeddedNamModel();
    void resetModels (NamPair& models) const;
    void installNamModels (NamPair&& models, const juce::String& name);

    // Guards namModels. processBlock only try-locks it, so it never waits on a model swap.
    juce::SpinLock namLock;
    NamPair namModels;

    double currentSampleRate = 0.0;
    juce::String activeModelName;
    juce::String customModelPath;

    // Scratch storage sized in prepareToPlay so processBlock does not allocate.
    // NAM_SAMPLE is float (NAM_SAMPLE_FLOAT), so these must be float to match nam::DSP::process().
    int preparedBlockSize = 0;
    juce::AudioBuffer<float> dryBuffer;
    std::vector<float> namInput[2];
    std::vector<float> namOutput[2];

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SonicWaveshaperAudioProcessor)
};
