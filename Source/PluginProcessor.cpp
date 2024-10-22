#include <cmath>
#include <algorithm>
#include <complex>
#include <array>

#include "APCommon.h"
#include "PluginProcessor.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


APComp::APComp()
: AudioProcessor(BusesProperties()
                 .withInput("Input", juce::AudioChannelSet::stereo(), true)
                 .withOutput("Output", juce::AudioChannelSet::stereo(), true)),
apvts(*this, nullptr, "PARAMETERS", createParameterLayout()),
inputGain(0),
outputGain(0),
parameterList(static_cast<int>(ParameterNames::END) + 1) {
        
    for (int i = 0; i < static_cast<int>(ParameterNames::END); ++i) {
        
        parameterList[i] = static_cast<juce::AudioParameterFloat*>(apvts.getParameter(queryParameter(static_cast<ParameterNames>(i)).id));
    }
}


void APComp::prepareToPlay(double sampleRate, int samplesPerBlock) {
    
    oversamplerReady.store(false);
    
    startOversampler(sampleRate, samplesPerBlock);
}


float APComp::getFloatKnobValue(ParameterNames parameter) const {
    return parameterList[static_cast<int>(parameter)]->get();
}


void APComp::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    midiMessages;
    juce::ScopedNoDenormals noDenormals;

    const int inputs = getTotalNumInputChannels();
    const int outputs = getTotalNumOutputChannels();

    constexpr float gainSmoothingCoefficient = 0.2f;
    
    const float inputGainValueKnob  = getFloatKnobValue(ParameterNames::inGain);
    const float outputGainValueKnob = getFloatKnobValue(ParameterNames::outGain);
    
    for (auto i = inputs; i < outputs; ++i) buffer.clear(i, 0, buffer.getNumSamples());

    if (!oversamplerReady.load()) return;

    juce::dsp::AudioBlock<float> originalBlock(buffer);
    juce::dsp::AudioBlock<float> mainBlock;
    
    if (inputs < 1) return;
    
    if (inputs == 1) mainBlock = originalBlock.getSingleChannelBlock(0);
    
    if (inputs > 1) mainBlock = originalBlock.getSubsetChannelBlock(0, 2);
    
    juce::dsp::AudioBlock<float> oversampledBlock = oversampler->processSamplesUp(mainBlock);
    
    const int selection = static_cast<int>(getFloatKnobValue(ParameterNames::selection));
  
    float* channelData[2];
    
    for (int i = 0; i < inputs && i < 2; i++) channelData[i] = oversampledBlock.getChannelPointer(i);
    
    size_t samples = oversampledBlock.getNumSamples();
    // TODO fix that smoothing
    inputGain = inputGainValueKnob * gainSmoothingCoefficient + (1 - gainSmoothingCoefficient) * inputGain;
    outputGain = outputGainValueKnob * gainSmoothingCoefficient + (1 - gainSmoothingCoefficient) * outputGain;

    for (int channel = 0; channel < 2 && channel < inputs; channel++) {
        switch (selection) {
            case static_cast<int>(ButtonName::tanh):
                doTanhStandard(channelData[channel], samples);
                break;
                    
            case static_cast<int>(ButtonName::sine):
                doSine(channelData[channel], samples);
                break;
                    
            case static_cast<int>(ButtonName::hard):
                doHard(channelData[channel], samples);
                break;
                    
            case static_cast<int>(ButtonName::log):
                doLog(channelData[channel], samples);
                break;
                    
            case static_cast<int>(ButtonName::sqrt):
                doSqrt(channelData[channel], samples);
                break;
                    
            case static_cast<int>(ButtonName::cube):
                doCube(channelData[channel], samples);
                break;
                   
            case static_cast<int>(ButtonName::fold):
                doFold(channelData[channel], samples);
                break;
                    
            case static_cast<int>(ButtonName::squaredSine):
                doSquaredSine(channelData[channel], samples);
                break;
        }
    }
    
    oversampler->processSamplesDown (mainBlock);
}


void APComp::doSine (float* samples, size_t len) {
    float igain = decibelsToGain(inputGain), ogain = decibelsToGain(outputGain);
    for (int i = 0; i < len; i++)
        samples[i] = std::sin(samples[i] * igain) * ogain;
}


void APComp::doTanhStandard (float* samples, size_t len) {
    float igain = decibelsToGain(inputGain), ogain = decibelsToGain(outputGain);

    for (int i = 0; i < len; i++)
        samples[i] = std::tanh(samples[i] * igain) * ogain;
}


void APComp::doHard (float* samples, size_t len) {
    float igain = decibelsToGain(inputGain), ogain = decibelsToGain(outputGain);

    for (int i = 0; i < len; i++) {
        samples[i] *= igain;
        samples[i] = ogain * samples[i] / powf(1 + powf(samples[i], 8), 1.0f / 8);
    }
}


void APComp::doLog (float* samples, size_t len) {
    float igain = decibelsToGain(inputGain), ogain = decibelsToGain(outputGain);

    for (int i = 0; i < len; i++) {
        samples[i] *= igain;
        samples[i] = samples[i] > 0 ? std::log(1 + samples[i]) : -std::log(1 - samples[i]);
        samples[i] *= .6f;
        samples[i] = ogain * samples[i] / powf(1 + powf(samples[i], 8), 1.0 / 8);
    }
}


void APComp::doSqrt (float* samples, size_t len) {
    float igain = decibelsToGain(inputGain), ogain = decibelsToGain(outputGain);

    for (int i = 0; i < len; i++) {
        samples[i] *= igain;
        samples[i] = samples[i] > 0 ? std::sqrtf(samples[i]) : -std::sqrtf(-samples[i]);
        samples[i] *= .6f;
        samples[i] = ogain * samples[i] / powf(1 + powf(samples[i], 8), 1.0 / 8);
    }
}


void APComp::doCube (float* samples, size_t len) {
    float igain = decibelsToGain(inputGain), ogain = decibelsToGain(outputGain);

    for (int i = 0; i < len; i++) {
        samples[i] *= igain;
        samples[i] = std::sin(std::cbrt(samples[i]) * .6f) * ogain;
    }
}


void APComp::doFold (float* samples, size_t len) {
    float igain = decibelsToGain(inputGain), ogain = decibelsToGain(outputGain);

    float threshold = 0.9f;
    for (int i = 0; i < len; i++) {
        samples[i] *= igain;
        samples[i] = std::fmod(samples[i], 2.0f * threshold);

        if (samples[i] > threshold) {
            samples[i] = 2.0f * threshold - samples[i];
        }
        else if (samples[i] < -threshold) {
            samples[i] = -2.0f * threshold - samples[i];
        }
        samples[i] *= ogain;
    }
}


void APComp::doSquaredSine (float* samples, size_t len) {
    float igain = decibelsToGain(inputGain), ogain = decibelsToGain(outputGain);

    for (int i = 0; i < len; i++) {
        samples[i] *= igain;

        samples[i] = samples[i] > 0 ? std::sin(samples[i] * samples[i]) : -std::sin(samples[i] * samples[i]);

        samples[i] *= ogain;
    }
}


void APComp::startOversampler(double sampleRate, int samplesPerBlock) {
    sampleRate;
    
    oversampler.reset();
    
    oversampler = std::make_unique<juce::dsp::Oversampling<float>>(2, 3, juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple);
    
    oversampler->initProcessing(static_cast<size_t>(samplesPerBlock));
    oversampler->reset();
    
    setLatencySamples(static_cast<int>(std::ceilf(oversampler->getLatencyInSamples())));
         
    oversamplerReady.store(true);
}
