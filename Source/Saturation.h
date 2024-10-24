#pragma once
#include <cmath>

// We're not gonna have to bother with compile time so I can do this
template <float (*Func)(float)>
void performScaledSaturation(float* samples, size_t len,
                                float inputG, float outputG) {
    for(size_t i = 0; i < len; i++)
        samples[i] = outputG * Func(inputG * samples[i]);
}

template <float (*Func)(float)>
void performSaturation(float* samples, size_t len) {
    for(size_t i = 0; i < len; i++)
        samples[i] = Func(samples[i]);
}

inline float doSquaredSine(float sample) {
    return sample > 0 ? std::sin(sample * sample) : -std::sin(sample * sample);
}

constexpr float threshold = .9f;
inline float doFold(float sample) {
    sample = std::fmod(sample, 2.0f * threshold);

    if (sample > threshold)
        return 2.0f * threshold - sample;
    if (sample < -threshold)
        return -2.0f * threshold - sample;

    return sample;
}

inline float doCube(float sample) {
    return std::sin(std::cbrt(sample) * .6f);
}

inline float doSqrt(float sample) {
    sample = sample > 0 ? sqrtf(sample) : -sqrtf(-sample);
    sample *= .6f;
    return sample / powf(1 + powf(sample, 8), 1.0 / 8);
}

inline float doLog(float sample) {
    sample = sample > 0 ? std::log(1 + sample) : -std::log(1 - sample);
    sample *= .6f;
    return sample / powf(1 + powf(sample, 8), 1.0 / 8);
}

inline float doHard(float sample) {
    if(sample < -1e4f) return -1;
    if(sample > 1e4f) return 1;
    return sample / powf(1 + powf(sample, 8), 1.0 / 8);
}

inline float doSine(float sample) {
    return std::sin(sample);
}

inline float doTanhStandard(float sample) {
    return std::tanh(sample);
}

inline float doAsym(float sample) {
    if(sample < 0)
        sample = -powf(sample, 8);

    if(sample < -1e4f) return -1;
    if(sample > 1e4f) return 1;
    return sample / powf(1 + powf(sample, 8), 1.f / 8);
}