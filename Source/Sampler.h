#pragma once

#define MAX_VOICES 16

const String samplesPath = "/Users/jan/JUCE/Projects/STILL/Resources/samples/";

struct samplerPart {
    String       name;
    String       sampleFile;
    int          baseNote;
    int          noteRangeLow;
    int          noteRangeHigh;
} book;

const samplerPart samplerParts[3] = {
    { "Bass", "BASS_2B.wav", 35, 0,42 },
    { "Piano", "PIANO_4A.wav", 69, 43,78 },
    { "Afterglow", "AFTERGLOW_6A.wav",93, 79,128 }
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
            File* sampleFile = new File(samplesPath + samplerParts[i].sampleFile);
            std::unique_ptr<AudioFormatReader> reader (audioFormatManager.createReaderFor(*sampleFile));
            BigInteger noteRange;
            noteRange.setRange(samplerParts[i].noteRangeLow, samplerParts[i].noteRangeHigh, true);
            synth.addSound(new SamplerSound(
                samplerParts[i].name,
                *reader,
                noteRange,
                samplerParts[i].baseNote,
                0, 0, 20
            ));
            Logger::outputDebugString("DEBUG - Added sound '" + samplerParts[i].name + "' to sampler ["+samplerParts[i].sampleFile+"]");
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
        keyboardState.processNextMidiBuffer (incomingMidi, bufferToFill.startSample, bufferToFill.numSamples, true);

        if ( !incomingMidi.isEmpty() ) {
            int numEvents = incomingMidi.getNumEvents(); // SLOW
            for (auto i = 0; i < numEvents; i++)
            {
                for (const auto midiEvent : incomingMidi) {
                    const auto midiMessage = midiEvent.getMessage();
                    Logger::outputDebugString("DEBUG - " + midiMessage.getDescription() );
                }
            }

        }
        synth.renderNextBlock (*bufferToFill.buffer, incomingMidi, bufferToFill.startSample, bufferToFill.numSamples);
    }

private:
    juce::MidiKeyboardState& keyboardState;
    juce::Synthesiser synth;
    AudioFormatManager audioFormatManager;
};
