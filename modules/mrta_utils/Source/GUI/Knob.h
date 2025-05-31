#pragma once
#include <JuceHeader.h>

// A simple knob‐drawing LookAndFeel
class Knob : public juce::LookAndFeel_V4
{
public:
    Knob()  {}
    ~Knob() override {}

    void drawRotarySlider (juce::Graphics& g,
                           int x, int y, int width, int height,
                           float sliderPosProportional,
                           float rotaryStartAngle,
                           float rotaryEndAngle,
                           juce::Slider& slider) override
    {
        auto radius  = juce::jmin(width, height) * 0.5f;
        auto centreX = x + width  * 0.5f;
        auto centreY = y + height * 0.5f;

        // 1) Draw a circular base with a simple gradient
        {
            juce::ColourGradient grad (juce::Colours::grey,
                                       centreX, centreY - radius,
                                       juce::Colours::darkgrey,
                                       centreX, centreY + radius,
                                       false);
            g.setGradientFill (grad);
            g.fillEllipse (centreX - radius, centreY - radius,
                           radius * 2.0f, radius * 2.0f);
        }

        // 2) Draw a black outline ring
        g.setColour (juce::Colours::black);
        g.drawEllipse (centreX - radius, centreY - radius,
                       radius * 2.0f, radius * 2.0f, 2.0f);

        // 3) Compute the pointer’s angle
        auto angle = rotaryStartAngle + sliderPosProportional * (rotaryEndAngle - rotaryStartAngle);

        // 4) Build a narrow “pointer” rectangle and rotate it
        juce::Path pointer;
        auto pointerLength    = radius * 0.8f;
        auto pointerThickness = 4.0f;
        pointer.addRectangle (-pointerThickness * 0.5f,
                              -radius,
                              pointerThickness,
                              pointerLength);
        pointer.applyTransform (juce::AffineTransform::rotation (angle)
                                                    .translated (centreX, centreY));

        // 5) Fill the pointer in yellow
        g.setColour (juce::Colours::yellow);
        g.fillPath (pointer);

        // 6) Optionally draw the numeric value inside
        auto textValue = juce::String ((float) slider.getValue(), 1);
        g.setColour (juce::Colours::white);
        g.setFont (radius * 0.3f);
        g.drawFittedText (textValue,
                          x, y + (int) radius - 10,
                          width, 20,
                          juce::Justification::centred,
                          1);
    }
};
