#pragma once
#include <JuceHeader.h>
#include <array>
#include <map>

//==============================================================================

class Distortion1AudioProcessor : public juce::AudioProcessor
{
public:
    Distortion1AudioProcessor();
    ~Distortion1AudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout&) const override;
   #endif
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;
    const juce::String getName() const override { return JucePlugin_Name; }

    bool acceptsMidi() const override   { return true; }
    bool producesMidi() const override  { return false; }
    bool isMidiEffect() const override  { return false; }
    double getTailLengthSeconds() const override { return 0.0; }

    //==============================================================================
    int getNumPrograms() override { return 1; }
    int getCurrentProgram() override { return 0; }
    void setCurrentProgram (int) override {}
    const juce::String getProgramName (int) override { return {}; }
    void changeProgramName (int, const juce::String&) override {}

    //==============================================================================
    void getStateInformation (juce::MemoryBlock&) override;
    void setStateInformation (const void*, int) override;

    juce::AudioProcessorValueTreeState parameters;

    // For oscilloscope rendering
    std::atomic<float> latestSample { 0.0f };

private:
    juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout();

    double sampleRateHz = 44100.0;

    float mOct[4] {0}, mSemi[4] {0}, mMix[4] {0};
    
    // Polyphonic note tracking
    struct NoteData {
        float freq;
        double phase[4] {0.0};
        float envelope = 1.0f;  // Envelope (1.0 = full volume, 0.0 = silent)
        bool releasing = false; // Track if note is in release phase
    };
    std::map<int, NoteData> activeNotes;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (Distortion1AudioProcessor)
};

