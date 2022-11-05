#pragma once


//==============================================================================
class AudioPlugin  : public AudioProcessor
{
public:

    //==============================================================================
    AudioPlugin();

    //==============================================================================
    void prepareToPlay (double, int) override;
    void releaseResources() override;

    template<typename T>
    void processSamples(AudioBuffer<T>& audioBuffer, MidiBuffer& midiBuffer);
    void processBlock (AudioBuffer<float>& audioBuffer, MidiBuffer& midiBuffer) override;
    void processBlock (AudioBuffer<double>& audioBuffer, MidiBuffer& midiBuffer) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override          { return new GenericAudioProcessorEditor (*this); }
    bool hasEditor() const override                        { return true;   }

    //==============================================================================
    const String getName() const override                  { return "Audio PlugIn"; }
    bool acceptsMidi() const override                      { return false; }
    bool producesMidi() const override                     { return false; }
    double getTailLengthSeconds() const override           { return 0; }

    //==============================================================================
    int getNumPrograms() override                          { return 1; }
    int getCurrentProgram() override                       { return 0; }
    void setCurrentProgram (int) override                  {}
    const String getProgramName (int) override             { return "None"; }
    void changeProgramName (int, const String&) override   {}

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;

    void setStateInformation (const void* data, int sizeInBytes) override;

    //==============================================================================
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override
    {
        const auto& mainInLayout  = layouts.getChannelSet (true,  0);
        const auto& mainOutLayout = layouts.getChannelSet (false, 0);

        return (mainInLayout == mainOutLayout && (! mainInLayout.isDisabled()));
    }

private:
    //==============================================================================
    AudioParameterFloat* gain;

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (AudioPlugin)
};