#include "PluginEditor.h"

SonicWaveshaperAudioProcessorEditor::SonicWaveshaperAudioProcessorEditor (SonicWaveshaperAudioProcessor& p)
    : AudioProcessorEditor (&p),
      processorRef (p),
      driveAttachment (p.apvts, ParamIDs::drive, driveSlider),
      mixAttachment (p.apvts, ParamIDs::mix, mixSlider),
      outputAttachment (p.apvts, ParamIDs::output, outputSlider),
      shapeAttachment (p.apvts, ParamIDs::shape, shapeBox)
{
    driveSlider.setTextValueSuffix (" dB");
    mixSlider.setTextValueSuffix (" %");
    outputSlider.setTextValueSuffix (" dB");

    shapeBox.addItemList ({ "Tanh", "Soft Cubic", "Hard Clip", "Foldback" }, 1);

    for (auto* l : { &driveLabel, &mixLabel, &outputLabel, &shapeLabel })
    {
        l->setJustificationType (juce::Justification::centred);
        l->setFont (juce::FontOptions (14.0f, juce::Font::bold));
    }

    for (auto* c : { static_cast<juce::Component*> (&driveSlider),
                     static_cast<juce::Component*> (&mixSlider),
                     static_cast<juce::Component*> (&outputSlider),
                     static_cast<juce::Component*> (&shapeBox),
                     static_cast<juce::Component*> (&driveLabel),
                     static_cast<juce::Component*> (&mixLabel),
                     static_cast<juce::Component*> (&outputLabel),
                     static_cast<juce::Component*> (&shapeLabel) })
        addAndMakeVisible (c);

    setSize (560, 320);
}

void SonicWaveshaperAudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colour (0xff1b1d22));

    g.setColour (juce::Colours::white);
    g.setFont (juce::FontOptions (22.0f, juce::Font::bold));
    g.drawText ("SonicWaveshaper", getLocalBounds().removeFromTop (50), juce::Justification::centred);
}

void SonicWaveshaperAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (20);
    area.removeFromTop (50);

    auto shapeArea = area.removeFromTop (60);
    shapeLabel.setBounds (shapeArea.removeFromTop (20));
    shapeBox.setBounds (shapeArea.reduced (120, 4));

    const int knobWidth = area.getWidth() / 3;

    auto driveArea = area.removeFromLeft (knobWidth);
    driveLabel.setBounds (driveArea.removeFromTop (20));
    driveSlider.setBounds (driveArea.reduced (8));

    auto mixArea = area.removeFromLeft (knobWidth);
    mixLabel.setBounds (mixArea.removeFromTop (20));
    mixSlider.setBounds (mixArea.reduced (8));

    outputLabel.setBounds (area.removeFromTop (20));
    outputSlider.setBounds (area.reduced (8));
}
