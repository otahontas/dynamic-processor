#include "PluginProcessor.h"
#include "LevelDetector.h"
#include "PluginEditor.h"
#include "Utils.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include <vector>

static const std::vector<mrta::ParameterInfo> Parameters{
    // gate
    {Param::ID::GateEnabled, Param::Name::GateEnabled,
     Param::Ranges::EnabledOff, Param::Ranges::EnabledOn,
     Param::Defaults::GateEnabledDefault},
    {Param::ID::GateOutputGain, Param::Name::GateOutputGain, Param::Units::Db,
     Param::Defaults::GateOutputGainDefault, Param::Ranges::OutputGainMin,
     Param::Ranges::OutputGainMax, Param::Ranges::OutputGainInc,
     Param::Ranges::OutputGainSkw},
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

    // compressor
    {Param::ID::CompressorEnabled, Param::Name::CompressorEnabled,
     Param::Ranges::EnabledOff, Param::Ranges::EnabledOn,
     Param::Defaults::CompressorEnabledDefault},
    {Param::ID::CompressorOutputGain, Param::Name::CompressorOutputGain,
     Param::Units::Db, Param::Defaults::CompressorOutputGainDefault,
     Param::Ranges::OutputGainMin, Param::Ranges::OutputGainMax,
     Param::Ranges::OutputGainInc, Param::Ranges::OutputGainSkw},
    {Param::ID::CompressorThreshold, Param::Name::CompressorThreshold,
     Param::Units::Db, Param::Defaults::CompressorThresholdDefault,
     Param::Ranges::CompressorThresholdMin,
     Param::Ranges::CompressorThresholdMax,
     Param::Ranges::CompressorThresholdInc,
     Param::Ranges::CompressorThresholdSkw},
    {Param::ID::CompressorRatio, Param::Name::CompressorRatio, Param::Units::Db,
     Param::Defaults::CompressorRatioDefault, Param::Ranges::CompressorRatioMin,
     Param::Ranges::CompressorRatioMax, Param::Ranges::CompressorRatioInc,
     Param::Ranges::CompressorRatioSkw},
    {Param::ID::CompressorKnee, Param::Name::CompressorKnee, Param::Units::Db,
     Param::Defaults::CompressorKneeDefault, Param::Ranges::CompressorKneeMin,
     Param::Ranges::CompressorKneeMax, Param::Ranges::CompressorKneeInc,
     Param::Ranges::CompressorKneeSkw},
    {Param::ID::CompressorAttack, Param::Name::CompressorAttack,
     Param::Units::Ms, Param::Defaults::CompressorAttackDefault,
     Param::Ranges::CompressorAttackMin, Param::Ranges::CompressorAttackMax,
     Param::Ranges::CompressorAttackInc, Param::Ranges::CompressorAttackSkw},
    {Param::ID::CompressorRelease, Param::Name::CompressorRelease,
     Param::Units::Ms, Param::Defaults::CompressorReleaseDefault,
     Param::Ranges::CompressorReleaseMin, Param::Ranges::CompressorReleaseMax,
     Param::Ranges::CompressorReleaseInc, Param::Ranges::CompressorReleaseSkw},
    {Param::ID::CompressorMakeupGain, Param::Name::CompressorMakeupGain,
     Param::Units::Db, Param::Defaults::CompressorMakeupGainDefault,
     Param::Ranges::CompressorMakeupGainMin,
     Param::Ranges::CompressorMakeupGainMax,
     Param::Ranges::CompressorMakeupGainInc,
     Param::Ranges::CompressorMakeupGainSkw},
};

DynamicsAudioProcessor::DynamicsAudioProcessor()
    : parameterManager(*this, ProjectInfo::projectName, Parameters) {
  // gate
  parameterManager.registerParameterCallback(
      Param::ID::GateEnabled, [this](float newValue, bool /*forced*/) {
        gateEnabled = (newValue > 0.5f);
      });
  parameterManager.registerParameterCallback(
      Param::ID::GateOutputGain, [this](float value, bool forced) {
        float gainLinear = juce::Decibels::decibelsToGain(value);
        if (forced) {
          gateOutputGainSmoother.setCurrentAndTargetValue(gainLinear);
        } else {
          gateOutputGainSmoother.setTargetValue(gainLinear);
        }
      });
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
  // compressor
  parameterManager.registerParameterCallback(
      Param::ID::CompressorEnabled, [this](float newValue, bool /*forced*/) {
        compressorEnabled = (newValue > 0.5f);
      });
  parameterManager.registerParameterCallback(
      Param::ID::CompressorOutputGain, [this](float value, bool forced) {
        float gainLinear = juce::Decibels::decibelsToGain(value);
        if (forced) {
          compressorOutputGainSmoother.setCurrentAndTargetValue(gainLinear);
        } else {
          compressorOutputGainSmoother.setTargetValue(gainLinear);
        }
      });
  parameterManager.registerParameterCallback(
      Param::ID::CompressorThreshold,
      [this](float value, bool /*forced*/) { compressorThresholdDb = value; });
  parameterManager.registerParameterCallback(
      Param::ID::CompressorRatio,
      [this](float value, bool /*forced*/) { compressorRatio = value; });
  parameterManager.registerParameterCallback(
      Param::ID::CompressorKnee,
      [this](float value, bool /*forced*/) { compressorKneeDb = value; });
  parameterManager.registerParameterCallback(
      Param::ID::CompressorAttack, [this](float newValueMs, bool /*forced*/) {
        compressorAttackCoef = DSP::Utils::calculateSmoothingCoefficient(
            newValueMs, currentSampleRate);
      });
  parameterManager.registerParameterCallback(
      Param::ID::CompressorRelease, [this](float newValueMs, bool /*forced*/) {
        compressorReleaseCoef = DSP::Utils::calculateSmoothingCoefficient(
            newValueMs, currentSampleRate);
      });
  parameterManager.registerParameterCallback(
      Param::ID::CompressorMakeupGain,
      [this](float value, bool /*forced*/) { compressorMakeupGainDb = value; });
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
  compressorCurrentGainDb = 0.0f;
  parameterManager.updateParameters(true);
  resetInternalGateValuesToDefaults();
}

