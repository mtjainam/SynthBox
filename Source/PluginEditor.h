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

class WaveformPreviewComponent : public juce::Component
{
public:
    enum WaveformType { SQUARE, SAW, TRIANGLE, SINE };
    
    WaveformPreviewComponent(WaveformType type) : waveformType(type) {}
    
    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);
        g.setColour(juce::Colours::white);
        
        const float width = (float)getWidth();
        const float height = (float)getHeight();
        const float midY = height * 0.5f;
        const float amplitude = height * 0.4f;
        
        juce::Path p;
        const int numPoints = 100;
        
        switch(waveformType)
        {
            case SQUARE:
                p.startNewSubPath(0, midY - amplitude);
                p.lineTo(width * 0.5f, midY - amplitude);
                p.lineTo(width * 0.5f, midY + amplitude);
                p.lineTo(width, midY + amplitude);
                break;
                
            case SAW:
                p.startNewSubPath(0, midY - amplitude);
                p.lineTo(width, midY + amplitude);
                break;
                
            case TRIANGLE:
                p.startNewSubPath(0, midY + amplitude);
                p.lineTo(width * 0.5f, midY - amplitude);
                p.lineTo(width, midY + amplitude);
                break;
                
            case SINE:
                for(int i = 0; i <= numPoints; ++i)
                {
                    float x = (float)i / numPoints * width;
                    float y = midY - amplitude * std::sin(juce::MathConstants<float>::twoPi * i / numPoints);
                    if(i == 0)
                        p.startNewSubPath(x, y);
                    else
                        p.lineTo(x, y);
                }
                break;
        }
        
        g.strokePath(p, juce::PathStrokeType(2.0f));
    }
    
private:
    WaveformType waveformType;
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

    juce::Slider oct[4], semi[4], mix[4];
    juce::Label  octL[4], semiL[4], mixL[4];
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> octA[4], semiA[4], mixA[4];
    
    WaveformPreviewComponent waveformPreviews[4];

    OscilloscopeComponent scope;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Distortion1AudioProcessorEditor)
};
