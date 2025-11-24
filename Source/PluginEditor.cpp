#include "PluginProcessor.h"
#include "PluginEditor.h"

Distortion1AudioProcessorEditor::Distortion1AudioProcessorEditor (Distortion1AudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), scope(p.latestSample), levelMeter(p.latestSample),
      waveformPreviews{ WaveformPreviewComponent(WaveformPreviewComponent::SQUARE, 0),
                        WaveformPreviewComponent(WaveformPreviewComponent::SAW, 1),
                        WaveformPreviewComponent(WaveformPreviewComponent::TRIANGLE, 2),
                        WaveformPreviewComponent(WaveformPreviewComponent::SINE, 3) }
{
    setSize (850, 500);
    setResizable(true, true);
    setResizeLimits(600, 400, 2000, 1200);
    addAndMakeVisible(scope);
    addAndMakeVisible(levelMeter);
    
    for(int i=0; i<4; ++i)
    {
        addAndMakeVisible(waveformPreviews[i]);
        waveformPreviews[i].setOnClickCallback([this](int index) {
            highlightKnob.setValue(index);
        });
    }

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
        oct[i].setRange(-2, 2, 1);
        oct[i].setSliderStyle(juce::Slider::RotaryVerticalDrag);
        oct[i].setTextBoxStyle(juce::Slider::TextBoxBelow, true, 45, 18);
        oct[i].setTextValueSuffix("");
        addAndMakeVisible(oct[i]);
        octL[i].setText("Oct", juce::dontSendNotification);
        octL[i].setJustificationType(juce::Justification::centred);
        addAndMakeVisible(octL[i]);
        make(semi[i],semiL[i],"Semi",-12,12,1);
        make(mix[i],mixL[i],"Mix",0,1,0.01);

        octA[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.parameters, oscIDs[i]+"_OCT", oct[i]);
        semiA[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.parameters, oscIDs[i]+"_SEMI", semi[i]);
        mixA[i] = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            audioProcessor.parameters, oscIDs[i]+"_MIX", mix[i]);
    }
    
    // Highlight knob (invisible but functional for OSC)
    highlightKnob.setRange(0, 3, 1);
    highlightKnob.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    highlightKnob.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    highlightKnob.setVisible(false);
    addChildComponent(highlightKnob);
    
    highlightA = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
        audioProcessor.parameters, "HIGHLIGHT_OSC", highlightKnob);
    
    // Listen to highlight knob changes
    highlightKnob.addListener(this);
    updateWaveformHighlights();
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
    const int meterWidth = (int)(20 * scale);
    const int meterX = scopeX + scopeWidth + (int)(5 * scale);
    
    scope.setBounds(scopeX, scopeY, scopeWidth, scopeHeight);
    levelMeter.setBounds(meterX, scopeY, meterWidth, scopeHeight);
    
    // Calculate available width for knobs and waveforms
    const int controlsStartX = meterX + meterWidth + (int)(20 * scale);
    const int controlsEndX = currentWidth - (int)(20 * scale);
    const int availableWidth = controlsEndX - controlsStartX;
    
    const int yStart = (int)(60 * scale);
    const int rowH = (int)(100 * scale);
    const int kSize = (int)(80 * scale);
    const int waveformWidth = (int)(120 * scale);
    const int waveformHeight = (int)(60 * scale);
    
    // Calculate spacing: 3 knobs + 1 waveform, distribute evenly
    const int totalItemsWidth = 3 * kSize + waveformWidth;
    const int totalGaps = 4; // 3 gaps between knobs + 1 gap before waveform
    const int gapX = (availableWidth - totalItemsWidth) / totalGaps;
    
    const int labelHeight = (int)(18 * scale);
    const int labelOffset = (int)(20 * scale);
    const int textBoxWidth = (int)(45 * scale);
    const int textBoxHeight = (int)(18 * scale);
    
    // Position highlight knob (invisible, positioned off-screen but functional)
    highlightKnob.setBounds(-100, -100, 1, 1);

    for(int i=0;i<4;++i)
    {
        int y = yStart + i*rowH;
        int xPos = controlsStartX;
        
        oct[i].setBounds(xPos, y, kSize, kSize);
        xPos += kSize + gapX;
        
        semi[i].setBounds(xPos, y, kSize, kSize);
        xPos += kSize + gapX;
        
        mix[i].setBounds(xPos, y, kSize, kSize);
        xPos += kSize + gapX;
        
        // Update text box sizes for scaling
        oct[i].setTextBoxStyle(juce::Slider::TextBoxBelow, true, textBoxWidth, textBoxHeight);
        semi[i].setTextBoxStyle(juce::Slider::TextBoxBelow, true, textBoxWidth, textBoxHeight);
        mix[i].setTextBoxStyle(juce::Slider::TextBoxBelow, true, textBoxWidth, textBoxHeight);

        octL[i].setBounds (oct[i].getX(),  y-labelOffset, kSize, labelHeight);
        semiL[i].setBounds(semi[i].getX(), y-labelOffset, kSize, labelHeight);
        mixL[i].setBounds (mix[i].getX(),  y-labelOffset, kSize, labelHeight);
        
        // Position waveform preview to the right of the knobs
        int waveformY = y + (kSize - waveformHeight) / 2;
        waveformPreviews[i].setBounds(xPos, waveformY, waveformWidth, waveformHeight);
    }
}

void Distortion1AudioProcessorEditor::sliderValueChanged (juce::Slider* s) 
{
    if (s == &highlightKnob)
    {
        updateWaveformHighlights();
    }
}

void Distortion1AudioProcessorEditor::updateWaveformHighlights()
{
    int highlightedIndex = (int)highlightKnob.getValue();
    for(int i=0; i<4; ++i)
    {
        waveformPreviews[i].setHighlighted(i == highlightedIndex);
    }
}

