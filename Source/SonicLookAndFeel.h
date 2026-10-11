#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Analog tabletop / stompbox look: matte cream faceplate, chunky fluted knobs with a white indicator slit,
// recessed bezels and tactile rounded buttons. Used by both the editor and any JUCE component.
class SonicLookAndFeel : public juce::LookAndFeel_V4
{
public:
    SonicLookAndFeel()
    {
        setColour (juce::Slider::textBoxTextColourId, juce::Colour (0xff1c1d1f));
        setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0xffd8d4cb));
        setColour (juce::Slider::textBoxOutlineColourId, juce::Colour (0x00000000));
        setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xfff7f5f0));
        setColour (juce::ComboBox::textColourId, juce::Colour (0xff1c1d1f));
        setColour (juce::ComboBox::outlineColourId, juce::Colour (0x00000000));
        setColour (juce::ComboBox::arrowColourId, juce::Colour (0xffe8503a));
        setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xfff6f4ee));
        setColour (juce::PopupMenu::textColourId, juce::Colour (0xff1c1d1f));
        setColour (juce::PopupMenu::highlightedBackgroundColourId, juce::Colour (0xffe8503a));
    }

    static juce::Colour faceplateTop()    { return juce::Colour (0xfff6f4ee); }
    static juce::Colour faceplateBottom() { return juce::Colour (0xffcfcbc2); }
    static juce::Colour accent()          { return juce::Colour (0xffe8503a); }
    static juce::Colour ink()             { return juce::Colour (0xff1c1d1f); }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider& slider) override
    {
        const auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (6.0f);
        const float d = juce::jmin (bounds.getWidth(), bounds.getHeight());
        const auto centre = bounds.getCentre();
        const float r = d * 0.5f;
        const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // recessed well
        auto well = juce::Rectangle<float> (d + 12.0f, d + 12.0f).withCentre (centre);
        g.setColour (juce::Colour (0xffb9b5ab));
        g.fillEllipse (well);

        // drop shadow
        g.setColour (juce::Colours::black.withAlpha (0.45f));
        g.fillEllipse (juce::Rectangle<float> (d, d).withCentre (centre + juce::Point<float> (0.0f, 4.0f)));

        // fluted cap: alternating ridges around the circumference
        const int flutes = 36;
        for (int i = 0; i < flutes; ++i)
        {
            const float a0 = juce::MathConstants<float>::twoPi * (float) i / (float) flutes;
            const float a1 = a0 + juce::MathConstants<float>::twoPi / (float) (flutes * 2);
            juce::Path wedge;
            wedge.startNewSubPath (centre);
            wedge.lineTo (centre.x + r * std::cos (a0), centre.y + r * std::sin (a0));
            wedge.lineTo (centre.x + r * std::cos (a1), centre.y + r * std::sin (a1));
            wedge.closeSubPath();
            g.setColour ((i % 2 == 0) ? juce::Colour (0xff1d1f22) : juce::Colour (0xff30343a));
            g.fillPath (wedge);
        }

        // smooth top face
        const float inner = d * 0.72f;
        auto face = juce::Rectangle<float> (inner, inner).withCentre (centre);
        juce::ColourGradient grad (juce::Colour (0xff5a6068), face.getX() + inner * 0.3f, face.getY() + inner * 0.2f,
                                   juce::Colour (0xff1a1c1f), face.getRight(), face.getBottom(), true);
        g.setGradientFill (grad);
        g.fillEllipse (face);

        // white indicator slit
        juce::Path slit;
        slit.addRoundedRectangle (-2.5f, -(r * 0.85f), 5.0f, r * 0.42f, 2.0f);
        g.setColour (juce::Colours::white);
        g.fillPath (slit, juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));

        // hairline outline
        g.setColour (juce::Colours::black);
        g.drawEllipse (juce::Rectangle<float> (d, d).withCentre (centre), 1.0f);
    }

    void drawComboBox (juce::Graphics& g, int width, int height, bool /*isButtonDown*/,
                       int, int, int, int, juce::ComboBox& box) override
    {
        auto bounds = juce::Rectangle<float> (0.0f, 0.0f, (float) width, (float) height).reduced (1.0f);
        g.setColour (box.findColour (juce::ComboBox::backgroundColourId));
        g.fillRoundedRectangle (bounds, 10.0f);
        g.setColour (accent());
        auto arrow = juce::Rectangle<float> (12.0f, 8.0f).withCentre ({ bounds.getRight() - 16.0f, bounds.getCentreY() });
        juce::Path p;
        p.addTriangle (arrow.getX(), arrow.getY(), arrow.getRight(), arrow.getY(), arrow.getCentreX(), arrow.getBottom());
        g.fillPath (p);
    }

    void drawLabel (juce::Graphics& g, juce::Label& label) override
    {
        g.fillAll (label.findColour (juce::Label::backgroundColourId));
        g.setColour (ink());
        g.setFont (getLabelFont (label));
        g.drawText (label.getText(), label.getLocalBounds().reduced (2, 0), juce::Justification::centred, false);
    }
};
