#include "PluginProcessor.h"
#include "PluginEditor.h"

SonicWaveshaperAudioProcessor::SonicWaveshaperAudioProcessor()
    : AudioProcessor (BusesProperties()
                        .withInput ("Input", juce::AudioChannelSet::stereo(), true)
                        .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      apvts (*this, nullptr, "PARAMETERS", createParameterLayout())
{
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

void SonicWaveshaperAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    dsp.prepare (sampleRate, samplesPerBlock, juce::jmax (1, getTotalNumOutputChannels()));
    setLatencySamples (dsp.getLatencySamples());
}

void SonicWaveshaperAudioProcessor::releaseResources()
{
    dsp.reset();
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

    const float driveDb = apvts.getRawParameterValue (ParamIDs::drive)->load();
    const int shapeIndex = juce::roundToInt (apvts.getRawParameterValue (ParamIDs::shape)->load());
    const float mixPercent = apvts.getRawParameterValue (ParamIDs::mix)->load();
    const float outputDb = apvts.getRawParameterValue (ParamIDs::output)->load();

    dsp.setParameters (juce::Decibels::decibelsToGain (driveDb),
                       static_cast<WaveshaperDSP::Shape> (juce::jlimit (0, 3, shapeIndex)),
                       mixPercent / 100.0f,
                       juce::Decibels::decibelsToGain (outputDb));

    dsp.process (buffer);
}

juce::AudioProcessorEditor* SonicWaveshaperAudioProcessor::createEditor()
{
    return new SonicWaveshaperAudioProcessorEditor (*this);
}

void SonicWaveshaperAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    auto state = apvts.copyState();
    std::unique_ptr<juce::XmlElement> xml (state.createXml());
    copyXmlToBinary (*xml, destData);
}

void SonicWaveshaperAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xmlState (getXmlFromBinary (data, sizeInBytes));

    if (xmlState != nullptr && xmlState->hasTagName (apvts.state.getType()))
        apvts.replaceState (juce::ValueTree::fromXml (*xmlState));
}

// Required entry point for the plugin wrappers (fixes createPluginFilter link errors).
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new SonicWaveshaperAudioProcessor();
}
