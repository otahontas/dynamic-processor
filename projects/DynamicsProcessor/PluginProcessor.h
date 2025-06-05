#pragma once

#include "LevelDetector.h"
#include <JuceHeader.h>
#include <vector>

namespace Param {
namespace ID {
// generic
static const juce::String Enabled{"enabled"};
static const juce::String MasterGain{"master_gain"};

// gate
static const juce::String GateThreshold{"gate_threshold"};
static const juce::String GateHysteresis{"gate_hysteresis"};
static const juce::String GateReduction{"gate_reduction"};
static const juce::String GateAttack{"gate_attack"};
static const juce::String GateHold{"gate_hold"};
static const juce::String GateRelease{"gate_release"};
} // namespace ID

namespace Name {
// generic
static const juce::String Enabled{"Enabled"};
static const juce::String MasterGain{"Master gain"};

// gate
static const juce::String GateThreshold{"Gate threshold"};
static const juce::String GateHysteresis{"Gate hysteresis"};
static const juce::String GateReduction{"Gate reduction"};
static const juce::String GateAttack{"Gate attack"};
static const juce::String GateHold{"Gate hold"};
static const juce::String GateRelease{"Gate release"};
} // namespace Name

namespace Ranges {
// generic
static const juce::String EnabledOff{"Off"};
static const juce::String EnabledOn{"On"};
static constexpr float MasterGainMin{-60.0f};
static constexpr float MasterGainMax{6.0f};
static constexpr float MasterGainInc{0.1f};
static constexpr float MasterGainSkw{2.0f};

// gate
static constexpr float GateThresholdMin{-100.0f};
static constexpr float GateThresholdMax{0.0f};
static constexpr float GateThresholdInc{0.1f};
static constexpr float GateThresholdSkw{1.0f};

static constexpr float GateHysteresisMin{-20.0f};
static constexpr float GateHysteresisMax{0.0f};
static constexpr float GateHysteresisInc{0.1f};
static constexpr float GateHysteresisSkw{1.0f};

static constexpr float GateReductionMin{-100.0f};
static constexpr float GateReductionMax{0.0f};
static constexpr float GateReductionInc{0.1f};
static constexpr float GateReductionSkw{1.0f};

static constexpr float GateAttackMin{0.0f};
static constexpr float GateAttackMax{100.0f};
static constexpr float GateAttackInc{0.1f};
static constexpr float GateAttackSkw{1.0f};

static constexpr float GateHoldMin{0.0f};
static constexpr float GateHoldMax{1000.f};
static constexpr float GateHoldInc{0.1f};
static constexpr float GateHoldSkw{1.0f};

static constexpr float GateReleaseMin{0.0f};
static constexpr float GateReleaseMax{10000.0f};
static constexpr float GateReleaseInc{0.1f};
static constexpr float GateReleaseSkw{0.2f};
} // namespace Ranges
namespace Defaults {
// generic
static constexpr bool EnabledDefault{true};
static constexpr float MasterGainDefault{0.0f};

// gate
static constexpr float GateThresholdDefault{-50.0f};
static constexpr float GateHysteresisDefault{-3.0f};
static constexpr float GateReductionDefault{-100.0f};
static constexpr float GateAttackDefault{3.0f};
static constexpr float GateHoldDefault{40.0f};
static constexpr float GateReleaseDefault{10.0f};
} // namespace Defaults

namespace Units {
static const juce::String Ms{"ms"};
static const juce::String Hz{"Hz"};
static const juce::String Db{"dB"};
} // namespace Units
} // namespace Param

class DynamicsAudioProcessor : public juce::AudioProcessor {
public:
  DynamicsAudioProcessor();
  ~DynamicsAudioProcessor() override;

  void prepareToPlay(double sampleRate, int samplesPerBlock) override;
  void releaseResources() override;
  void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;
  void getStateInformation(juce::MemoryBlock &destData) override;
  void setStateInformation(const void *data, int sizeInBytes) override;

  mrta::ParameterManager &getParameterManager() { return parameterManager; }

  juce::AudioProcessorEditor *createEditor() override;
  bool hasEditor() const override;
  const juce::String getName() const override;
  bool acceptsMidi() const override;
  bool producesMidi() const override;
  bool isMidiEffect() const override;
  double getTailLengthSeconds() const override;
  int getNumPrograms() override;
  int getCurrentProgram() override;
  void setCurrentProgram(int index) override;
  const juce::String getProgramName(int index) override;
  void changeProgramName(int index, const juce::String &newName) override;

private:
  mrta::ParameterManager parameterManager;
  // generic
  double currentSampleRate = 0;
  bool isProcessorEnabled = Param::Defaults::EnabledDefault;
  juce::SmoothedValue<float> masterGainSmoother;
  std::vector<DSP::LevelDetector> levelDetectors;

  // gate params (stored directly)
  float gateOpenThresholdDb = Param::Defaults::GateThresholdDefault;
  float gateHysteresisDb = Param::Defaults::GateHysteresisDefault;
  float gateCloseThresholdDb = gateOpenThresholdDb + gateHysteresisDb;
  float gateReductionDb = Param::Defaults::GateReductionDefault;

  // gate params (calculated in parameter callbacks)
  float gateAttackCoefficient = 0.0f;
  float gateHoldSamples = 0.0f;
  float gateReleaseCoefficient = 0.0f;

  // internal vals
  bool gateIsOpen = true;
  float gateCurrentGainDb = 0.0f;
  int gateHoldCounter = 0;

  // helpers
  void resetInternalGateValuesToDefaults();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DynamicsAudioProcessor)
};
