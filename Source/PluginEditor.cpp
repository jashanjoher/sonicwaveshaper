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
        l->setFont (juce::FontOptions (13.0f, juce::Font::bold));
        l->setColour (juce::Label::textColourId, SonicLookAndFeel::ink());
    }

    for (auto* s : { &driveSlider, &mixSlider, &outputSlider })
        s->setColour (juce::Slider::textBoxTextColourId, SonicLookAndFeel::ink());

    for (auto* c : { static_cast<juce::Component*> (&driveSlider),
                     static_cast<juce::Component*> (&mixSlider),
                     static_cast<juce::Component*> (&outputSlider),
                     static_cast<juce::Component*> (&shapeBox),
                     static_cast<juce::Component*> (&driveLabel),
                     static_cast<juce::Component*> (&mixLabel),
                     static_cast<juce::Component*> (&outputLabel),
                     static_cast<juce::Component*> (&shapeLabel) })
        addAndMakeVisible (c);

    setSize (600, 360);
}

SonicWaveshaperAudioProcessorEditor::~SonicWaveshaperAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void SonicWaveshaperAudioProcessorEditor::paint (juce::Graphics& g)
{
    // matte cream faceplate with recessed chassis edge
    juce::ColourGradient face (SonicLookAndFeel::faceplateTop(), 0.0f, 0.0f,
                               SonicLookAndFeel::faceplateBottom(), 0.0f, (float) getHeight(), false);
    g.setGradientFill (face);
    g.fillAll();

    g.setColour (juce::Colour (0xff2a2c2f));
    g.drawRect (getLocalBounds(), 3);

    g.setColour (SonicLookAndFeel::ink());
    g.setFont (juce::FontOptions (20.0f, juce::Font::bold));
    g.drawText ("SONIC AUDIO LABS  //  SW-74 MK II", getLocalBounds().removeFromTop (46), juce::Justification::centred);

    g.setColour (SonicLookAndFeel::accent());
    g.setFont (juce::FontOptions (11.0f, juce::Font::bold));
    g.drawText ("ADAA-1 WAVESHAPER", getLocalBounds().removeFromBottom (22), juce::Justification::centred);
}

void SonicWaveshaperAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (20);
    area.removeFromTop (46);
    area.removeFromBottom (18);

    auto shapeArea = area.removeFromTop (58);
    shapeLabel.setBounds (shapeArea.removeFromTop (18));
    shapeBox.setBounds (shapeArea.reduced (120, 6));

    area.removeFromTop (6);
    const int knobWidth = area.getWidth() / 3;

    auto driveArea = area.removeFromLeft (knobWidth);
    driveLabel.setBounds (driveArea.removeFromTop (18));
    driveSlider.setBounds (driveArea.reduced (8));

    auto mixArea = area.removeFromLeft (knobWidth);
    mixLabel.setBounds (mixArea.removeFromTop (18));
    mixSlider.setBounds (mixArea.reduced (8));

    outputLabel.setBounds (area.removeFromTop (18));
    outputSlider.setBounds (area.reduced (8));
}
