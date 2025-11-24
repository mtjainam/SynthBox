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
    
    WaveformPreviewComponent(WaveformType type, int index) 
        : waveformType(type), waveformIndex(index) 
    {
        setMouseCursor(juce::MouseCursor::PointingHandCursor);
    }
    
    void setHighlighted(bool highlighted) 
    { 
        isHighlighted = highlighted; 
        repaint(); 
    }
    
    void setOnClickCallback(std::function<void(int)> callback) 
    { 
        onClickCallback = callback; 
    }
    
    void paint(juce::Graphics& g) override
    {
        // Draw background with highlight
        if (isHighlighted)
        {
            g.setColour(juce::Colours::yellow.withAlpha(0.3f));
            g.fillRoundedRectangle(getLocalBounds().toFloat(), 4.0f);
            g.setColour(juce::Colours::yellow);
            g.drawRoundedRectangle(getLocalBounds().toFloat(), 4.0f, 2.0f);
        }
        else
        {
            g.fillAll(juce::Colours::black);
        }
        
        g.setColour(isHighlighted ? juce::Colours::yellow : juce::Colours::white);
        
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
        
        g.strokePath(p, juce::PathStrokeType(isHighlighted ? 3.0f : 2.0f));
    }
    
    void mouseDown(const juce::MouseEvent&) override
    {
        if (onClickCallback)
            onClickCallback(waveformIndex);
    }
    
private:
    WaveformType waveformType;
    int waveformIndex;
    bool isHighlighted = false;
    std::function<void(int)> onClickCallback;
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
    void updateWaveformHighlights();

private:
    Distortion1AudioProcessor& audioProcessor;

    juce::Slider oct[4], semi[4], mix[4];
    juce::Label  octL[4], semiL[4], mixL[4];
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> octA[4], semiA[4], mixA[4];
    
    juce::Slider highlightKnob;
    juce::Label highlightLabel;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> highlightA;
    
    WaveformPreviewComponent waveformPreviews[4];

    OscilloscopeComponent scope;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Distortion1AudioProcessorEditor)
};
