#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include "PluginProcessor.h"

class SonicWaveshaperAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit SonicWaveshaperAudioProcessorEditor (SonicWaveshaperAudioProcessor&);
    ~SonicWaveshaperAudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    SonicWaveshaperAudioProcessor& processorRef;

    juce::Slider driveSlider { juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow };
    juce::Slider mixSlider { juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow };
    juce::Slider outputSlider { juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxBelow };
    juce::ComboBox shapeBox;

    juce::Label driveLabel { {}, "Drive" };
    juce::Label mixLabel { {}, "Mix" };
    juce::Label outputLabel { {}, "Output" };
    juce::Label shapeLabel { {}, "Shape" };

    using SliderAttachment = juce::AudioProcessorValueTreeState::SliderAttachment;
    using ComboAttachment = juce::AudioProcessorValueTreeState::ComboBoxAttachment;

    SliderAttachment driveAttachment;
    SliderAttachment mixAttachment;
    SliderAttachment outputAttachment;
    ComboAttachment shapeAttachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SonicWaveshaperAudioProcessorEditor)
};
