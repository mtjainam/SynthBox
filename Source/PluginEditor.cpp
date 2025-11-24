#include "PluginProcessor.h"
#include "PluginEditor.h"

Distortion1AudioProcessorEditor::Distortion1AudioProcessorEditor (Distortion1AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), scope(p.latestSample)
{
    setSize (850, 500);
    addAndMakeVisible(scope);

    auto make = [&](juce::Slider& s, juce::Label& l, const juce::String& txt,
                    float min,float max,float step)
    {
        s.setRange(min,max,step);
        s.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        s.setTextBoxStyle(juce::Slider::TextBoxBelow,true,45,18);
        addAndMakeVisible(s);
        l.setText(txt,juce::dontSendNotification);
        l.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(l);
    };

    const juce::String oscIDs[4]={"SQUARE","SAW","TRIANGLE","SINE"};
    for(int i=0;i<4;++i)
    {
        make(fine[i],fineL[i],"Fine",-100,100,0.1);
        make(oct[i],octL[i],"Oct",-2,2,1);
        make(semi[i],semiL[i],"Semi",-12,12,1);
        make(mix[i],mixL[i],"Mix",0,1,0.01);

        fineA[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.parameters, oscIDs[i]+"_FINE", fine[i]);
        octA[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.parameters, oscIDs[i]+"_OCT", oct[i]);
        semiA[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.parameters, oscIDs[i]+"_SEMI", semi[i]);
        mixA[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.parameters, oscIDs[i]+"_MIX", mix[i]);
    }
}

void Distortion1AudioProcessorEditor::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.setFont(16.0f);
    g.drawText("4-Osc Synth  |  Square, Saw, Triangle, Sine", 250, 10, 500, 25, juce::Justification::centredLeft);
}

void Distortion1AudioProcessorEditor::resized()
{
    scope.setBounds(10, 50, 200, getHeight()-60);
    int xStart = 230;
    int yStart = 60;
    int rowH   = 100;
    int kSize  = 80;
    int gapX   = 100;

    for(int i=0;i<4;++i)
    {
        int y = yStart + i*rowH;
        fine[i].setBounds(xStart + 0*gapX, y, kSize, kSize);
        oct[i].setBounds (xStart + 1*gapX, y, kSize, kSize);
        semi[i].setBounds(xStart + 2*gapX, y, kSize, kSize);
        mix[i].setBounds (xStart + 3*gapX, y, kSize, kSize);

        fineL[i].setBounds(fine[i].getX(), y-20, kSize, 18);
        octL[i].setBounds (oct[i].getX(),  y-20, kSize, 18);
        semiL[i].setBounds(semi[i].getX(), y-20, kSize, 18);
        mixL[i].setBounds (mix[i].getX(),  y-20, kSize, 18);
    }
}

void Distortion1AudioProcessorEditor::sliderValueChanged (juce::Slider*) {}

