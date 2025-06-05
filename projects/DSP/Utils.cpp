#include "Utils.h"
#include <cmath>

namespace DSP {
Utils::Utils() {}
Utils::~Utils() {}

float Utils::msToSamples(float ms, double sampleRate) {
  return (ms / 1000) * static_cast<float>(sampleRate);
}

float Utils::calculateSmoothingCoefficient(float ms, double sampleRate) {
  return static_cast<float>(std::exp(-1.0 / msToSamples(ms, sampleRate)));
}

float Utils::calculateOnePoleSmoothedOutput(float currentValue,
                                            float targetValue,
                                            float coefficient) {
  return coefficient * currentValue + (1.0 - coefficient) * targetValue;
}

} // namespace DSP
