#include "PluginProcessor.h"
#include "PluginEditor.h"

static const std::vector<mrta::ParameterInfo> Parameters{
    {Param::ID::Enabled, Param::Name::Enabled, Param::Ranges::EnabledOff,
     Param::Ranges::EnabledOn, true},

    // TODO: replace with ranges
    // TODO: add the defaults to header
    {Param::ID::GateThreshold, Param::Name::GateThreshold, "dB", -40.0f, -96.0f,
     0.0f, 0.1f, 2.0f},
    {Param::ID::GateAttack, Param::Name::GateAttack, "ms", 5.0f, 0.1f, 1000.0f,
     0.1f, 0.3f},
    {Param::ID::GateHold, Param::Name::GateHold, "ms", 10.0f, 0.0f, 2000.0f,
     0.1f, 0.3f},
    {Param::ID::GateRelease, Param::Name::GateRelease, "ms", 50.0f, 1.0f,
     5000.0f, 0.1f, 0.3f},
};

NoiseGateAudioProcessor::NoiseGateAudioProcessor()
    : parameterManager(*this, ProjectInfo::projectName, Parameters) {

  parameterManager.registerParameterCallback(
      Param::ID::Enabled, [this](float value, bool /*forced*/) {
        isProcessorEnabled = (value > 0.5f);

        // if not enabled, reset the state. TODO: investigate if good
        if (!isProcessorEnabled) {
          // TODO: use defaults so they're shared
          gateEnvelope = 0.0f;
          gateCurrentGain = 0.0f;
          gateHoldCounter = 0;
        }
      });

  parameterManager.registerParameterCallback(
      Param::ID::GateThreshold, [this](float value, bool /*forced*/) {
        gateThresholdLinear = juce::Decibels::decibelsToGain(value);
      });

  // TODO: check if better to calculate in prepare to play or even when
  // processing block?
  parameterManager.registerParameterCallback(
      Param::ID::GateAttack, [this](float value, bool /*forced*/) {
        gateAttackCoeff = calculateCoefficient(value, currentSampleRate);
      });
  parameterManager.registerParameterCallback(
      Param::ID::GateHold, [this](float value, bool /*forced*/) {
        gateHoldSamples =
            (value / 1000.0f) * static_cast<float>(currentSampleRate);
      });
  parameterManager.registerParameterCallback(
      Param::ID::GateRelease, [this](float value, bool /*forced*/) {
        gateReleaseCoeff = calculateCoefficient(value, currentSampleRate);
      });
}

NoiseGateAudioProcessor::~NoiseGateAudioProcessor() {}

float NoiseGateAudioProcessor::calculateCoefficient(float timeMs, double sr) {
  if (timeMs <= 0.0f || sr <= 0.0) {
    return 0.0f;
  }
  return std::exp(-1.0f / ((timeMs / 1000.0f) * static_cast<float>(sr)));
}

void NoiseGateAudioProcessor::prepareToPlay(double sampleRate,
                                            int samplesPerBlock) {
  currentSampleRate = sampleRate;

  // TODO: use defaults from header (share with disabling resetting the state)
  gateEnvelope = 0.0f;
  gateCurrentGain = 0.0f;
  gateHoldCounter = 0;

  // calc coeffs with fixed values TODO: set fixed values in header
  detectorAttackCoeff = calculateCoefficient(1.0f, currentSampleRate);
  detectorReleaseCoeff = calculateCoefficient(100.0f, currentSampleRate);

  parameterManager.updateParameters(true);
}

void NoiseGateAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                           juce::MidiBuffer &midiMessages) {
  juce::ScopedNoDenormals noDenormals;
  parameterManager.updateParameters();

  if (!isProcessorEnabled) {
    return;
  }

  for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
    auto *channelData = buffer.getWritePointer(channel);

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
      float inputSample = channelData[sample];
      float inputAbs = std::abs(inputSample);

      // 1. update internal peak detector
      if (inputAbs > gateEnvelope) {
        gateEnvelope = detectorAttackCoeff * gateEnvelope +
                       (1.0f - detectorAttackCoeff) * inputAbs;
      } else {
        gateEnvelope = detectorReleaseCoeff * gateEnvelope +
                       (1.0f - detectorReleaseCoeff) * inputAbs;
      }

      // 2. run the gate logic
      float targetGain = 0.0f;
      bool signalIsAboveThreshold = (gateEnvelope > gateThresholdLinear);

      if (signalIsAboveThreshold) {
        targetGain = 1.0f;
        gateHoldCounter = static_cast<int>(gateHoldSamples);
      } else {
        if (gateHoldCounter > 0) {
          gateHoldCounter--;
          targetGain = 1.0f;
        } else {
          targetGain = 0.0f;
        }
      }

      // 3. smooth
      if (targetGain > gateCurrentGain) {
        gateCurrentGain = gateAttackCoeff * gateCurrentGain +
                          (1.0f - gateAttackCoeff) * targetGain;
        gateCurrentGain = std::min(gateCurrentGain, targetGain);
      } else if (targetGain < gateCurrentGain) {
        gateCurrentGain = gateReleaseCoeff * gateCurrentGain +
                          (1.0f - gateReleaseCoeff) * targetGain;
        gateCurrentGain = std::max(gateCurrentGain, targetGain);
      }

      channelData[sample] = inputSample * gateCurrentGain;
    }
  }
}

void NoiseGateAudioProcessor::releaseResources() {
  // TODO: use defaults here too
  gateEnvelope = 0.0f;
  gateCurrentGain = 0.0f;
  gateHoldCounter = 0;
}

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
