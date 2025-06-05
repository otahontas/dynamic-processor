#pragma once

#include "PluginProcessor.h"

class DynamicsAudioProcessorEditor : public juce::AudioProcessorEditor {
public:
  explicit DynamicsAudioProcessorEditor(DynamicsAudioProcessor &);
  ~DynamicsAudioProcessorEditor() override;

  void paint(juce::Graphics &) override;
  void resized() override;

private:
  DynamicsAudioProcessor &audioProcessor;
  mrta::GenericParameterEditor genericParameterEditor;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DynamicsAudioProcessorEditor)
};
