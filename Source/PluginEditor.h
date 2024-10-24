#pragma once

#include "PluginProcessor.h"

constexpr int ioRow1 = 349, ioRow2 = 430, ioColumn = 101;
constexpr int ioRadius = 34;

constexpr float selectionColumn = 259;
constexpr float selectionFirstY = 159, selectionLastY = 446;
constexpr float numberOfSelections = 8;
constexpr float selectionRadius = 18;
constexpr float spacingY = (selectionLastY - selectionFirstY) / (numberOfSelections - 1);

constexpr int scopeL = 37, scopeR = 189,
                scopeT = 132, scopeB = 274;

constexpr int mathL = 230, mathR = 451,
        mathT = 20, mathB = 70;

class GUI  : public juce::AudioProcessorEditor, private juce::Timer {
  public:
    GUI (APSatur&);
    ~GUI() override;
    
    void paint (juce::Graphics&) override;
    void resized() override;
    void timerCallback() override;
    void mouseDown (const juce::MouseEvent& event) override;
    void mouseDrag (const juce::MouseEvent& event) override;
    void mouseUp (const juce::MouseEvent& event) override;
    ButtonName determineButton(const juce::MouseEvent &event);
    
  private:
    APSatur& audioProcessor;
                
    juce::Image backgroundImage;
    
    juce::Image tanhImage;
    juce::Image hardImage;
    juce::Image logImage;
    juce::Image foldImage;
    juce::Image squaredSineImage;
    juce::Image sineImage;
    juce::Image sqrtImage;
    juce::Image cubeImage;

    juce::Font customTypeface;
        
    juce::Slider inGainSlider;
    juce::Slider outGainSlider;
    juce::Slider selectionSlider;
            
    std::vector<std::pair<std::string, std::reference_wrapper<juce::Slider>>> sliders {
        {"inGainSlider",        std::ref(inGainSlider)},
        {"outGainSlider",       std::ref(outGainSlider)},
        {"selectionSlider",     std::ref(selectionSlider)},
    };

    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> inGainAttachment, outGainAttachment, selectionAttachment;
    
    float previousMouseY = 0;
    
    ButtonName currentButtonSelection;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (GUI)
};
