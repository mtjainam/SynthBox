#pragma once
#include <JuceHeader.h>
#include "PluginProcessor.h"

//==============================================================================

class OscilloscopeComponent : public juce::Component, private juce::Timer
{
public:
    OscilloscopeComponent(std::atomic<float>& src) : source(src) { startTimerHz(60); }

    void paint (juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);
        g.setColour(juce::Colours::green);
        const float midY = getHeight()/2.0f;
        juce::Path p;
        p.startNewSubPath(0, midY);
        for(size_t i=0;i<points.size();++i)
            p.lineTo((float)i, midY - points[i]*midY*0.9f);
        g.strokePath(p, juce::PathStrokeType(1.5f));
    }

private:
    void timerCallback() override
    {
        float s = source.load();
        points.push_back(s);
        if(points.size()>getWidth()) points.erase(points.begin());
        repaint();
    }

    std::atomic<float>& source;
    std::vector<float> points;
};

//==============================================================================

class Distortion1AudioProcessorEditor : public juce::AudioProcessorEditor,
                                        public juce::Slider::Listener
{
public:
    Distortion1AudioProcessorEditor (Distortion1AudioProcessor&);
    ~Distortion1AudioProcessorEditor() override = default;

    void paint (juce::Graphics&) override;
    void resized() override;
    void sliderValueChanged (juce::Slider* s) override;

private:
    Distortion1AudioProcessor& audioProcessor;

    juce::Slider fine[4], oct[4], semi[4], mix[4];
    juce::Label  fineL[4], octL[4], semiL[4], mixL[4];
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> fineA[4], octA[4], semiA[4], mixA[4];

    OscilloscopeComponent scope;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Distortion1AudioProcessorEditor)
};
