#include <cmath>
#include <algorithm>
#include <complex>
#include <array>

#include "APCommon.h"
#include "PluginProcessor.h"
#include "Saturation.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif


APSatur::APSatur()
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


void APSatur::prepareToPlay(double sampleRate, int samplesPerBlock) {
    startOversampler(sampleRate, samplesPerBlock);
}


float APSatur::getFloatKnobValue(ParameterNames parameter) const {
    return parameterList[static_cast<int>(parameter)]->get();
}


void APSatur::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    midiMessages;
    juce::ScopedNoDenormals noDenormals;

    const int inputs = getTotalNumInputChannels();
    const int outputs = getTotalNumOutputChannels();

    const float inputGainValueKnob  = decibelsToGain(getFloatKnobValue(ParameterNames::inGain));
    const float outputGainValueKnob = decibelsToGain(getFloatKnobValue(ParameterNames::outGain));

    juce::dsp::AudioBlock<float> originalBlock(buffer);
    juce::dsp::AudioBlock<float> mainBlock;
    
    if (inputs < 1) return;
    
    if (inputs == 1) mainBlock = originalBlock.getSingleChannelBlock(0);
    
    if (inputs > 1) mainBlock = originalBlock.getSubsetChannelBlock(0, 2);
    
    std::shared_ptr<juce::dsp::Oversampling<float>> os(oversampler);
    if(!os) return;

    /**
     * Solve : inputGain * x^samples = newInputGain
     * x = (inputGain/newInputGain)^(1/samples)
     */
    size_t n = mainBlock.getNumSamples();
    if(inputGain == inputGainValueKnob) {
        mainBlock.multiplyBy(inputGain);
    } else {
        float iStep = std::pow(inputGain / inputGainValueKnob, 1.f / n);
        for (int channel = 0; channel < 2 && channel < inputs; channel++) {
            float gain = inputGain;
            float* ptr = mainBlock.getChannelPointer(channel);
            for(size_t i = 0; i < n; i++) {
                ptr[i] *= gain;
                gain *= iStep;
            }
        }
        inputGain = inputGainValueKnob;
    }

    juce::dsp::AudioBlock<float> oversampledBlock = os->processSamplesUp(mainBlock);
    
    const int selection = static_cast<int>(getFloatKnobValue(ParameterNames::selection));
  
    float* channelData[2];
    
    for (int i = 0; i < inputs && i < 2; i++) channelData[i] = oversampledBlock.getChannelPointer(i);
    
    size_t samples = oversampledBlock.getNumSamples();

    for (int channel = 0; channel < 2 && channel < inputs; channel++) {
        switch (selection) {
            case static_cast<int>(ButtonName::tanh):
                performSaturation<doTanhStandard>(channelData[channel], samples);
                break;

            case static_cast<int>(ButtonName::sine):
                performSaturation<doSine>(channelData[channel], samples);
                break;

            case static_cast<int>(ButtonName::hard):
                performSaturation<doHard>(channelData[channel], samples);
                break;

            case static_cast<int>(ButtonName::log):
                performSaturation<doLog>(channelData[channel], samples);
                break;

            case static_cast<int>(ButtonName::sqrt):
                performSaturation<doSqrt>(channelData[channel], samples);
                break;

            case static_cast<int>(ButtonName::cube):
                performSaturation<doCube>(channelData[channel], samples);
                break;

            case static_cast<int>(ButtonName::fold):
                performSaturation<doFold>(channelData[channel], samples);
                break;

            case static_cast<int>(ButtonName::squaredSine):
                performSaturation<doSquaredSine>(channelData[channel], samples);
                break;

            case static_cast<int>(ButtonName::asymmetricExp):
                performSaturation<doAsym>(channelData[channel], samples);
                break;
        }
    }

    os->processSamplesDown (mainBlock);

    // Same as input gain
    if(outputGain == outputGainValueKnob) {
        mainBlock.multiplyBy(outputGain);
    } else {
        float oStep = std::pow(outputGain / outputGainValueKnob, 1.f / n);
        for (int channel = 0; channel < 2 && channel < inputs; channel++) {
            float gain = outputGain;
            float* ptr = mainBlock.getChannelPointer(channel);
            for(size_t i = 0; i < n; i++) {
                ptr[i] *= gain;
                gain *= oStep;
            }
        }
        outputGain = outputGainValueKnob;
    }
}

void APSatur::startOversampler(double sampleRate, int samplesPerBlock) {
    sampleRate;
    
    oversampler = std::make_shared<juce::dsp::Oversampling<float>>(2, 3, juce::dsp::Oversampling<float>::filterHalfBandFIREquiripple);
    
    oversampler->initProcessing(static_cast<size_t>(samplesPerBlock));
    oversampler->reset();
    
    setLatencySamples(static_cast<int>(ceilf(oversampler->getLatencyInSamples())));
}
