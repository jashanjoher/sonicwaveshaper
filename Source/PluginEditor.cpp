#include "PluginEditor.h"

SonicWaveshaperAudioProcessorEditor::SonicWaveshaperAudioProcessorEditor (SonicWaveshaperAudioProcessor& p)
    : AudioProcessorEditor (&p),
      processorRef (p),
      driveAttachment (p.apvts, ParamIDs::drive, driveSlider),
      mixAttachment (p.apvts, ParamIDs::mix, mixSlider),
      outputAttachment (p.apvts, ParamIDs::output, outputSlider),
      shapeAttachment (p.apvts, ParamIDs::shape, shapeBox)
{
    setLookAndFeel (&lookAndFeel);

    driveSlider.setTextValueSuffix (" dB");
    mixSlider.setTextValueSuffix (" %");
    outputSlider.setTextValueSuffix (" dB");

    shapeBox.addItemList ({ "Tanh", "Soft Cubic", "Hard Clip", "Foldback" }, 1);

    for (auto* l : { &driveLabel, &mixLabel, &outputLabel, &shapeLabel })
    {
        l->setJustificationType (juce::Justification::centred);
        l->setFont (juce::FontOptions (11.0f, juce::Font::bold));
        l->setColour (juce::Label::textColourId, SonicLookAndFeel::ink());
    }

    for (auto* s : { &driveSlider, &mixSlider, &outputSlider })
        s->setColour (juce::Slider::textBoxTextColourId, SonicLookAndFeel::tealDeep());

    for (auto* c : { static_cast<juce::Component*> (&driveSlider),
                     static_cast<juce::Component*> (&mixSlider),
                     static_cast<juce::Component*> (&outputSlider),
                     static_cast<juce::Component*> (&shapeBox),
                     static_cast<juce::Component*> (&driveLabel),
                     static_cast<juce::Component*> (&mixLabel),
                     static_cast<juce::Component*> (&outputLabel),
                     static_cast<juce::Component*> (&shapeLabel) })
        addAndMakeVisible (c);

    setSize (380, 300);
}

SonicWaveshaperAudioProcessorEditor::~SonicWaveshaperAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void SonicWaveshaperAudioProcessorEditor::paint (juce::Graphics& g)
{
    // bubblegum faceplate
    juce::ColourGradient face (SonicLookAndFeel::pinkTop(), 0.0f, 0.0f,
                               SonicLookAndFeel::pinkBottom(), 0.0f, (float) getHeight(), false);
    g.setGradientFill (face);
    g.fillRoundedRectangle (getLocalBounds().toFloat(), 18.0f);

    g.setColour (SonicLookAndFeel::pinkDeep());
    g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (1.5f), 18.0f, 2.5f);

    g.setColour (SonicLookAndFeel::ink());
    g.setFont (juce::FontOptions (18.0f, juce::Font::bold));
    g.drawText ("sonic waveshaper", getLocalBounds().removeFromTop (38), juce::Justification::centred);

    g.setColour (SonicLookAndFeel::tealDeep());
    g.setFont (juce::FontOptions (9.0f, juce::Font::bold));
    g.drawText ("SW-74 MK II  ADAA-1", getLocalBounds().removeFromBottom (18), juce::Justification::centred);
}

void SonicWaveshaperAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (16);
    area.removeFromTop (34);
    area.removeFromBottom (20);

    auto shapeArea = area.removeFromTop (46);
    shapeLabel.setBounds (shapeArea.removeFromTop (16));
    shapeBox.setBounds (shapeArea.reduced (70, 4));

    const int knobWidth = area.getWidth() / 3;

    auto driveArea = area.removeFromLeft (knobWidth);
    driveLabel.setBounds (driveArea.removeFromTop (16));
    driveSlider.setBounds (driveArea.reduced (4));

    auto mixArea = area.removeFromLeft (knobWidth);
    mixLabel.setBounds (mixArea.removeFromTop (16));
    mixSlider.setBounds (mixArea.reduced (4));

    outputLabel.setBounds (area.removeFromTop (16));
    outputSlider.setBounds (area.reduced (4));
}
