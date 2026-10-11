#pragma once

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_dsp/juce_dsp.h>
#include <array>
#include <cmath>
#include <memory>

// Built-in multi-stage triode preamp saturation (Preamp 8000 profile).
//
// Signal path per sample, at 2x oversampling:
//   1. Input gain (Saturation knob drives this) and a bias offset that shifts the operating point.
//   2. Three cascaded triode-style stages. Each stage applies an asymmetric soft clip (positive and negative
//      half-cycles clip differently, which produces even harmonics) and an RC-style DC blocker between stages.
//   3. First-order ADAA (antiderivative antialiasing) on each stage's nonlinearity, so the clipper's
//      high-frequency content is reduced without an extra filter.
//   4. Dry/wet mix and output compensation.
//
// No external model weights or files are needed.
class WaveshaperDSP
{
public:
    // Kept as the four shape IDs so the existing parameter layout still works.
    // Shapes select the stage clipper: Tanh = classic triode, SoftCubic = warm, HardClip = hard, Foldback = fold.
    enum class Shape : int { Tanh = 0, SoftCubic, HardClip, Foldback };

    void prepare (double sampleRate, int maxBlockSize, int numChannels)
    {
        juce::ignoreUnused (sampleRate);

        numChans = juce::jlimit (1, 2, numChannels);

        oversampling = std::make_unique<juce::dsp::Oversampling<float>> (
            (size_t) numChans,
            1, // factor 1 => 2x oversampling
            juce::dsp::Oversampling<float>::filterHalfBandPolyphaseIIR,
            true,
            false);

        oversampling->initProcessing ((size_t) juce::jmax (1, maxBlockSize));
        oversampling->reset();

        dryBuffer.setSize (numChans, juce::jmax (1, maxBlockSize));
        dryBuffer.clear();

        resetState();
        latencySamples = (int) oversampling->getLatencyInSamples();
    }

    void reset()
    {
        if (oversampling != nullptr)
            oversampling->reset();
        resetState();
    }

    int getLatencySamples() const noexcept { return latencySamples; }

    void setParameters (float newDriveLinear, Shape newShape, float newMix01, float newOutputLinear) noexcept
    {
        driveLinear = juce::jmax (0.0f, newDriveLinear);
        shape = newShape;
        mix = juce::jlimit (0.0f, 1.0f, newMix01);
        outputLinear = newOutputLinear;
    }

