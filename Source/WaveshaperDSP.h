#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <cmath>
#include <memory>

// Header-only waveshaper DSP with 2x oversampling.
class WaveshaperDSP
{
public:
    enum class Shape : int { Tanh = 0, SoftCubic, HardClip, Foldback };

    void prepare (double sampleRate, int maxBlockSize, int numChannels)
    {
        juce::ignoreUnused (sampleRate);

        oversampling = std::make_unique<juce::dsp::Oversampling<float>> (
            (size_t) juce::jmax (1, numChannels),
            1, // factor 1 => 2^1 = 2x oversampling
            juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
            true,
            false);

        oversampling->initProcessing ((size_t) juce::jmax (1, maxBlockSize));
        oversampling->reset();

        dryBuffer.setSize (juce::jmax (1, numChannels), juce::jmax (1, maxBlockSize));
        dryBuffer.clear();

        latencySamples = (int) oversampling->getLatencyInSamples();
    }

    void reset()
    {
        if (oversampling != nullptr)
            oversampling->reset();
    }

    int getLatencySamples() const noexcept { return latencySamples; }

    void setParameters (float newDriveLinear, Shape newShape, float newMix01, float newOutputLinear) noexcept
    {
        driveLinear = newDriveLinear;
        shape = newShape;
        mix = juce::jlimit (0.0f, 1.0f, newMix01);
        outputLinear = newOutputLinear;
    }

    void process (juce::AudioBuffer<float>& buffer)
    {
        if (oversampling == nullptr)
            return;

        const int numChannels = buffer.getNumChannels();
        const int numSamples = buffer.getNumSamples();

        if (numChannels == 0 || numSamples == 0)
            return;

        // Make sure scratch storage is large enough (only reallocates if the host exceeds prepared size).
        if (dryBuffer.getNumChannels() < numChannels || dryBuffer.getNumSamples() < numSamples)
            dryBuffer.setSize (numChannels, numSamples, false, false, true);

        for (int ch = 0; ch < numChannels; ++ch)
            dryBuffer.copyFrom (ch, 0, buffer, ch, 0, numSamples);

        juce::dsp::AudioBlock<float> block (buffer);
        auto upsampled = oversampling->processSamplesUp (block);

        for (size_t ch = 0; ch < upsampled.getNumChannels(); ++ch)
        {
            auto* data = upsampled.getChannelPointer (ch);

            for (size_t i = 0; i < upsampled.getNumSamples(); ++i)
                data[i] = shapeSample (data[i] * driveLinear);
        }

        oversampling->processSamplesDown (block);

        // Dry/wet mix and output gain
        const float dryGain = 1.0f - mix;
        const float wetGain = mix * outputLinear;

        for (int ch = 0; ch < numChannels; ++ch)
        {
            auto* out = buffer.getWritePointer (ch);
            const auto* dry = dryBuffer.getReadPointer (ch);

            for (int i = 0; i < numSamples; ++i)
                out[i] = dry[i] * dryGain + out[i] * wetGain;
        }
    }

private:
    float shapeSample (float x) const noexcept
    {
        switch (shape)
        {
            case Shape::Tanh:
                return std::tanh (x);

            case Shape::SoftCubic:
            {
                const float y = juce::jlimit (-1.0f, 1.0f, x);
                return 1.5f * y - 0.5f * y * y * y;
            }

            case Shape::HardClip:
                return juce::jlimit (-1.0f, 1.0f, x);

            case Shape::Foldback:
            {
                // Triangle fold into [-1, 1] with period 4
                float y = std::fmod (x + 1.0f, 4.0f);
                if (y < 0.0f)
                    y += 4.0f;
                return y <= 2.0f ? y - 1.0f : 3.0f - y;
            }
        }

        return x;
    }

    std::unique_ptr<juce::dsp::Oversampling<float>> oversampling;
    juce::AudioBuffer<float> dryBuffer;

    float driveLinear = 1.0f;
    float outputLinear = 1.0f;
    float mix = 1.0f;
    Shape shape = Shape::Tanh;
    int latencySamples = 0;
};
