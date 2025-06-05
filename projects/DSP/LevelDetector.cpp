#include "LevelDetector.h"
#include "Utils.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include <algorithm>
#include <cmath>

namespace DSP {
LevelDetector::LevelDetector() {}
LevelDetector::~LevelDetector() {}

void LevelDetector::prepare(double sampleRate) {
  currentSampleRate = sampleRate;
  attackDetectorCoefficient = DSP::Utils::calculateSmoothingCoefficient(
      attackDetectionTimeMs, sampleRate);
  releaseDetectorCoefficient = DSP::Utils::calculateSmoothingCoefficient(
      releaseDetectionTimeMs, sampleRate);
  reset();
}

float LevelDetector::process(float sample) {
  auto inputLevelDb =
      std::max(juce::Decibels::gainToDecibels(std::abs(sample)), silenceFloor);

  auto smoothedAttackOutput = DSP::Utils::calculateOnePoleSmoothedOutput(
      previousSmoothedAttackOutput, inputLevelDb, attackDetectorCoefficient);

  auto smoothedReleaseOutput = DSP::Utils::calculateOnePoleSmoothedOutput(
      previousSmoothedReleaseOutput, inputLevelDb, releaseDetectorCoefficient);

  currentEnvelopeDb = (inputLevelDb > currentEnvelopeDb)
                          ? smoothedAttackOutput
                          : smoothedReleaseOutput;

  previousSmoothedAttackOutput = smoothedAttackOutput;
  previousSmoothedReleaseOutput = smoothedReleaseOutput;

  return currentEnvelopeDb;
}

void LevelDetector::reset() {
  currentEnvelopeDb = silenceFloor;
  previousSmoothedAttackOutput = silenceFloor;
  previousSmoothedReleaseOutput = silenceFloor;
}

} // namespace DSP
