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
        l->setFont (juce::FontOptions (9.0f, juce::Font::bold));
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

    setSize (340, 300);
}

SonicWaveshaperAudioProcessorEditor::~SonicWaveshaperAudioProcessorEditor()
{
    setLookAndFeel (nullptr);
}

void SonicWaveshaperAudioProcessorEditor::paint (juce::Graphics& g)
{
    auto body = getLocalBounds().toFloat().reduced (6.0f);

    // soft drop shadow under the pedal
    g.setColour (juce::Colour (0x40501a33));
    g.fillRoundedRectangle (body.translated (0.0f, 6.0f), 20.0f);

    // bubblegum faceplate
    juce::ColourGradient face (SonicLookAndFeel::pinkTop(), 0.0f, body.getY(),
                               SonicLookAndFeel::pinkBottom(), 0.0f, body.getBottom(), false);
    g.setGradientFill (face);
    g.fillRoundedRectangle (body, 20.0f);

    g.setColour (SonicLookAndFeel::pinkDeep());
    g.drawRoundedRectangle (body.reduced (1.0f), 20.0f, 2.0f);

    g.setColour (SonicLookAndFeel::ink());
    g.setFont (juce::FontOptions (16.0f, juce::Font::bold));
    g.drawText ("sonic waveshaper", getLocalBounds().removeFromTop (34), juce::Justification::centred);

    g.setColour (SonicLookAndFeel::tealDeep());
    g.setFont (juce::FontOptions (8.0f, juce::Font::bold));
    g.drawText ("SW-74 MK II  ADAA-1", getLocalBounds().removeFromBottom (16), juce::Justification::centred);
}

void SonicWaveshaperAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (16);
    area.removeFromTop (30);
    area.removeFromBottom (18);

    auto shapeArea = area.removeFromBottom (48);
    shapeLabel.setBounds (shapeArea.removeFromTop (14));
    shapeBox.setBounds (shapeArea.reduced (60, 4));

    // small, balanced knob row: saturation is the centre knob and is only slightly larger
    const int colW = area.getWidth() / 3;

    auto mixArea = area.removeFromLeft (colW);
    mixLabel.setBounds (mixArea.removeFromTop (14));
    mixSlider.setBounds (mixArea.reduced (2));

    auto satArea = area.removeFromLeft (colW);
    driveLabel.setBounds (satArea.removeFromTop (14));
    driveSlider.setBounds (satArea.reduced (0, 0).withSizeKeepingCentre (juce::jmin (satArea.getWidth(), 120), satArea.getHeight()));

    outputLabel.setBounds (area.removeFromTop (14));
    outputSlider.setBounds (area.reduced (2));
}
