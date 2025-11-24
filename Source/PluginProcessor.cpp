#include "PluginProcessor.h"
#include "PluginEditor.h"

Distortion1AudioProcessor::Distortion1AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor (BusesProperties()
                     .withInput  ("Input", juce::AudioChannelSet::stereo(), true)
                     .withOutput ("Output", juce::AudioChannelSet::stereo(), true)),
      parameters (*this, nullptr, "PARAMETERS", createParameterLayout())
#endif
{
}

Distortion1AudioProcessor::~Distortion1AudioProcessor() {}

juce::AudioProcessorValueTreeState::ParameterLayout Distortion1AudioProcessor::createParameterLayout()
{
    std::vector<std::unique_ptr<juce::RangedAudioParameter>> p;
    const juce::String oscIDs[4] = {"SQUARE","SAW","TRIANGLE","SINE"};

    for (int i=0;i<4;++i)
    {
        p.push_back(std::make_unique<juce::AudioParameterInt>(oscIDs[i]+"_OCT","Octave",-2,2,0));
        p.push_back(std::make_unique<juce::AudioParameterFloat>(oscIDs[i]+"_SEMI","Semitone",-12.f,12.f,0.f));
        p.push_back(std::make_unique<juce::AudioParameterFloat>(oscIDs[i]+"_MIX","Mix",0.f,1.f,0.5f));
    }
    
    // Highlight parameter for OSC routing (0-3: Square, Saw, Triangle, Sine)
    p.push_back(std::make_unique<juce::AudioParameterInt>("HIGHLIGHT_OSC","Highlight Oscillator",0,3,0));
    
    return { p.begin(), p.end() };
}

void Distortion1AudioProcessor::prepareToPlay (double sr, int) { sampleRateHz = sr; }

void Distortion1AudioProcessor::releaseResources() {}

bool Distortion1AudioProcessor::isBusesLayoutSupported (const BusesLayout& l) const
{
    return l.getMainOutputChannelSet() == juce::AudioChannelSet::stereo();
}

void Distortion1AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
{
    juce::ScopedNoDenormals _;
    buffer.clear();

    // MIDI handling - polyphonic
    for (const auto metadata : midi)
    {
        const auto msg = metadata.getMessage();
        const int noteNumber = msg.getNoteNumber();
        
        if (msg.isNoteOn())
        {
            NoteData note;
            note.freq = juce::MidiMessage::getMidiNoteInHertz(noteNumber);
            for(int i=0; i<4; ++i) note.phase[i] = 0.0;
            note.envelope = 1.0f;
            note.releasing = false;
            activeNotes[noteNumber] = note;
        }
        else if (msg.isNoteOff())
        {
            auto it = activeNotes.find(noteNumber);
            if (it != activeNotes.end())
            {
                it->second.releasing = true; // Start release instead of removing
            }
        }
    }

    // update params
    const juce::String oscIDs[4]={"SQUARE","SAW","TRIANGLE","SINE"};
    for(int i=0;i<4;++i){
        mOct[i]  = (float)*parameters.getRawParameterValue(oscIDs[i]+"_OCT");
        mSemi[i] = *parameters.getRawParameterValue(oscIDs[i]+"_SEMI");
        mMix[i]  = *parameters.getRawParameterValue(oscIDs[i]+"_MIX");
    }

    auto* left = buffer.getWritePointer(0);
    auto* right= buffer.getNumChannels()>1?buffer.getWritePointer(1):nullptr;

    if (activeNotes.empty()) return;

    const int n = buffer.getNumSamples();
    for(int s=0;s<n;++s)
    {
        float out=0.f;
        
        // Process each active note
        for(auto it = activeNotes.begin(); it != activeNotes.end();)
        {
            auto& [noteNum, note] = *it;
            
            // Update envelope (release decay)
            if (note.releasing)
            {
                note.envelope *= 0.95f; // Fast decay (adjust for release time)
                if (note.envelope < 0.001f) // Remove when silent
                {
                    it = activeNotes.erase(it);
                    continue;
                }
            }
            
            float noteOut = 0.f;
            for(int o=0;o<4;++o)
            {
                float freq = note.freq * std::pow(2.f,mOct[o]) * std::pow(2.f,mSemi[o]/12.f);
                double phaseInc = juce::MathConstants<double>::twoPi * freq / sampleRateHz;

                float val=0.f;
                double ph=note.phase[o];
                switch(o){
                    case 0: val = std::sin(ph)>0?1.f:-1.f; break;
                    case 1: val = (float)(2.0*(ph/juce::MathConstants<double>::twoPi)-1.0); break;
                    case 2: val = (float)(2.0/juce::MathConstants<double>::pi)*std::asin(std::sin(ph)); break;
                    case 3: val = std::sin(ph); break;
                }
                noteOut += val * mMix[o];
                note.phase[o] += phaseInc;
                if(note.phase[o]>=juce::MathConstants<double>::twoPi) note.phase[o]-=juce::MathConstants<double>::twoPi;
            }
            out += noteOut * 0.25f * note.envelope; // Apply envelope
            
            ++it;
        }

        left[s]=out;
        if(right) right[s]=out;
        latestSample.store(out);
    }
}

void Distortion1AudioProcessor::getStateInformation (juce::MemoryBlock& dest)
{
    copyXmlToBinary (*parameters.copyState().createXml(), dest);
}
void Distortion1AudioProcessor::setStateInformation (const void* d, int sz)
{
    if(auto xml=getXmlFromBinary(d,sz))
        if(xml->hasTagName(parameters.state.getType()))
            parameters.replaceState(juce::ValueTree::fromXml(*xml));
}

bool Distortion1AudioProcessor::hasEditor() const { return true; }
juce::AudioProcessorEditor* Distortion1AudioProcessor::createEditor() { return new Distortion1AudioProcessorEditor(*this); }

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter() { return new Distortion1AudioProcessor(); }

