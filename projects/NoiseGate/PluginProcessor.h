/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
/**
*/

namespace Param
{
    namespace ID
    {
        static const juce::String Enabled { "enabled" };
        static const juce::String Threshold { "threshold" };
        static const juce::String Ratio { "ratio" };
        static const juce::String Attack { "attack" };
        static const juce::String Release { "release" };
    }

    namespace Name
    {
        static const juce::String Enabled { "Enabled" };
        static const juce::String Threshold { "Threshold" };
        static const juce::String Ratio { "Ratio" };
        static const juce::String Attack { "Attack" };
        static const juce::String Release { "Release" };
    }

    namespace Ranges
    {
        static constexpr float ThresholdMin { -60.f };
        static constexpr float ThresholdMax { 0.f };
        static constexpr float ThresholdInc { 0.1f };
        static constexpr float ThresholdSkw { 0.5f };

        static constexpr float RatioMin { 1.f };
        static constexpr float RatioMax { 100.f };
        static constexpr float RatioInc { 0.1f };
        static constexpr float RatioSkw { 0.5f };

        static constexpr float AttackMin { 1.f };
        static constexpr float AttackMax { 10.f };
        static constexpr float AttackInc { 0.1f };
        static constexpr float AttackSkw { 0.5f };

        static constexpr float ReleaseMin { 5.f };
        static constexpr float ReleaseMax { 500.f };
        static constexpr float ReleaseInc { 0.1f };
        static constexpr float ReleaseSkw { 0.5f };

        static const juce::String EnabledOff { "Off" };
        static const juce::String EnabledOn { "On" };
    }

    namespace Units
    {
        static const juce::String Ms { "ms" };
        static const juce::String Hz { "Hz" };
        static const juce::String Db { "dB" };
    }
}

class NoiseGateAudioProcessor  : public juce::AudioProcessor
{
public:
    //==============================================================================
    NoiseGateAudioProcessor();
    ~NoiseGateAudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    mrta::ParameterManager& getParameterManager() { return parameterManager; }

    // static constexpr float MaxDelaySizeMs { 20.f };
    static const unsigned int MaxChannels { 2 };
    // static const unsigned int MaxProcessBlockSamples{ 32 };

private:
    //==============================================================================
    mrta::ParameterManager parameterManager;

    bool enabled { true };
    juce::dsp::NoiseGate<float> noiseGate;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (NoiseGateAudioProcessor)
};
