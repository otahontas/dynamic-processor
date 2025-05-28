#include "PluginProcessor.h"
#include "PluginEditor.h"

// TODO: add the defaults to header as well?
static const std::vector<mrta::ParameterInfo> Parameters{
    {Param::ID::Enabled, Param::Name::Enabled, Param::Ranges::EnabledOff,
     Param::Ranges::EnabledOn, true},
    {Param::ID::Threshold, Param::Name::Threshold, Param::Units::Db, -20.f,
     Param::Ranges::ThresholdMin, Param::Ranges::ThresholdMax,
     Param::Ranges::ThresholdInc, Param::Ranges::ThresholdSkw},
    {Param::ID::Ratio, Param::Name::Ratio, Param::Units::Db, 2.f,
     Param::Ranges::RatioMin, Param::Ranges::RatioMax, Param::Ranges::RatioInc,
     Param::Ranges::RatioSkw},
    {Param::ID::Attack, Param::Name::Attack, Param::Units::Ms, 5.f,
     Param::Ranges::AttackMin, Param::Ranges::AttackMax,
     Param::Ranges::AttackInc, Param::Ranges::AttackSkw},
    {Param::ID::Release, Param::Name::Release, Param::Units::Ms, 5.f,
     Param::Ranges::ReleaseMin, Param::Ranges::ReleaseMax,
     Param::Ranges::ReleaseInc, Param::Ranges::ReleaseSkw}};

NoiseGateAudioProcessor::NoiseGateAudioProcessor()
    : parameterManager(*this, ProjectInfo::projectName, Parameters) {
  parameterManager.registerParameterCallback(
      Param::ID::Enabled, [this](float newValue, bool /*forced*/) {
        isProcessorEnabled = (newValue > 0.5f);
      });

  parameterManager.registerParameterCallback(
      Param::ID::Threshold, [this](float newValue, bool /*forced*/) {
        noiseGate.setThreshold(newValue);
      });

  parameterManager.registerParameterCallback(
      Param::ID::Ratio, [this](float newValue, bool /*forced*/) {
        noiseGate.setRatio(newValue);
      });

  parameterManager.registerParameterCallback(
      Param::ID::Attack, [this](float newValue, bool /*forced*/) {
        noiseGate.setAttack(newValue);
      });

  parameterManager.registerParameterCallback(
      Param::ID::Release, [this](float newValue, bool /*forced*/) {
        noiseGate.setRelease(newValue);
      });
}

NoiseGateAudioProcessor::~NoiseGateAudioProcessor() {}

void NoiseGateAudioProcessor::prepareToPlay(double sampleRate,
                                            int samplesPerBlock) {
  juce::uint32 numChannels{static_cast<juce::uint32>(

      std::max(getMainBusNumInputChannels(), getMainBusNumOutputChannels()))};
  noiseGate.prepare(
      {sampleRate, static_cast<juce::uint32>(samplesPerBlock), numChannels});
  parameterManager.updateParameters(true);
}

void NoiseGateAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                           juce::MidiBuffer &midiMessages) {
  juce::ScopedNoDenormals noDenormals;
  parameterManager.updateParameters();

  if (!isProcessorEnabled) {
    return;
  }

  {
    juce::dsp::AudioBlock<float> audioBlock(buffer.getArrayOfWritePointers(),
                                            buffer.getNumChannels(),
                                            buffer.getNumSamples());
    juce::dsp::ProcessContextReplacing<float> ctx(audioBlock);
    noiseGate.process(ctx);
  }
}

void NoiseGateAudioProcessor::releaseResources() { noiseGate.reset(); }

void NoiseGateAudioProcessor::getStateInformation(juce::MemoryBlock &destData) {
  parameterManager.getStateInformation(destData);
}

void NoiseGateAudioProcessor::setStateInformation(const void *data,
                                                  int sizeInBytes) {
  parameterManager.setStateInformation(data, sizeInBytes);
}

juce::AudioProcessorEditor *NoiseGateAudioProcessor::createEditor() {
  return new NoiseGateAudioProcessorEditor(*this);
}

//==============================================================================
const juce::String NoiseGateAudioProcessor::getName() const {
  return JucePlugin_Name;
}
bool NoiseGateAudioProcessor::acceptsMidi() const { return false; }
bool NoiseGateAudioProcessor::producesMidi() const { return false; }
bool NoiseGateAudioProcessor::isMidiEffect() const { return false; }
double NoiseGateAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int NoiseGateAudioProcessor::getNumPrograms() { return 1; }
int NoiseGateAudioProcessor::getCurrentProgram() { return 0; }
void NoiseGateAudioProcessor::setCurrentProgram(int) {}
const juce::String NoiseGateAudioProcessor::getProgramName(int) { return {}; }
void NoiseGateAudioProcessor::changeProgramName(int, const juce::String &) {}
bool NoiseGateAudioProcessor::hasEditor() const { return true; }
//==============================================================================

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new NoiseGateAudioProcessor();
}
