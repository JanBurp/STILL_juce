#pragma once


#define MAX_VOICES 16

const String samplesPath = "/Users/jan/JUCE/Projects/STILL/Samples/wav/";

struct samplerNote {
    String       name;
    File*        sampleFile;
    int          baseNote;
    int          noteRangeLow;
    int          noteRangeHigh;
} book;

const samplerNote allSamplerNotes[3] = {
    { "Bass", new File(samplesPath + "BASS_2B.wav"), 35, 0,42 },
    { "Piano", new File(samplesPath + "PIANO_4A.wav"), 69, 43,78 },
    { "Afterglow", new File(samplesPath + "AFTERGLOW_6A.wav"),93, 79,128 }
};


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
        for (int i = 0; i < 3; ++i)
        {
            std::unique_ptr<AudioFormatReader> reader (audioFormatManager.createReaderFor(*allSamplerNotes[i].sampleFile));
            BigInteger noteRange;
            noteRange.setRange(allSamplerNotes[i].noteRangeLow, allSamplerNotes[i].noteRangeHigh, true);
            synth.addSound(new SamplerSound(
                allSamplerNotes[i].name,
                *reader,
                noteRange,
                allSamplerNotes[i].baseNote,
                0, 0, 20
            ));
        }

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
