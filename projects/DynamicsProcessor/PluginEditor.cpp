#include "PluginEditor.h"

DynamicsAudioProcessorEditor::DynamicsAudioProcessorEditor(
    DynamicsAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p),
      genericParameterEditor(audioProcessor.getParameterManager()) {
  auto numParams = audioProcessor.getParameterManager().getParameters().size();
  auto paramHeight = genericParameterEditor.parameterWidgetHeight;

  setSize(300, numParams * paramHeight);
  addAndMakeVisible(genericParameterEditor);
}

DynamicsAudioProcessorEditor::~DynamicsAudioProcessorEditor() {}

void DynamicsAudioProcessorEditor::paint(juce::Graphics &g) {
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void DynamicsAudioProcessorEditor::resized() {
  genericParameterEditor.setBounds(getLocalBounds());
}
