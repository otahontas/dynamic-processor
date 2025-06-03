#pragma once

#include <JuceHeader.h>
namespace Param {
namespace ID {
// generic
static const juce::String Enabled{"enabled"};
static const juce::String MasterGain{"master_gain"};

// gate
static const juce::String GateThreshold{"gate_threshold"};
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

class NoiseGateAudioProcessor : public juce::AudioProcessor {
public:
  NoiseGateAudioProcessor();
  ~NoiseGateAudioProcessor() override;

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
  bool isProcessorEnabled = true;
  juce::SmoothedValue<float> masterGainSmoother;

  // internal gate defaults
  // - envelope, gain and hold counter start always from zero
  // - attack (fast) and release (moderate) time are used to set how quickly
  // peak detection reacts to changes in input. Set to 0 if you want peak
  // detection to work instantly all the time.
  static constexpr float GATE_ENVELOPE_DEFAULT = 0.0f;
  static constexpr float GATE_REDUCTION_LINEAR_DEFAULT = 0.0f;
  static constexpr int GATE_HOLD_COUNTER_DEFAULT = 0;
  static constexpr float GATE_ENVELOPE_DETECTOR_ATTACK_TIME_DEFAULT = 1.0f;
  static constexpr float GATE_ENVELOPE_DETECTOR_RELEASE_TIME_DEFAULT = 100.0f;

  // gate stuff
  // == calculated / set for internal stuff
  float gateEnvelope = GATE_ENVELOPE_DEFAULT;
  float gateCurrentGain = GATE_REDUCTION_LINEAR_DEFAULT;
  int gateHoldCounter = GATE_HOLD_COUNTER_DEFAULT;
  float detectorAttackCoeff = 0.0f;
  float detectorReleaseCoeff = 0.0f;
  // == calculated based on user settings
  float gateThresholdLinear = 0.0f;
  float gateReductionLinear = GATE_REDUCTION_LINEAR_DEFAULT;
  float gateAttackCoeff = 0.0f;
  float gateReleaseCoeff = 0.0f;
  float gateHoldSamples = 0.0f;

  // helpers
  float msToSamples(float timeMs, double sampleRate);
  float calculateInternalGateCoeff(float valueMs, double sampleRate);
  float applyOnePoleSmoothing(float currentValue, float targetValue,
                              float smoothingCoeff);
  void resetInternalGateValuesToDefaults();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoiseGateAudioProcessor)
};
