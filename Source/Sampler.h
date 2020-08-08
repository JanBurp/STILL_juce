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

        // Add files

        // Bass sounds
        File* bassFile = new File("/Users/jan/JUCE/Projects/STILL/Samples/wav/BASS_2B.wav");
        std::unique_ptr<AudioFormatReader> bassReader (audioFormatManager.createReaderFor(*bassFile));

        BigInteger bassNotes;
        bassNotes.setRange(0, 42, true);
        synth.addSound(new SamplerSound("Bass", *bassReader, bassNotes, 35, 0, 0, 20));

        // Piano sounds
        File* pianoFile = new File("/Users/jan/JUCE/Projects/STILL/Samples/wav/PIANO_4A.wav");
        std::unique_ptr<AudioFormatReader> pianoReader (audioFormatManager.createReaderFor(*pianoFile));

        BigInteger pianoNotes;
        pianoNotes.setRange(43, 78, true);
        synth.addSound(new SamplerSound("Piano", *pianoReader, pianoNotes, 69, 0, 0, 20));

        // Afterglow sounds
        File* afterglowFile = new File("/Users/jan/JUCE/Projects/STILL/Samples/wav/AFTERGLOW_6A.wav");
        std::unique_ptr<AudioFormatReader> afterglowReader (audioFormatManager.createReaderFor(*afterglowFile));

        BigInteger afterglowNotes;
        afterglowNotes.setRange(79, 128, true);
        synth.addSound(new SamplerSound("Afterglow", *afterglowReader, afterglowNotes, 93, 0, 0, 20));


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
