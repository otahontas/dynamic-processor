#pragma once

#include <JuceHeader.h>
namespace Param {
namespace ID {
// gate
static const juce::String GateThreshold{"gate_threshold"};
static const juce::String GateAttack{"gate_attack"};
static const juce::String GateHold{"gate_hold"};
static const juce::String GateRelease{"gate_release"};

// TODO: overall output gain?

// generic
static const juce::String Enabled{"enabled"};
} // namespace ID

namespace Name {

// gate
static const juce::String GateThreshold{"Gate Threshold"};
static const juce::String GateAttack{"Gate Attack"};
static const juce::String GateHold{"Gate Hold"};
static const juce::String GateRelease{"Gate Release"};

// generic
static const juce::String Enabled{"Enabled"};

} // namespace Name

namespace Ranges {
// TODO: check good skew and inc values
// Gate
static constexpr float GateThresholdMin{-96.f};
static constexpr float GateThresholdMax{0.0f};
static constexpr float GateThresholdInc{0.1f};
static constexpr float GateThresholdSkw{2.0f};

static constexpr float GateAttackMin{1.f};
static constexpr float GateAttackMax{1000.f};
static constexpr float GateAttackInc{0.1f};
static constexpr float GateAttackSkw{0.3f};

static constexpr float GateHoldMin{0.0f};
static constexpr float GateHoldMax{2000.f};
static constexpr float GateHoldInc{0.1f};
static constexpr float GateHoldSkw{0.3f};

static constexpr float GateReleaseMin{1.0f};
static constexpr float GateReleaseMax{5000.f};
static constexpr float GateReleaseInc{0.1f};
static constexpr float GateReleaseSkw{0.3f};

// Generic
static const juce::String EnabledOff{"Off"};
static const juce::String EnabledOn{"On"};

} // namespace Ranges
namespace Defaults {

// gate
static constexpr float GateThresholdDefault{-40.0f};
static constexpr float GateAttackDefault{5.0f};
static constexpr float GateHoldDefault{10.0f};
static constexpr float GateReleaseDefault{50.0f};

// generic
static constexpr bool EnabledDefault{true};

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

  // gate stuff
  float gateEnvelope = 0.0f;
  float gateCurrentGain = 0.0f;
  float gateThresholdLinear = 0.0f;
  float gateAttackCoeff = 0.0f;
  float gateReleaseCoeff = 0.0f;
  float gateHoldSamples = 0.0f;
  int gateHoldCounter = 0;

  // (gate) coeffs for the envelope detector
  float detectorAttackCoeff = 0.0f;
  float detectorReleaseCoeff = 0.0f;

  // generic
  double currentSampleRate = 0;
  bool isProcessorEnabled = true;

  // helpers
  float calculateCoefficient(float timeMs, double sr);

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(NoiseGateAudioProcessor)
};
