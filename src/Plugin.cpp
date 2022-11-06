#include <JuceHeader.h>
#include "Plugin.h"

AudioPlugin::AudioPlugin()
    : AudioProcessor(BusesProperties().withInput("Input", AudioChannelSet::stereo()).withOutput("Output", AudioChannelSet::stereo()))
{
    addParameter(line_count = new AudioParameterInt({"lines", 1}, "Lines", 1, LINE_COUNT_MAX, 5));
    addParameter(delay_time = new AudioParameterFloat({"delay_time", 1}, "Delay Time", 0.0f, 1.0f, 0.5f));
    addParameter(attenuation = new AudioParameterFloat({"attenuation", 1}, "Attenuation", 0.0f, 1.0f, 0.6f));
    addParameter(dry = new AudioParameterFloat({"dry", 1}, "Dry", 0.0f, 1.0f, 1.0f));
    addParameter(wet = new AudioParameterFloat({"wet", 1}, "Wet", 0.0f, 1.0f, 1.0f));
}

void AudioPlugin::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock)
{
    lines = new float*[LINE_COUNT_MAX];

    for (size_t line = 0; line < LINE_COUNT_MAX; line++)
    {
        lines[line] = new float[(size_t)(sampleRate)];
        for (size_t sample = 0; sample < (size_t)(sampleRate); sample++)
        {
            lines[line][sample] = 0.0f;
        }
    }
}

void AudioPlugin::releaseResources()
{
    for (size_t line = 0; line < (size_t)line_count->get(); line++)
    {
        delete[] lines[line];
    }
    delete[] lines;
}

template <typename T>
void AudioPlugin::processSamples(AudioBuffer<T> &audioBuffer, MidiBuffer &midiBuffer)
{
    auto reader = audioBuffer.getArrayOfReadPointers();
    auto writer = audioBuffer.getArrayOfWritePointers();

    size_t delay_time_in_samples = (size_t)(delay_time->get() * getSampleRate());

    for (size_t sample = 0; sample < (size_t)audioBuffer.getNumSamples(); sample++)
    {
        float sample_dry = 0.0f;

        for (size_t channel = 0; channel < (size_t)audioBuffer.getNumChannels(); channel++)
        {
            sample_dry += (float)reader[channel][sample];
        }

        float sample_wet = 0.0f;
        for (size_t line = (size_t)(line_count->get() - 1); line > 0; line--)
        {
            sample_wet += lines[line][line_head];
            lines[line][line_head] = lines[line - 1][line_head] * (1.0f - attenuation->get());
        }
        sample_wet += lines[0][line_head];
        lines[0][line_head] = sample_dry * (1.0f - attenuation->get());

        float mixed = (sample_dry * dry->get()) + (sample_wet * wet->get());

        for (size_t channel = 0; channel < (size_t)audioBuffer.getNumChannels(); channel++)
        {
            writer[channel][sample] = mixed;
        }
        
        line_head = (line_head + 1) % delay_time_in_samples;
    }
}

void AudioPlugin::processBlock(AudioBuffer<float> &audioBuffer, MidiBuffer &midiBuffer)
{
    processSamples<float>(audioBuffer, midiBuffer);
}

void AudioPlugin::processBlock(AudioBuffer<double> &audioBuffer, MidiBuffer &midiBuffer)
{
    processSamples<double>(audioBuffer, midiBuffer);
}

void AudioPlugin::getStateInformation(MemoryBlock &destData)
{
    MemoryOutputStream *memoryOutputStream = new MemoryOutputStream(destData, true);
    memoryOutputStream->writeInt(*line_count);
    memoryOutputStream->writeFloat(*delay_time);
    memoryOutputStream->writeFloat(*attenuation);
    memoryOutputStream->writeFloat(*dry);
    memoryOutputStream->writeFloat(*wet);
    delete memoryOutputStream;
}

void AudioPlugin::setStateInformation(const void *data, int sizeInBytes)
{
    MemoryInputStream *memoryInputStream = new MemoryInputStream(data, static_cast<size_t>(sizeInBytes), false);
    line_count->setValueNotifyingHost(memoryInputStream->readInt());
    delay_time->setValueNotifyingHost(memoryInputStream->readFloat());
    attenuation->setValueNotifyingHost(memoryInputStream->readFloat());
    dry->setValueNotifyingHost(memoryInputStream->readFloat());
    wet->setValueNotifyingHost(memoryInputStream->readFloat());
    delete memoryInputStream;
}

juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter()
{
    return new AudioPlugin();
}