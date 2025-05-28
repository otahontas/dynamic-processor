#include "PluginProcessor.h"
#include "PluginEditor.h"
#include <algorithm>
#include <vector>

static const std::vector<mrta::ParameterInfo> Parameters{
    // generic
    {Param::ID::Enabled, Param::Name::Enabled, Param::Ranges::EnabledOff,
     Param::Ranges::EnabledOn, Param::Defaults::EnabledDefault},
    {Param::ID::MasterGain, Param::Name::MasterGain, Param::Units::Db,
     Param::Defaults::MasterGainDefault, Param::Ranges::MasterGainMin,
     Param::Ranges::MasterGainMax, Param::Ranges::MasterGainInc,
     Param::Ranges::MasterGainSkw},

    // gate
    {Param::ID::GateThreshold, Param::Name::GateThreshold, Param::Units::Db,
     Param::Defaults::GateThresholdDefault, Param::Ranges::GateThresholdMin,
     Param::Ranges::GateThresholdMax, Param::Ranges::GateThresholdInc,
     Param::Ranges::GateThresholdSkw},

    {Param::ID::GateAttack, Param::Name::GateAttack, Param::Units::Ms,
     Param::Defaults::GateAttackDefault, Param::Ranges::GateAttackMin,
     Param::Ranges::GateAttackMax, Param::Ranges::GateAttackInc,
     Param::Ranges::GateAttackSkw},

    {Param::ID::GateHold, Param::Name::GateHold, Param::Units::Ms,
     Param::Defaults::GateHoldDefault, Param::Ranges::GateHoldMin,
     Param::Ranges::GateHoldMax, Param::Ranges::GateHoldInc,
     Param::Ranges::GateHoldSkw},

    {Param::ID::GateRelease, Param::Name::GateRelease, Param::Units::Ms,
     Param::Defaults::GateReleaseDefault, Param::Ranges::GateReleaseMin,
     Param::Ranges::GateReleaseMax, Param::Ranges::GateReleaseInc,
     Param::Ranges::GateReleaseSkw},
};

NoiseGateAudioProcessor::NoiseGateAudioProcessor()
    : parameterManager(*this, ProjectInfo::projectName, Parameters) {
  // generic
  parameterManager.registerParameterCallback(
      Param::ID::Enabled, [this](float newValue, bool /*forced*/) {
        isProcessorEnabled = (newValue > 0.5f);
        if (!isProcessorEnabled) {
          resetInternalGateValuesToDefaults();
        }
      });
  parameterManager.registerParameterCallback(
      Param::ID::MasterGain, [this](float value, bool forced) {
        float gainLinear = juce::Decibels::decibelsToGain(value);
        if (forced) {
          masterGainSmoother.setCurrentAndTargetValue(gainLinear);
        } else {
          masterGainSmoother.setTargetValue(gainLinear);
        }
      });

  // gate
  // TODO: smoothen the value changes for gate
  parameterManager.registerParameterCallback(
      Param::ID::GateThreshold, [this](float newValueDb, bool /*forced*/) {
        gateThresholdLinear = juce::Decibels::decibelsToGain(newValueDb);
      });
  parameterManager.registerParameterCallback(
      Param::ID::GateAttack, [this](float newValueMs, bool /*forced*/) {
        gateAttackCoeff =
            calculateInternalGateCoeff(newValueMs, currentSampleRate);
      });
  parameterManager.registerParameterCallback(
      Param::ID::GateHold, [this](float newValueMs, bool /*forced*/) {
        gateHoldSamples = msToSamples(newValueMs, currentSampleRate);
      });
  parameterManager.registerParameterCallback(
      Param::ID::GateRelease, [this](float newValueMs, bool /*forced*/) {
        gateReleaseCoeff =
            calculateInternalGateCoeff(newValueMs, currentSampleRate);
      });
}

NoiseGateAudioProcessor::~NoiseGateAudioProcessor() {}

