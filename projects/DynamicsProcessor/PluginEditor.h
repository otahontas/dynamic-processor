#pragma once

#include "PluginProcessor.h"
#include <JuceHeader.h>

class NoiseGateAudioProcessorEditor : public juce::AudioProcessorEditor {
public:
  explicit NoiseGateAudioProcessorEditor(NoiseGateAudioProcessor &);
  ~NoiseGateAudioProcessorEditor() override;

  void paint(juce::Graphics &) override;
  void resized() override;

private:
  NoiseGateAudioProcessor &audioProcessor;
  mrta::GenericParameterEditor genericParameterEditor;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoiseGateAudioProcessorEditor)
};
