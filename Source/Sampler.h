#pragma once

#define MAX_VOICES 16

const String samplesPath = "/Users/jan/JUCE/Projects/STILL/Resources/samples/";
const String midiPath = "/Users/jan/JUCE/Projects/STILL/Resources/midi/";

struct samplerPart {
    String              name;
    String              sampleFile;
    String              midiFile;
    int                 baseNote;
    int                 noteRangeLow;
    int                 noteRangeHigh;
    MidiMessageSequence midiSequence;
} book;

samplerPart samplerParts[3] = {
    { "Bass", "BASS_2B.wav", "", 35, 0,42 },
    { "Piano", "PIANO_4A.wav", "piano.mid", 69, 43,78 },
    { "Afterglow", "AFTERGLOW_6A.wav", "", 93, 79,128 }
};

MidiMessageCollector midiCollector;
juce::MidiBuffer midiBuffer;
int samplesPlayed;
bool midiIsPlaying = false;

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

        // Add samplefiles
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

        // Load Midi files
        for (int i = 0; i < 3; ++i)
        {
            if ( samplerParts[i].midiFile !="" ) {
                loadMidiFileToBuffer(midiPath + samplerParts[i].midiFile);
            }
        }
    }

    void loadMidiFileToBuffer(String file) {
        // Load file
        FileInputStream fileStream(file);
        MidiFile M;
        M.readFrom(fileStream);
        M.convertTimestampTicksToSeconds();
        Logger::outputDebugString("DEBUG - loaded MIDI file '" + file);

        // Add events to buffer
        midiBuffer.clear();
        double sampleRate = 44100; // We know the samplerate, because the samples are known?? //synth.getSampleRate();
        Logger::outputDebugString( "Samplerate = " +std::to_string(sampleRate) );
        for (int t = 0; t < M.getNumTracks(); t++) {
            const MidiMessageSequence* track = M.getTrack(t);
            for (int i = 0; i < track->getNumEvents(); i++) {
                MidiMessage& m = track->getEventPointer(i)->message;
                int sampleOffset = (int)(sampleRate * m.getTimeStamp());
                midiBuffer.addEvent(m, sampleOffset);
//                Logger::outputDebugString( std::to_string(sampleOffset) + " : " + m.getDescription() + " - " + std::to_string(i) );
            }
        }
        samplesPlayed = 0;
        midiIsPlaying = true;
    }

    void setUsingSynthSound()
    {
        synth.clearSounds();
    }

    void prepareToPlay (int /*samplesPerBlockExpected*/, double sampleRate) override
    {
        midiCollector.reset (sampleRate);
        synth.setCurrentPlaybackSampleRate (sampleRate); // [3]
    }

    void releaseResources() override {}

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        bufferToFill.clearActiveBufferRegion();

        juce::MidiBuffer incomingMidi;

        // MIDI from keyboard
        keyboardState.processNextMidiBuffer (incomingMidi, bufferToFill.startSample, bufferToFill.numSamples, true);

        // MIDI from file(s)
        midiCollector.removeNextBlockOfMessages(incomingMidi, bufferToFill.numSamples);
        // add events from playing midi-file
        if (midiIsPlaying) {
            int sampleDeltaToAdd = -samplesPlayed;
            incomingMidi.addEvents(midiBuffer, samplesPlayed, bufferToFill.numSamples, sampleDeltaToAdd);
            samplesPlayed += bufferToFill.numSamples;
            // pass these messages to the keyboard state so that it can update the component
            // to show on-screen which keys are being pressed on the physical midi keyboard.
            keyboardState.processNextMidiBuffer(incomingMidi, 0, bufferToFill.numSamples, true);
        }

        // Log playing MIDI notes
        if ( !incomingMidi.isEmpty() ) {
            int numEvents = incomingMidi.getNumEvents();
            for (auto i = 0; i < numEvents; i++)
            {
                for (const auto midiEvent : incomingMidi) {
                    const auto midiMessage = midiEvent.getMessage();
                    Logger::outputDebugString("PLAY - " + midiMessage.getDescription() );
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