    void process (juce::AudioBuffer<float>& buffer)
    {
        if (oversampling == nullptr)
            return;

        const int numChannels = juce::jmin (buffer.getNumChannels(), numChans);
        const int numSamples = buffer.getNumSamples();

        if (numChannels == 0 || numSamples == 0)
            return;

        if (dryBuffer.getNumChannels() < numChannels || dryBuffer.getNumSamples() < numSamples)
            dryBuffer.setSize (numChannels, numSamples, false, false, true);

        for (int ch = 0; ch < numChannels; ++ch)
            dryBuffer.copyFrom (ch, 0, buffer, ch, 0, numSamples);

        juce::dsp::AudioBlock<float> block (buffer);
        auto upsampled = oversampling->processSamplesUp (block);

        const float gainIn = driveLinear;

        for (size_t ch = 0; ch < upsampled.getNumChannels(); ++ch)
        {
            auto* data = upsampled.getChannelPointer (ch);
            auto& st = channelState[juce::jmin ((int) ch, 1)];

            for (size_t i = 0; i < upsampled.getNumSamples(); ++i)
                data[i] = processPreamp (data[i] * gainIn, st);
        }

        oversampling->processSamplesDown (block);

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
    struct ChannelState
    {
        // ADAA-1 memory for each of the three stages: previous input and previous antiderivative.
        std::array<float, 3> prevX { 0.0f, 0.0f, 0.0f };
        std::array<float, 3> prevF { 0.0f, 0.0f, 0.0f };
        // DC blocker memory between stages.
        std::array<float, 2> dcX { 0.0f, 0.0f };
        std::array<float, 2> dcY { 0.0f, 0.0f };
    };

    void resetState() noexcept
    {
        for (auto& st : channelState)
            st = ChannelState {};
    }

    // Clipper nonlinearity shared by all stages. Asymmetric through the bias term.
    static float clipFn (float x, Shape s) noexcept
    {
        switch (s)
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
                float y = std::fmod (x + 1.0f, 4.0f);
                if (y < 0.0f)
                    y += 4.0f;
                return y <= 2.0f ? y - 1.0f : 3.0f - y;
            }
        }
        return x;
    }

    // Antiderivative of clipFn for the ADAA-1 step. Used for Tanh and HardClip and Cubic.
    static float clipAntiderivative (float x, Shape s) noexcept
    {
        switch (s)
        {
            case Shape::Tanh:
            {
                const float a = std::abs (x);
                return a + std::log (1.0f + std::exp (-2.0f * a)) - 0.69314718f;
            }
            case Shape::SoftCubic:
            {
                const float a = juce::jlimit (-1.0f, 1.0f, x);
                return 0.75f * a * a - 0.125f * a * a * a * a;
            }
            case Shape::HardClip:
            {
                const float a = std::abs (x);
                return a <= 1.0f ? 0.5f * a * a : a - 0.5f;
            }
            case Shape::Foldback:
                return 0.0f; // fold is not ADAA-smoothed; handled as plain waveshaping below
        }
        return 0.0f;
    }

    // One triode-style stage: bias shift, ADAA-1 clip, and asymmetric even-harmonic term.
    float triodeStage (float x, int stage, float& prevX, float& prevF) noexcept
    {
        // Bias: a small DC offset moves the operating point, so the positive and negative swings clip differently.
        const float bias = (stage == 0) ? 0.08f : 0.04f;
        const float xb = x + bias;

        float y;
        if (shape == Shape::Foldback)
        {
            y = clipFn (xb, Shape::Foldback);
        }
        else
        {
            // First-order ADAA: average of the antiderivative difference.
            const float F = clipAntiderivative (xb, shape);
            const float dx = xb - prevX;
            if (std::abs (dx) < 1.0e-5f)
                y = clipFn ((xb + prevX) * 0.5f, shape);
            else
                y = (F - prevF) / dx;
            prevX = xb;
            prevF = F;
        }

        // Remove the bias again so stages do not drift.
        const float bias2 = bias * 0.5f;
        return y - bias2;
    }

    float processPreamp (float x, ChannelState& st) noexcept
    {
        // Stage 1: input triode, strongest asymmetry.
        float y = triodeStage (x, 0, st.prevX[0], st.prevF[0]);

        // DC blocker between stages (first-order high-pass).
        y = dcBlock (y, 0, st);

        // Stage 2: second gain stage, tighter clip.
        y = triodeStage (y * 0.85f, 1, st.prevX[1], st.prevF[1]);

        y = dcBlock (y, 1, st);

        // Stage 3: output triode, mild asymmetry.
        y = triodeStage (y * 0.9f, 2, st.prevX[2], st.prevF[2]);

        return y;
    }

    float dcBlock (float x, int idx, ChannelState& st) noexcept
    {
        constexpr float R = 0.995f;
        const float y = x - st.dcX[idx] + R * st.dcY[idx];
        st.dcX[idx] = x;
        st.dcY[idx] = y;
        return y;
    }

    std::unique_ptr<juce::dsp::Oversampling<float>> oversampling;
    juce::AudioBuffer<float> dryBuffer;
    std::array<ChannelState, 2> channelState {};

    int numChans = 2;
    float driveLinear = 1.0f;
    float outputLinear = 1.0f;
    float mix = 1.0f;
    Shape shape = Shape::Tanh;
    int latencySamples = 0;
};
