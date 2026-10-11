#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

// Compact pink and turquoise pedal look: bubblegum faceplate, small fluted turquoise knobs with a white
// indicator slit, soft ambient occlusion and drop shadows.
class SonicLookAndFeel : public juce::LookAndFeel_V4
{
public:
    SonicLookAndFeel()
    {
        setColour (juce::ComboBox::backgroundColourId, juce::Colour (0xfffff2f8));
        setColour (juce::ComboBox::textColourId, ink());
        setColour (juce::ComboBox::outlineColourId, juce::Colour (0x00000000));
        setColour (juce::ComboBox::arrowColourId, teal());
        setColour (juce::PopupMenu::backgroundColourId, juce::Colour (0xfffff2f8));
        setColour (juce::PopupMenu::textColourId, ink());
        setColour (juce::PopupMenu::highlightedBackgroundColourId, teal());
        setColour (juce::Slider::textBoxTextColourId, tealDeep());
        setColour (juce::Slider::textBoxBackgroundColourId, juce::Colour (0x00000000));
        setColour (juce::Slider::textBoxOutlineColourId, juce::Colour (0x00000000));
    }

    static juce::Colour pinkTop()    { return juce::Colour (0xfffff2f8); }
    static juce::Colour pinkBottom() { return juce::Colour (0xfff6b9d1); }
    static juce::Colour pinkDeep()   { return juce::Colour (0xffd9739c); }
    static juce::Colour teal()       { return juce::Colour (0xff3fd3c6); }
    static juce::Colour tealDeep()   { return juce::Colour (0xff15877d); }
    static juce::Colour ink()        { return juce::Colour (0xff3a1f33); }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider&) override
    {
        const auto bounds = juce::Rectangle<float> ((float) x, (float) y, (float) width, (float) height).reduced (3.0f);
        const float d = juce::jmin (bounds.getWidth(), bounds.getHeight());
        const auto centre = bounds.getCentre();
        const float r = d * 0.5f;
        const float angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        // recessed bezel well
        g.setColour (juce::Colour (0xfff0a3c0));
        g.fillEllipse (juce::Rectangle<float> (d + 8.0f, d + 8.0f).withCentre (centre));

        // layered drop shadow / ambient occlusion
        g.setColour (tealDeep().withAlpha (0.18f));
        g.fillEllipse (juce::Rectangle<float> (d + 6.0f, d + 6.0f).withCentre (centre + juce::Point<float> (0.0f, 5.0f)));
        g.setColour (tealDeep().withAlpha (0.35f));
        g.fillEllipse (juce::Rectangle<float> (d, d).withCentre (centre + juce::Point<float> (0.0f, 3.0f)));

        // fluted turquoise cap
        const int flutes = 28;
        for (int i = 0; i < flutes; ++i)
        {
            const float a0 = juce::MathConstants<float>::twoPi * (float) i / (float) flutes;
            const float a1 = a0 + juce::MathConstants<float>::twoPi / (float) (flutes * 2);
            juce::Path wedge;
            wedge.startNewSubPath (centre);
            wedge.lineTo (centre.x + r * std::cos (a0), centre.y + r * std::sin (a0));
            wedge.lineTo (centre.x + r * std::cos (a1), centre.y + r * std::sin (a1));
            wedge.closeSubPath();
            g.setColour ((i % 2 == 0) ? juce::Colour (0xff2fb8ac) : juce::Colour (0xff25a196));
            g.fillPath (wedge);
        }

        // smooth top face with ambient occlusion rim
        const float inner = d * 0.74f;
        auto face = juce::Rectangle<float> (inner, inner).withCentre (centre);
        juce::ColourGradient grad (juce::Colour (0xffe6fffb), face.getX() + inner * 0.3f, face.getY() + inner * 0.2f,
                                   juce::Colour (0xff2bb3a7), face.getRight(), face.getBottom(), true);
        g.setGradientFill (grad);
        g.fillEllipse (face);

        // white indicator slit
        juce::Path slit;
        slit.addRoundedRectangle (-2.0f, -(r * 0.8f), 4.0f, r * 0.32f, 1.5f);
        g.setColour (juce::Colours::white);
        g.fillPath (slit, juce::AffineTransform::rotation (angle).translated (centre.x, centre.y));

        g.setColour (tealDeep());
        g.drawEllipse (juce::Rectangle<float> (d, d).withCentre (centre), 1.0f);
    }

    void drawComboBox (juce::Graphics& g, int width, int height, bool,
                       int, int, int, int, juce::ComboBox& box) override
    {
        auto bounds = juce::Rectangle<float> (0.0f, 0.0f, (float) width, (float) height).reduced (1.0f);
        g.setColour (juce::Colour (0x33000000));
        g.fillRoundedRectangle (bounds.translated (0.0f, 2.0f), 10.0f);
        g.setColour (box.findColour (juce::ComboBox::backgroundColourId));
        g.fillRoundedRectangle (bounds, 10.0f);
        g.setColour (teal());
        juce::Path p;
        const float cx = bounds.getRight() - 14.0f, cy = bounds.getCentreY();
        p.addTriangle (cx - 5.0f, cy - 3.0f, cx + 5.0f, cy - 3.0f, cx, cy + 3.0f);
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
