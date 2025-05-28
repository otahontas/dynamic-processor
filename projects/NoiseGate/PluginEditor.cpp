/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "PluginEditor.h"
#include "PluginProcessor.h"

//==============================================================================
NoiseGateAudioProcessorEditor::NoiseGateAudioProcessorEditor(
    NoiseGateAudioProcessor &p)
    : AudioProcessorEditor(&p), audioProcessor(p),
      genericParameterEditor(audioProcessor.getParameterManager()) {
  // Make sure that before the constructor has finished, you've set the
  // editor's size to whatever you need it to be.
  unsigned int numParams{static_cast<unsigned int>(
      audioProcessor.getParameterManager().getParameters().size())};
  unsigned int paramHeight{
      static_cast<unsigned int>(genericParameterEditor.parameterWidgetHeight)};

  addAndMakeVisible(genericParameterEditor);
  setSize(300, numParams * paramHeight);
}

NoiseGateAudioProcessorEditor::~NoiseGateAudioProcessorEditor() {}

//==============================================================================
void NoiseGateAudioProcessorEditor::paint(juce::Graphics &g) {
  // (Our component is opaque, so we must completely fill the background with a
  // solid colour)
  g.fillAll(
      getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));

  /*
  g.setColour (juce::Colours::white);
  g.setFont (juce::FontOptions (15.0f));
  g.drawFittedText ("Hello World!", getLocalBounds(),
  juce::Justification::centred, 1);
  */
}

void NoiseGateAudioProcessorEditor::resized() {
  // This is generally where you'll want to lay out the positions of any
  // subcomponents in your editor..
  genericParameterEditor.setBounds(getLocalBounds());
}
