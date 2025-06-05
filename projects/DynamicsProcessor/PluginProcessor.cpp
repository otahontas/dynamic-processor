#include "PluginProcessor.h"
#include "LevelDetector.h"
#include "PluginEditor.h"
#include "Utils.h"
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

    {Param::ID::GateReduction, Param::Name::GateReduction, Param::Units::Db,
     Param::Defaults::GateReductionDefault, Param::Ranges::GateReductionMin,
     Param::Ranges::GateReductionMax, Param::Ranges::GateReductionInc,
     Param::Ranges::GateReductionSkw},

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
  parameterManager.registerParameterCallback(
      Param::ID::GateThreshold, [this](float newValueDb, bool /*forced*/) {
        gateThresholdLinear = juce::Decibels::decibelsToGain(newValueDb);
      });
  parameterManager.registerParameterCallback(
      Param::ID::GateReduction, [this](float newValueDb, bool /*forced*/) {
        gateReductionLinear = juce::Decibels::decibelsToGain(newValueDb);
      });
  parameterManager.registerParameterCallback(
      Param::ID::GateAttack, [this](float newValueMs, bool /*forced*/) {
        gateAttackCoefficient = DSP::Utils::calculateSmoothingCoefficient(
            newValueMs, currentSampleRate);
      });
  parameterManager.registerParameterCallback(
      Param::ID::GateHold, [this](float newValueMs, bool /*forced*/) {
        gateHoldSamples =
            DSP::Utils::msToSamples(newValueMs, currentSampleRate);
      });
  parameterManager.registerParameterCallback(
      Param::ID::GateRelease, [this](float newValueMs, bool /*forced*/) {
        gateReleaseCoefficient = DSP::Utils::calculateSmoothingCoefficient(
            newValueMs, currentSampleRate);
      });
}

NoiseGateAudioProcessor::~NoiseGateAudioProcessor() {}

void NoiseGateAudioProcessor::resetInternalGateValuesToDefaults() {
  gateCurrentGain = gateReductionLinear;
  gateHoldCounter = GATE_HOLD_COUNTER_DEFAULT;
}

void NoiseGateAudioProcessor::prepareToPlay(double sampleRate,
                                            int samplesPerBlock) {
  currentSampleRate = sampleRate;
  levelDetector.prepare(sampleRate);
  parameterManager.updateParameters(true);
  resetInternalGateValuesToDefaults();
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
    auto *writeBuffer = buffer.getWritePointer(channel);

    for (int sample = 0; sample < buffer.getNumSamples(); ++sample) {
      float inputSample = channelData[sample];

      // 1. get the envelope level of the input sample
      float gateEnvelope = levelDetector.process(inputSample);

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

      // 3. get the gain reduction (if open, no reduction, otherwise reduce)
      float targetGain = gateOpen ? 1.0f : gateReductionLinear;

      // 4. smooth gain with one-pole smoothing based on attack and release from
      // user
      // clamps (min / max) values just in case gate over 1 / less than 0
      if (targetGain > gateCurrentGain) {
        gateCurrentGain =
            std::min(DSP::Utils::calculateOnePoleSmoothedOutput(
                         gateCurrentGain, targetGain, gateAttackCoefficient),
                     targetGain);
      } else if (targetGain < gateCurrentGain) {
        gateCurrentGain =
            std::max(DSP::Utils::calculateOnePoleSmoothedOutput(
                         gateCurrentGain, targetGain, gateReleaseCoefficient),
                     targetGain);
      }
      writeBuffer[sample] =
          inputSample * gateCurrentGain * currentMasterOutputGain;
    }
  }
}

void NoiseGateAudioProcessor::releaseResources() {
  resetInternalGateValuesToDefaults();
  levelDetector.reset();
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
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new NoiseGateAudioProcessor();
}
