#pragma once

namespace DSP {
class LevelDetector {
public:
  LevelDetector();
  ~LevelDetector();

  void prepare(double sampleRate);
  float process(float sample);
  void reset();

private:
  float silenceFloor = -100.0f;
  float attackDetectionTimeMs = 1.0f;
  float releaseDetectionTimeMs = 100.0f;
  double currentSampleRate = 0;

  float attackDetectorCoefficient = 0.0f;
  float releaseDetectorCoeffiecient = 0.0f;

  float currentEnvelopeDb = silenceFloor;
  float previousSmoothedAttackOutput = silenceFloor;
  float previousSmoothedReleaseOutput = silenceFloor;
};

} // namespace DSP
