#include "APCommon.h"
#include "PluginProcessor.h"
#include "PluginEditor.h"


juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new APSatur(); }
const juce::String APSatur::getName() const { return JucePlugin_Name; }
bool APSatur::acceptsMidi() const { return false; }
bool APSatur::producesMidi() const { return false; }
bool APSatur::isMidiEffect() const { return false; }
double APSatur::getTailLengthSeconds() const { return 0.0; }
int APSatur::getNumPrograms() { return 1; }
int APSatur::getCurrentProgram() { return 0; }
void APSatur::setCurrentProgram(int index) { index; }

const juce::String APSatur::getProgramName(int index) {
    index;
    return {};
}

void APSatur::changeProgramName (int index, const juce::String& newName) {
    index;
    newName;
}

bool APSatur::hasEditor() const { return true; }

void APSatur::releaseResources() {}

bool APSatur::isBusesLayoutSupported(const BusesLayout& layouts) const {
    layouts;
    return true;
}

juce::AudioProcessorEditor* APSatur::createEditor() { return new GUI (*this); }

void APSatur::getStateInformation (juce::MemoryBlock& destData) {
    std::unique_ptr<juce::XmlElement> xml (apvts.state.createXml());
    copyXmlToBinary (*xml, destData);
}

void APSatur::setStateInformation (const void* data, int sizeInBytes) {
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml != nullptr)
    {
        if (xml->hasTagName (apvts.state.getType()))
        {
            apvts.state = juce::ValueTree::fromXml (*xml);
        }
    }
}
