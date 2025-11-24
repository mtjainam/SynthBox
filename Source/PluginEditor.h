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

class LevelMeterComponent : public juce::Component, private juce::Timer
{
public:
    LevelMeterComponent(std::atomic<float>& src) : source(src) 
    { 
        startTimerHz(60);
        level = 0.0f;
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::black);
        
        const float width = (float)getWidth();
        const float height = (float)getHeight();
        
        // Draw level bar
        const float levelHeight = height * level;
        const float barY = height - levelHeight;
        
        // Color gradient: green -> yellow -> red
        juce::Colour barColour;
        if (level < 0.7f)
            barColour = juce::Colours::green;
        else if (level < 0.9f)
            barColour = juce::Colours::yellow;
        else
            barColour = juce::Colours::red;
        
        g.setColour(barColour);
        g.fillRect(0.0f, barY, width, levelHeight);
        
        // Draw outline
        g.setColour(juce::Colours::white.withAlpha(0.3f));
        g.drawRect(getLocalBounds().toFloat(), 1.0f);
    }

private:
    void timerCallback() override
    {
        float s = std::abs(source.load());
        // Smooth the level with a simple peak hold
        if (s > level)
            level = s;
        else
            level = level * 0.95f; // Decay
        
        repaint();
    }

    std::atomic<float>& source;
    float level;
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
        g.fillAll(juce::Colours::black);
        juce::Colour blueColour = juce::Colour(0xff4a90e2); // Blue similar to JUCE knobs
        g.setColour(isHighlighted ? blueColour : juce::Colours::white);
        
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
        
        g.strokePath(p, juce::PathStrokeType(isHighlighted ? 2.5f : 2.0f));
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
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> highlightA;
    
    WaveformPreviewComponent waveformPreviews[4];

    OscilloscopeComponent scope;
    LevelMeterComponent levelMeter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Distortion1AudioProcessorEditor)
};