// TODO: check if juce provides this too
float NoiseGateAudioProcessor::msToSamples(float valueMs, double sampleRate) {
  return (valueMs / 1000.0f) * static_cast<float>(sampleRate);
}

float NoiseGateAudioProcessor::calculateInternalGateCoeff(float timeMs,
                                                          double sampleRate) {
  if (timeMs <= 0.0f || sampleRate <= 0.0) {
    return 0.0f;
  }
  return std::exp(-1.0f / msToSamples(timeMs, sampleRate));
}

void NoiseGateAudioProcessor::resetInternalGateValuesToDefaults() {
  gateEnvelope = GATE_ENVELOPE_DEFAULT;
  gateCurrentGain = GATE_CURRENT_GAIN_DEFAULT;
  gateHoldCounter = GATE_HOLD_COUNTER_DEFAULT;
}

void NoiseGateAudioProcessor::prepareToPlay(double sampleRate,
                                            int samplesPerBlock) {
  currentSampleRate = sampleRate;
  resetInternalGateValuesToDefaults();
  detectorAttackCoeff = calculateInternalGateCoeff(
      GATE_ENVELOPE_DETECTOR_ATTACK_TIME_DEFAULT, currentSampleRate);
  detectorReleaseCoeff = calculateInternalGateCoeff(
      GATE_ENVELOPE_DETECTOR_RELEASE_TIME_DEFAULT, currentSampleRate);

  parameterManager.updateParameters(true);
}

void NoiseGateAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                           juce::MidiBuffer &midiMessages) {
  juce::ScopedNoDenormals noDenormals;
  parameterManager.updateParameters();

  if (!isProcessorEnabled) {
    return;
  }

  float currentMasterOutputGain = masterGainSmoother.getNextValue();

  for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
    // read & write to sepearate buffs, VSTs might have differences between
    auto *channelData = buffer.getReadPointer(channel);
    auto *writeChannel = buffer.getWritePointer(channel);

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
      float inputSample = channelData[sample];
      float inputAbs = std::abs(inputSample);

      // 1. update internal gate envelope that tracks the input level
      if (inputAbs > gateEnvelope) {
        gateEnvelope = detectorAttackCoeff * gateEnvelope +
                       (1.0f - detectorAttackCoeff) * inputAbs;
      } else {
        gateEnvelope = detectorReleaseCoeff * gateEnvelope +
                       (1.0f - detectorReleaseCoeff) * inputAbs;
      }

      // 2. run the gate logic
      // gate is
      // - open if
      //    - input level is over threshold
      //    - input level is not over threshold but we're not pass the hold time
      // - off otherwise
      bool gateOpen = false;
      if (gateEnvelope > gateThresholdLinear) {
        gateOpen = true;
        gateHoldCounter = static_cast<int>(gateHoldSamples);
      } else {
        if (gateHoldCounter > 0) {
          gateHoldCounter--;
          gateOpen = true;
        } else {
          gateOpen = false;
        }
      }
      float gateOpenFloatVal = static_cast<float>(gateOpen);

      // 3. smooth gain based on attack and release from user
      // clamps (min / max) values just in case gate over 1 / less than 0
      if (gateOpen > gateCurrentGain) {
        gateCurrentGain = gateAttackCoeff * gateCurrentGain +
                          (1.0f - gateAttackCoeff) * gateOpenFloatVal;
        gateCurrentGain = std::min(gateCurrentGain, gateOpenFloatVal);
      } else if (gateOpen < gateCurrentGain) {
        gateCurrentGain = gateReleaseCoeff * gateCurrentGain +
                          (1.0f - gateReleaseCoeff) * gateOpenFloatVal;
        gateCurrentGain = std::max(gateCurrentGain, gateOpenFloatVal);
      }

      writeChannel[sample] =
          inputSample * gateCurrentGain * currentMasterOutputGain;
    }
  }
}

void NoiseGateAudioProcessor::releaseResources() {
  resetInternalGateValuesToDefaults();
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
