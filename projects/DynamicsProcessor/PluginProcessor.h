#pragma once

#include "LevelDetector.h"
#include <JuceHeader.h>
#include <vector>

namespace Param {
namespace ID {
// gate
static const juce::String GateEnabled{"gate_enabled"};
static const juce::String GateOutputGain{"gate_output_gain"};
static const juce::String GateThreshold{"gate_threshold"};
static const juce::String GateHysteresis{"gate_hysteresis"};
static const juce::String GateReduction{"gate_reduction"};
static const juce::String GateAttack{"gate_attack"};
static const juce::String GateHold{"gate_hold"};
static const juce::String GateRelease{"gate_release"};
// compressor
static const juce::String CompressorEnabled{"compressor_enabled"};
static const juce::String CompressorOutputGain{"compressor_output_gain"};
} // namespace ID

namespace Name {
// gate
static const juce::String GateEnabled{"Gate Enabled"};
static const juce::String GateOutputGain{"Gate Output Gain"};
static const juce::String GateThreshold{"Gate threshold"};
static const juce::String GateHysteresis{"Gate hysteresis"};
static const juce::String GateReduction{"Gate reduction"};
static const juce::String GateAttack{"Gate attack"};
static const juce::String GateHold{"Gate hold"};
static const juce::String GateRelease{"Gate release"};
// compressor
static const juce::String CompressorEnabled{"Compressor Enabled"};
static const juce::String CompressorOutputGain{"Compressor Output Gain"};
} // namespace Name

namespace Ranges {
// generic
static const juce::String EnabledOff{"Off"};
static const juce::String EnabledOn{"On"};
static constexpr float OutputGainMin{-60.0f};
static constexpr float OutputGainMax{6.0f};
static constexpr float OutputGainInc{0.1f};
static constexpr float OutputGainSkw{2.0f};

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
// gate
static constexpr bool GateEnabledDefault{true};
static constexpr float GateOutputGainDefault{0.0f};
static constexpr float GateThresholdDefault{-50.0f};
static constexpr float GateHysteresisDefault{-3.0f};
static constexpr float GateReductionDefault{-100.0f};
static constexpr float GateAttackDefault{3.0f};
static constexpr float GateHoldDefault{40.0f};
static constexpr float GateReleaseDefault{10.0f};
// compressor
static constexpr bool CompressorEnabledDefault{true};
static constexpr float CompressorOutputGainDefault{0.0f};
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
  // generic
  mrta::ParameterManager parameterManager;
  double currentSampleRate = 0;

  // gate params (stored directly)
  bool gateEnabled = Param::Defaults::GateEnabledDefault;
  juce::SmoothedValue<float> gateOutputGainSmoother;

  float gateOpenThresholdDb = Param::Defaults::GateThresholdDefault;
  float gateHysteresisDb = Param::Defaults::GateHysteresisDefault;
  float gateCloseThresholdDb = gateOpenThresholdDb + gateHysteresisDb;
  float gateReductionDb = Param::Defaults::GateReductionDefault;

  // gate params (calculated in parameter callbacks)
  float gateAttackCoefficient = 0.0f;
  float gateHoldSamples = 0.0f;
  float gateReleaseCoefficient = 0.0f;

  // internal vals
  std::vector<DSP::LevelDetector> levelDetectors;
  bool gateIsOpen = true;
  float gateCurrentGainDb = 0.0f;
  int gateHoldCounter = 0;

  // compressor params (stored directly, hardcoded for now)
  bool compressorEnabled = Param::Defaults::CompressorEnabledDefault;
  juce::SmoothedValue<float> compressorOutputGainSmoother;
  float compressorThresholdDb = -18.0f;
  float compressorRatio = 4.0f;
  float compressorKneeDb = 6.0f;
  float compressorAttackMs = 5.0f;
  float compressorReleaseMs = 50.0f;
  float compressorMakeupGainDb = 0.0f;
  float compressorAttackCoef = 0.0f;
  float compressorReleaseCoef = 0.0f;
  float compressorCurrentGainDb = 0.0f;

  // helpers
  void resetInternalGateValuesToDefaults();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(DynamicsAudioProcessor)
};
