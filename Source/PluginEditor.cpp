#include "PluginProcessor.h"
#include "PluginEditor.h"

Distortion1AudioProcessorEditor::Distortion1AudioProcessorEditor (Distortion1AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), scope(p.latestSample)
{
    setSize (850, 500);
    setResizable(true, true);
    setResizeLimits(600, 400, 2000, 1200);
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
    
    // Scale font size based on window size
    const float baseWidth = 850.0f;
    const float scale = getWidth() / baseWidth;
    const float fontSize = 16.0f * scale;
    g.setFont(fontSize);
    
    const int textX = (int)(250 * scale);
    const int textY = (int)(10 * scale);
    const int textW = (int)(500 * scale);
    const int textH = (int)(25 * scale);
    g.drawText("SynthBox", textX, textY, textW, textH, juce::Justification::centredLeft);
}

void Distortion1AudioProcessorEditor::resized()
{
    // Base dimensions (original size)
    const int baseWidth = 850;
    const int baseHeight = 500;
    
    // Current dimensions
    const int currentWidth = getWidth();
    const int currentHeight = getHeight();
    
    // Calculate scale factors (use the smaller scale to maintain aspect ratio better)
    const float scaleX = currentWidth / (float)baseWidth;
    const float scaleY = currentHeight / (float)baseHeight;
    const float scale = std::min(scaleX, scaleY);
    
    // Scale all dimensions
    const int scopeWidth = (int)(200 * scale);
    const int scopeX = (int)(10 * scale);
    const int scopeY = (int)(50 * scale);
    const int scopeHeight = currentHeight - (int)(60 * scale);
    
    scope.setBounds(scopeX, scopeY, scopeWidth, scopeHeight);
    
    const int xStart = (int)(230 * scale);
    const int yStart = (int)(60 * scale);
    const int rowH = (int)(100 * scale);
    const int kSize = (int)(80 * scale);
    const int gapX = (int)(100 * scale);
    const int labelHeight = (int)(18 * scale);
    const int labelOffset = (int)(20 * scale);
    const int textBoxWidth = (int)(45 * scale);
    const int textBoxHeight = (int)(18 * scale);

    for(int i=0;i<4;++i)
    {
        int y = yStart + i*rowH;
        fine[i].setBounds(xStart + 0*gapX, y, kSize, kSize);
        oct[i].setBounds (xStart + 1*gapX, y, kSize, kSize);
        semi[i].setBounds(xStart + 2*gapX, y, kSize, kSize);
        mix[i].setBounds (xStart + 3*gapX, y, kSize, kSize);
        
        // Update text box sizes for scaling
        fine[i].setTextBoxStyle(juce::Slider::TextBoxBelow, true, textBoxWidth, textBoxHeight);
        oct[i].setTextBoxStyle(juce::Slider::TextBoxBelow, true, textBoxWidth, textBoxHeight);
        semi[i].setTextBoxStyle(juce::Slider::TextBoxBelow, true, textBoxWidth, textBoxHeight);
        mix[i].setTextBoxStyle(juce::Slider::TextBoxBelow, true, textBoxWidth, textBoxHeight);

        fineL[i].setBounds(fine[i].getX(), y-labelOffset, kSize, labelHeight);
        octL[i].setBounds (oct[i].getX(),  y-labelOffset, kSize, labelHeight);
        semiL[i].setBounds(semi[i].getX(), y-labelOffset, kSize, labelHeight);
        mixL[i].setBounds (mix[i].getX(),  y-labelOffset, kSize, labelHeight);
    }
}

void Distortion1AudioProcessorEditor::sliderValueChanged (juce::Slider*) {}

