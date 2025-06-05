#include "Utils.h"
#include <cmath>

namespace DSP {
namespace Utils {
float msToSamples(float ms, double sampleRate) {
  return (ms / 1000.0f) * static_cast<float>(sampleRate);
}

float calculateSmoothingCoefficient(float ms, double sampleRate) {
  return static_cast<float>(std::exp(-1.0f / msToSamples(ms, sampleRate)));
}

float calculateOnePoleSmoothedOutput(float currentValue, float targetValue,
                                     float coefficient) {
  return coefficient * currentValue + (1.0f - coefficient) * targetValue;
}
} // namespace Utils
} // namespace DSP
