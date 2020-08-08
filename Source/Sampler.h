#pragma once

#define MAX_VOICES 4

//==============================================================================
class Sampler   : public juce::AudioSource
{
public:
    Sampler (juce::MidiKeyboardState& keyState)
        : keyboardState (keyState)
    {
        for (auto i = 0; i < MAX_VOICES; ++i)                // [1]
            synth.addVoice (new SamplerVoice());

        // set up our AudioFormatManager class as detailed in the API docs
        // we can now use WAV and AIFF files!
        audioFormatManager.registerBasicFormats();

        File* file = new File("/Users/jan/JUCE/Projects/STILL/Samples/PIANO_C#2.wav");
        std::unique_ptr<AudioFormatReader> reader (audioFormatManager.createReaderFor(*file));

        // allow our sound to be played on all notes
        BigInteger allNotes;
        allNotes.setRange(0, 128, true);

        // finally, add our sound
        synth.addSound(new SamplerSound("default", *reader, allNotes, 60, 0, 10, 10.0));

    }

    void setUsingSynthSound()
    {
        synth.clearSounds();
    }

    void prepareToPlay (int /*samplesPerBlockExpected*/, double sampleRate) override
    {
        synth.setCurrentPlaybackSampleRate (sampleRate); // [3]
    }

    void releaseResources() override {}

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        bufferToFill.clearActiveBufferRegion();

        juce::MidiBuffer incomingMidi;
        keyboardState.processNextMidiBuffer (incomingMidi, bufferToFill.startSample,
                                             bufferToFill.numSamples, true);       // [4]

        synth.renderNextBlock (*bufferToFill.buffer, incomingMidi,
                               bufferToFill.startSample, bufferToFill.numSamples); // [5]
    }

private:
    juce::MidiKeyboardState& keyboardState;
    juce::Synthesiser synth;
    AudioFormatManager audioFormatManager;
};