void DynamicsAudioProcessor::processBlock(juce::AudioBuffer<float> &buffer,
                                          juce::MidiBuffer &midiMessages) {
  juce::ScopedNoDenormals noDenormals;
  parameterManager.updateParameters();

  if (!gateEnabled && !compressorEnabled) {
    return;
  }

  int numChannels = buffer.getNumChannels();
  int numSamples = buffer.getNumSamples();

  for (int sample = 0; sample < numSamples; ++sample) {
    // --- LEVEL DETECTION ---//
    float maxEnvelopeDb = -101.0f; // Start with a very low value, below
                                   // silenceFloor from level detector
    for (int channel = 0; channel < numChannels; ++channel) {
      auto *channelData = buffer.getReadPointer(channel);
      DSP::LevelDetector &levelDetector = levelDetectors[channel];
      float inputSample = channelData[sample];
      float envDb = levelDetector.process(inputSample);
      if (envDb > maxEnvelopeDb)
        maxEnvelopeDb = envDb;
    }

    //--- GATE LOGIC  ---//
    float gateCurrentGainLinear = 1.0f;
    if (gateEnabled) {
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
      float gateTargetGainDb = gateIsOpen ? 0.0f : gateReductionDb;
      if (gateTargetGainDb > gateCurrentGainDb) {
        gateCurrentGainDb = DSP::Utils::calculateOnePoleSmoothedOutput(
            gateCurrentGainDb, gateTargetGainDb, gateAttackCoefficient);
      } else if (gateTargetGainDb < gateCurrentGainDb) {
        gateCurrentGainDb = DSP::Utils::calculateOnePoleSmoothedOutput(
            gateCurrentGainDb, gateTargetGainDb, gateReleaseCoefficient);
      }
      gateCurrentGainLinear =
          juce::Decibels::decibelsToGain(gateCurrentGainDb) *
          gateOutputGainSmoother.getNextValue();
    }

    //--- COMPRESSOR LOGIC ---//
    float compressorCurrentGainLinear = 1.0f;
    if (compressorEnabled) {
      float inputLevelOverThresholdDb = maxEnvelopeDb - compressorThresholdDb;
      float gainReductionDb = 0.0f;
      // soft knee
      if (compressorKneeDb > 0.0f) {
        float kneeHalf = compressorKneeDb / 2.0f;
        if (inputLevelOverThresholdDb <= -kneeHalf) {
          gainReductionDb = 0.0f;
        } else if (inputLevelOverThresholdDb > kneeHalf) {
          gainReductionDb =
              compressorThresholdDb + (inputLevelOverThresholdDb - kneeHalf) -
              (compressorThresholdDb +
               (inputLevelOverThresholdDb - kneeHalf) / compressorRatio);
        } else {
          gainReductionDb = ((1.0f / compressorRatio - 1.0f) *
                             powf(maxEnvelopeDb - compressorThresholdDb +
                                      compressorKneeDb / 2.0f,
                                  2.0f)) /
                            (2.0f * compressorKneeDb);
        }
      } else { // knee 0 -> hard knee
        if (inputLevelOverThresholdDb > 0.0f) {
          gainReductionDb = inputLevelOverThresholdDb -
                            inputLevelOverThresholdDb / compressorRatio;
        }
      }

      float compressorTargetGainDb = -gainReductionDb + compressorMakeupGainDb;
      float compressorCoef = (compressorTargetGainDb > compressorCurrentGainDb)
                                 ? compressorAttackCoef
                                 : compressorReleaseCoef;
      compressorCurrentGainDb = DSP::Utils::calculateOnePoleSmoothedOutput(
          compressorCurrentGainDb, compressorTargetGainDb, compressorCoef);
      compressorCurrentGainLinear =
          juce::Decibels::decibelsToGain(compressorCurrentGainDb) *
          compressorOutputGainSmoother.getNextValue();
    }

    //--- APPLY GAINS ---//
    for (int channel = 0; channel < numChannels; ++channel) {
      auto *writeBuffer = buffer.getWritePointer(channel);
      auto *channelData = buffer.getReadPointer(channel);
      writeBuffer[sample] = channelData[sample] * gateCurrentGainLinear *
                            compressorCurrentGainLinear;
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
  return new DynamicsAudioProcessorEditor(*this);
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
