#include "LevelDetector.h"
#include "Utils.h"
#include "juce_audio_basics/juce_audio_basics.h"
#include <algorithm>

namespace DSP {
LevelDetector::LevelDetector() {}
LevelDetector::~LevelDetector() {}

void LevelDetector::prepare(double sampleRate) {
  currentSampleRate = sampleRate;
  attackDetectorCoefficient = DSP::Utils::calculateSmoothingCoefficient(
      attackDetectionTimeMs, sampleRate);
  releaseDetectorCoeffiecient = DSP::Utils::calculateSmoothingCoefficient(
      releaseDetectionTimeMs, sampleRate);
  reset();
}

float LevelDetector::process(float sample) {
  float flooredInputDb =
      std::max(juce::Decibels::gainToDecibels(std::abs(sample)), silenceFloor);

  auto smootherCoefficient = flooredInputDb > currentEnvelopeDb
                                 ? attackDetectorCoefficient
                                 : releaseDetectorCoeffiecient;
  currentEnvelopeDb = DSP::Utils::calculateOnePoleSmoothedOutput(
      currentEnvelopeDb, flooredInputDb, smootherCoefficient);
  return currentEnvelopeDb;
}

void LevelDetector::reset() {
  currentEnvelopeDb = silenceFloor;
  previousSmoothedAttackOutput = silenceFloor;
  previousSmoothedReleaseOutput = silenceFloor;
}

} // namespace DSP
