#include "PluginEditor.h"
#include "PluginProcessor.h"

NoiseGateAudioProcessorEditor::NoiseGateAudioProcessorEditor(
    NoiseGateAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p),
      genericParameterEditor(audioProcessor.getParameterManager()) {
  auto numParams = audioProcessor.getParameterManager().getParameters().size();
  auto paramHeight = genericParameterEditor.parameterWidgetHeight;

  setSize(300, numParams * paramHeight);
  addAndMakeVisible(genericParameterEditor);
}

NoiseGateAudioProcessorEditor::~NoiseGateAudioProcessorEditor() {}

void NoiseGateAudioProcessorEditor::paint(juce::Graphics &g) {
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void NoiseGateAudioProcessorEditor::resized() {
  genericParameterEditor.setBounds(getLocalBounds());
}