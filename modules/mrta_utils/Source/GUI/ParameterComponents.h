#pragma once

namespace mrta
{

class ParameterSlider : public juce::Slider
{
public:
    ParameterSlider(const juce::String& paramID, juce::AudioProcessorValueTreeState& apvts) :
        juce::Slider(juce::Slider::RotaryHorizontalVerticalDrag, juce::Slider::TextBoxRight),
        att(apvts, paramID, *this)
    {
        juce::AudioParameterFloat* param { dynamic_cast<juce::AudioParameterFloat*>(apvts.getParameter(paramID)) };
        if (!param)
        {
            // Parameter type is not Float
            jassertfalse;
        }

        setColour(juce::Slider::backgroundColourId, juce::Colours::white);
        setColour(juce::Slider::rotarySliderFillColourId, juce::Colours::orange);
    }

    ParameterSlider() = delete;

private:
    juce::AudioProcessorValueTreeState::SliderAttachment att;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParameterSlider)
};

class ParameterComboBox : public juce::ComboBox
{
public:
    ParameterComboBox(const juce::String& paramID, juce::AudioProcessorValueTreeState& apvts) :
        att(apvts, paramID, *this)
    {
        juce::AudioParameterChoice* param { dynamic_cast<juce::AudioParameterChoice*>(apvts.getParameter(paramID)) };
        if (!param)
        {
            // Parameter type is not Choice
            jassertfalse;
        }

        addItemList(param->choices, 1);
        setSelectedItemIndex(param->getIndex());
    }

    ParameterComboBox() = delete;

private:
    juce::AudioProcessorValueTreeState::ComboBoxAttachment att;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParameterComboBox)
};

class ParameterButton : public juce::TextButton
{
public:
    ParameterButton(const juce::String& paramID, juce::AudioProcessorValueTreeState& apvts) :
        att(apvts, paramID, *this)
    {
        juce::AudioParameterBool* param { dynamic_cast<juce::AudioParameterBool*>(apvts.getParameter(paramID)) };
        if (!param)
        {
            // Parameter type is not Bool
            jassertfalse;
        }

        juce::StringArray labels { param->getAllValueStrings() };

        setClickingTogglesState(true);
        setButtonText(labels[param->get() ? 1 : 0]);

        updateButtonColours();
        onStateChange = [labels, this]
        {
            setButtonText(labels[getToggleState() ? 1 : 0]);
            updateButtonColours();
        };
    }

    ParameterButton() = delete;

private:
    void updateButtonColours()
    {
        if (getToggleState())
        {
            setColour(juce::TextButton::buttonColourId, juce::Colours::green);
            setColour(juce::TextButton::buttonOnColourId, juce::Colours::green);
            setColour(juce::TextButton::textColourOnId, juce::Colours::white);
            setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        }
        else
        {
            setColour(juce::TextButton::buttonColourId, juce::Colour(153, 0, 0));
            setColour(juce::TextButton::buttonOnColourId, juce::Colour(153, 0, 0));
            setColour(juce::TextButton::textColourOnId, juce::Colours::white);
            setColour(juce::TextButton::textColourOffId, juce::Colours::white);
        }
    }

private:
    juce::AudioProcessorValueTreeState::ButtonAttachment att;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ParameterButton)
};

}

