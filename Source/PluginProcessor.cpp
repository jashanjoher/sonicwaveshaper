#include "PluginProcessor.h"
#include "PluginEditor.h"
#include "EmbeddedModel.h"

namespace
{
    constexpr const char* kCustomModelPathKey = "customNamModelPath";
    constexpr const char* kDefaultModelName = "Default: WaveNet A1";
}

SonicWaveshaperAudioProcessor::SonicWaveshaperAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
    loadEmbeddedNamModel();
}

juce::AudioProcessorValueTreeState::ParameterLayout SonicWaveshaperAudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> params;

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::drive, 1 }, "Drive",
        juce::NormalisableRange<float> (0.0f, 24.0f, 0.01f), 6.0f));

    params.push_back (std::make_unique<juce::AudioParameterChoice> (
        juce::ParameterID { ParamIDs::shape, 1 }, "Shape",
        juce::StringArray { "Tanh", "Soft Cubic", "Hard Clip", "Foldback" }, 0));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::mix, 1 }, "Mix",
        juce::NormalisableRange<float> (0.0f, 100.0f, 0.1f), 100.0f));

    params.push_back (std::make_unique<juce::AudioParameterFloat> (
        juce::ParameterID { ParamIDs::output, 1 }, "Output",
        juce::NormalisableRange<float> (-24.0f, 12.0f, 0.01f), 0.0f));

    return { params.begin(), params.end() };
}

void SonicWaveshaperAudioProcessor::resetModels (NamPair& models) const
{
    if (currentSampleRate <= 0.0 || preparedBlockSize <= 0)
        return;

    for (auto& model : models)
        if (model != nullptr)
            model->Reset (currentSampleRate, preparedBlockSize);
}

// Swaps the active models under the lock. The retired models are destroyed after the lock is released.
void SonicWaveshaperAudioProcessor::installNamModels (NamPair&& models, const juce::String& name)
{
    NamPair retired;

    {
        const juce::SpinLock::ScopedLockType lock (namLock);
        retired = std::move (namModels);
        namModels = std::move (models);
    }

    activeModelName = name;
}

bool SonicWaveshaperAudioProcessor::loadEmbeddedNamModel()
{
    NamPair models;

    try
    {
        const std::string modelText (reinterpret_cast<const char*> (kWavenetA1Model), kWavenetA1ModelSize);
        const nlohmann::json config = nlohmann::json::parse (modelText);

        for (auto& model : models)
            model = nam::get_dsp (config);
    }
    catch (const std::exception& e)
    {
        DBG ("Embedded NAM model load failed: " << e.what());
        installNamModels (NamPair{}, "Default: WaveNet A1 (unavailable)");
        return false;
    }

    resetModels (models);
    installNamModels (std::move (models), kDefaultModelName);
    return true;
}

juce::Result SonicWaveshaperAudioProcessor::loadCustomNamModel (const juce::File& file)
{
    auto fallBack = [this] (const juce::String& reason)
    {
        loadEmbeddedNamModel();
        customModelPath.clear();
        return juce::Result::fail (reason + " The default model is in use.");
    };

    if (! file.existsAsFile() || file.getSize() == 0)
        return fallBack ("The selected file is missing or empty.");

    NamPair models;

    try
    {
        const nlohmann::json config = nlohmann::json::parse (file.loadFileAsString().toStdString());

        for (auto& model : models)
            model = nam::get_dsp (config);
    }
    catch (const std::exception& e)
    {
        return fallBack (file.getFileName() + " could not be loaded (" + e.what() + ").");
    }

    if (models[0] == nullptr || models[1] == nullptr)
        return fallBack (file.getFileName() + " did not produce a usable model.");

    resetModels (models);
    installNamModels (std::move (models), file.getFileNameWithoutExtension());
    customModelPath = file.getFullPathName();

    return juce::Result::ok();
}

void SonicWaveshaperAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    currentSampleRate = sampleRate;
    preparedBlockSize = juce::jmax (1, samplesPerBlock);

    dryBuffer.setSize (2, preparedBlockSize);
    dryBuffer.clear();

    for (int ch = 0; ch < 2; ++ch)
    {
        namInput[ch].assign ((size_t) preparedBlockSize, 0.0);
        namOutput[ch].assign ((size_t) preparedBlockSize, 0.0);
    }

    {
        const juce::SpinLock::ScopedLockType lock (namLock);

        for (auto& model : namModels)
            if (model != nullptr)
                model->Reset (sampleRate, preparedBlockSize);
    }

    // The NAM wavenet adds no fixed latency.
    setLatencySamples (0);
}

void SonicWaveshaperAudioProcessor::releaseResources()
{
}

bool SonicWaveshaperAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    const auto mainOut = layouts.getMainOutputChannelSet();

    if (mainOut != juce::AudioChannelSet::stereo() && mainOut != juce::AudioChannelSet::mono())
        return false;

    return layouts.getMainInputChannelSet() == mainOut;
}

void SonicWaveshaperAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    const auto totalNumInputChannels = getTotalNumInputChannels();
    const auto totalNumOutputChannels = getTotalNumOutputChannels();

    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear (i, 0, buffer.getNumSamples());

    const int numChannels = juce::jmin (buffer.getNumChannels(), 2);
    const int numSamples = buffer.getNumSamples();

    if (preparedBlockSize <= 0 || numChannels == 0 || numSamples == 0)
        return;

    // If a model swap is in progress, pass audio through unchanged for this block instead of waiting.
    const juce::SpinLock::ScopedTryLockType tryLock (namLock);
    if (! tryLock.isLocked())
        return;

    // If no model is available, pass audio through unchanged.
    if (namModels[0] == nullptr || namModels[1] == nullptr)
        return;

    // Saturation (Drive) sets the input gain into the neural model.
    const float driveDb = apvts.getRawParameterValue (ParamIDs::drive)->load();
    const float mixPercent = apvts.getRawParameterValue (ParamIDs::mix)->load();
    const float outputDb = apvts.getRawParameterValue (ParamIDs::output)->load();

    const float inputGain = juce::Decibels::decibelsToGain (driveDb);
    const float mix = juce::jlimit (0.0f, 1.0f, mixPercent / 100.0f);
    const float dryGain = 1.0f - mix;
    const float wetGain = mix * juce::Decibels::decibelsToGain (outputDb);

    // Process in chunks no larger than the size prepared in prepareToPlay.
    for (int start = 0; start < numSamples; start += preparedBlockSize)
    {
        const int n = juce::jmin (preparedBlockSize, numSamples - start);

        for (int ch = 0; ch < numChannels; ++ch)
        {
            float* channelData = buffer.getWritePointer (ch, start);
            dryBuffer.copyFrom (ch, 0, buffer, ch, start, n);

            auto& in = namInput[ch];
            auto& out = namOutput[ch];

            for (int i = 0; i < n; ++i)
                in[(size_t) i] = (double) channelData[i] * (double) inputGain;

            double* inPtr = in.data();
            double* outPtr = out.data();
            namModels[ch]->process (&inPtr, &outPtr, n);

            const float* dry = dryBuffer.getReadPointer (ch);

            for (int i = 0; i < n; ++i)
                channelData[i] = dry[i] * dryGain + (float) outPtr[i] * wetGain;
        }
    }
}

juce::AudioProcessorEditor* SonicWaveshaperAudioProcessor::createEditor()
{
    return new SonicWaveshaperAudioProcessorEditor (*this);
}

void SonicWaveshaperAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    state.setProperty (kCustomModelPathKey, customModelPath, nullptr);

    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void SonicWaveshaperAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState == nullptr || ! xmlState->hasTagName (apvts.state.getType()))
        return;

    const auto restored = juce::ValueTree::fromXml (*xmlState);
    const juce::String savedPath = restored.getProperty (kCustomModelPathKey).toString();

    apvts.replaceState (restored);

    if (savedPath.isNotEmpty())
    {
        const auto result = loadCustomNamModel (juce::File (savedPath));

        if (result.failed())
            DBG ("Saved NAM profile not restored: " << result.getErrorMessage());
    }
    else if (customModelPath.isNotEmpty())
    {
        loadEmbeddedNamModel();
        customModelPath.clear();
    }
}

// Required entry point for the plugin wrappers (fixes createPluginFilter link errors).
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SonicWaveshaperAudioProcessor();
}
