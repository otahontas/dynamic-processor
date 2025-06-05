#pragma once

namespace DSP {
class Utils {
public:
  Utils();
  ~Utils();

  static float msToSamples(float timeMs, double sampleRate);
  static float calculateSmoothingCoefficient(float ms, double sampleRate);
  static float calculateOnePoleSmoothedOutput(float currenValue,
                                              float targetValue,
                                              float coefficient);
};
} // namespace DSP
