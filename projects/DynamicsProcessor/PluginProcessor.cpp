#include "PluginProcessor.h"
#include "LevelDetector.h"
#include "PluginEditor.h"
#include "Utils.h"
#include "juce_audio_basics/juce_audio_basics.h"
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

    {Param::ID::GateHysteresis, Param::Name::GateHysteresis, Param::Units::Db,
     Param::Defaults::GateHysteresisDefault, Param::Ranges::GateHysteresisMin,
     Param::Ranges::GateHysteresisMax, Param::Ranges::GateHysteresisInc,
     Param::Ranges::GateHysteresisSkw},

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

DynamicsAudioProcessor::DynamicsAudioProcessor()
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
        gateOpenThresholdDb = newValueDb;
        gateCloseThresholdDb = gateOpenThresholdDb + gateHysteresisDb;
      });
  parameterManager.registerParameterCallback(
      Param::ID::GateHysteresis, [this](float newValueDb, bool /*forced*/) {
        gateHysteresisDb = newValueDb;
        gateCloseThresholdDb = gateOpenThresholdDb + gateHysteresisDb;
      });
  parameterManager.registerParameterCallback(
      Param::ID::GateReduction, [this](float newValueDb, bool /*forced*/) {
        gateReductionDb = newValueDb;
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

DynamicsAudioProcessor::~DynamicsAudioProcessor() {}

void DynamicsAudioProcessor::resetInternalGateValuesToDefaults() {
  gateIsOpen = true;
  gateCurrentGainDb = 0.0f;
  gateHoldCounter = 0;
}

void DynamicsAudioProcessor::prepareToPlay(double sampleRate,
                                           int samplesPerBlock) {
  currentSampleRate = sampleRate;
  int numChannels = getTotalNumInputChannels();
  levelDetectors.resize(numChannels);
  for (auto &levelDetector : levelDetectors)
    levelDetector.prepare(sampleRate);
  parameterManager.updateParameters(true);
  resetInternalGateValuesToDefaults();
}

void DynamicsAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                          juce::MidiBuffer &midiMessages) {
  juce::ScopedNoDenormals noDenormals;
  parameterManager.updateParameters();

  if (!isProcessorEnabled) {
    return;
  }

  float currentMasterOutputGain = masterGainSmoother.getNextValue();
  int numChannels = buffer.getNumChannels();
  int numSamples = buffer.getNumSamples();

  for (int sample = 0; sample < numSamples; ++sample) {
    float maxEnvelopeDb =
        gateOpenThresholdDb - 1000.0f; // definitely below threshold

    // collect envelope values from all channels
    for (int channel = 0; channel < numChannels; ++channel) {
      auto *channelData = buffer.getReadPointer(channel);
      DSP::LevelDetector &levelDetector = levelDetectors[channel];
      float inputSample = channelData[sample];
      float envDb = levelDetector.process(inputSample);
      if (envDb > maxEnvelopeDb)
        maxEnvelopeDb = envDb;
    }

    // linked gating logic
    float gateEnvelopeDb = maxEnvelopeDb;
    if (gateEnvelopeDb > gateOpenThresholdDb) {
      gateIsOpen = true;
      gateHoldCounter = static_cast<int>(gateHoldSamples);
    } else if (gateEnvelopeDb < gateCloseThresholdDb) {
      if (gateHoldCounter > 0) {
        gateHoldCounter--;
      } else {
        gateIsOpen = false;
      }
    }

    float targetGainDb = gateIsOpen ? 0.0f : gateReductionDb;

    if (targetGainDb > gateCurrentGainDb) {
      gateCurrentGainDb =
          std::min(DSP::Utils::calculateOnePoleSmoothedOutput(
                       gateCurrentGainDb, targetGainDb, gateAttackCoefficient),
                   targetGainDb);
    } else if (targetGainDb < gateCurrentGainDb) {
      gateCurrentGainDb =
          std::max(DSP::Utils::calculateOnePoleSmoothedOutput(
                       gateCurrentGainDb, targetGainDb, gateReleaseCoefficient),
                   targetGainDb);
    }
    auto gateCurrentGainLinear =
        juce::Decibels::decibelsToGain(gateCurrentGainDb);

    // apply gate gain to all channels for this sample
    for (int channel = 0; channel < numChannels; ++channel) {
      auto *writeBuffer = buffer.getWritePointer(channel);
      auto *channelData = buffer.getReadPointer(channel);
      writeBuffer[sample] =
          channelData[sample] * gateCurrentGainLinear * currentMasterOutputGain;
    }
  }
}

void DynamicsAudioProcessor::releaseResources() {
  resetInternalGateValuesToDefaults();
  for (auto &detector : levelDetectors)
    detector.reset();
}

void DynamicsAudioProcessor::getStateInformation(juce::MemoryBlock &destData) {
  parameterManager.getStateInformation(destData);
}

void DynamicsAudioProcessor::setStateInformation(const void *data,
                                                 int sizeInBytes) {
  parameterManager.setStateInformation(data, sizeInBytes);
}

juce::AudioProcessorEditor *DynamicsAudioProcessor::createEditor() {
  return new DynamicsAudioProcessor(*this);
}

const juce::String DynamicsAudioProcessor::getName() const {
  return JucePlugin_Name;
}
bool DynamicsAudioProcessor::acceptsMidi() const { return false; }
bool DynamicsAudioProcessor::producesMidi() const { return false; }
bool DynamicsAudioProcessor::isMidiEffect() const { return false; }
double DynamicsAudioProcessor::getTailLengthSeconds() const { return 0.0; }
int DynamicsAudioProcessor::getNumPrograms() { return 1; }
int DynamicsAudioProcessor::getCurrentProgram() { return 0; }
void DynamicsAudioProcessor::setCurrentProgram(int) {}
const juce::String DynamicsAudioProcessor::getProgramName(int) { return {}; }
void DynamicsAudioProcessor::changeProgramName(int, const juce::String &) {}
bool DynamicsAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() {
  return new DynamicsAudioProcessor();
}
