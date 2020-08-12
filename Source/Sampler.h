#pragma once

#define MAX_VOICES 16
const double sampleRate = 44100; // We know the samplerate, because the samples are known?? //synth.getSampleRate();

const String samplesPath = "/Users/jan/JUCE/Projects/STILL/Resources/samples/";
const String midiPath = "/Users/jan/JUCE/Projects/STILL/Resources/midi/";

// Sampler
struct samplerPart {
    const String    name;
    const String    sampleFile;
    const int       baseNote;
    const int       noteRangeLow;
    const int       noteRangeHigh;
    const double    attack;
    const double    release;
    const double    length;
};

const int numOfSampleParts = 3;
const samplerPart samplerParts[numOfSampleParts] = {
    { "Bass", "BASS_2B.wav", 35, 0,42, 0,10,82 },
    { "Piano", "PIANO_4A.wav", 69, 43,78, 0,5,21 },
    { "Afterglow", "AFTERGLOW_6A.wav", 93, 79,128, 0,10,30 }
};

// MIDI
struct midiLoop {
    const String        name;
    const String        midiFile;
    int                 bufferStart;
    int                 bufferLength;
    juce::MidiBuffer    midiBuffer;
};

const int numOfMidiLoops = 3;
midiLoop midiLoops[numOfMidiLoops] = {
    { "Bass", "bass.mid" },
    { "Piano", "piano.mid" },
    { "Afterglow", "afterglow.mid" }
};


//==============================================================================
class Sampler   : public juce::AudioSource
{
public:
    Sampler (juce::MidiKeyboardState& keyState)
        : keyboardState (keyState)
    {
        // Add Voices
        for (auto i = 0; i < MAX_VOICES; ++i) {
            voices[i] = new SamplerVoice();
            synth.addVoice (voices[i]);
        }

        // Add SampleSounds
        Logger::outputDebugString( "Samplerate = " +std::to_string(sampleRate) );
        audioFormatManager.registerBasicFormats();
        for (int i = 0; i < numOfSampleParts; ++i)
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
                samplerParts[i].attack, samplerParts[i].release, samplerParts[i].length
            ));
            Logger::outputDebugString("DEBUG - Added sound '" + samplerParts[i].name + "' to sampler ["+samplerParts[i].sampleFile+"]");
        }

        // Add Midi files
        for (int i = 0; i < numOfMidiLoops; ++i)
        {
            midiLoops[i].midiBuffer = loadMidiFileToBuffer( midiPath + midiLoops[i].midiFile);
            midiLoops[i].bufferStart = 0;
            midiLoops[i].bufferLength = midiLoops[i].midiBuffer.getLastEventTime();
        }
    }

    juce::MidiBuffer loadMidiFileToBuffer(String file) {
        // Load file
        FileInputStream fileStream(file);
        MidiFile midiFile;
        midiFile.readFrom(fileStream);
        midiFile.convertTimestampTicksToSeconds();
        Logger::outputDebugString("DEBUG - loaded MIDI file '" + file);

        // Add events to buffer
        juce::MidiBuffer midiBuffer;
        midiBuffer.clear();
        for (int t = 0; t < midiFile.getNumTracks(); t++) {
            const MidiMessageSequence* track = midiFile.getTrack(t);
            for (int i = 0; i < track->getNumEvents(); i++) {
                MidiMessage& midiMessage = track->getEventPointer(i)->message;
                if ( midiMessage.isNoteOnOrOff() ) {
                    int sampleOffset = (int)(sampleRate * midiMessage.getTimeStamp());
                    midiBuffer.addEvent(midiMessage, sampleOffset);
                }
            }
        }
        return midiBuffer;
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
        juce::MidiBuffer playingMidi;

        // MIDI from keyboard
        keyboardState.processNextMidiBuffer (playingMidi, bufferToFill.startSample, bufferToFill.numSamples, true);

        // MIDI from file(s)
        midiCollector.removeNextBlockOfMessages(playingMidi, bufferToFill.numSamples);

        // add events from playing midi-files
        for (int i = 0; i < numOfMidiLoops; ++i) {
            playingMidi.addEvents(midiLoops[i].midiBuffer, samplesPlayed - midiLoops[i].bufferStart, bufferToFill.numSamples, 0);
            // Loop?
            if (samplesPlayed > ( midiLoops[i].bufferStart + midiLoops[i].bufferLength) ) {
                midiLoops[i].bufferStart += midiLoops[i].bufferLength + sampleRate; // add some extra (1sec) to make sure, first event is playing
                Logger::outputDebugString("["+String(samplesPlayed)+"] LOOP : " + midiLoops[i].name + "[" + String(midiLoops[i].bufferStart) + "," + String(midiLoops[i].bufferLength) + "]");
            }
        }

        // Log playing MIDI notes
        if ( !playingMidi.isEmpty() ) {
            int numEvents = playingMidi.getNumEvents();
            for (auto i = 0; i < numEvents; i++)
            {
                for (const auto midiEvent : playingMidi) {
                    const auto midiMessage = midiEvent.getMessage();
                    Logger::outputDebugString("["+String(samplesPlayed)+"] PLAY - " + midiMessage.getDescription() + " ["+ String(midiMessage.getTimeStamp()) +"]" );
                }
            }
            // pass these messages to the keyboard state so that it can update the component
            // to show on-screen which keys are being pressed on the physical midi keyboard.
            keyboardState.processNextMidiBuffer(playingMidi, 0, bufferToFill.numSamples, true);
        }

        // Keep samplePosition
        samplesPlayed += bufferToFill.numSamples;

        // Play
        synth.renderNextBlock (*bufferToFill.buffer, playingMidi, bufferToFill.startSample, bufferToFill.numSamples);
    }

    int getSamplePosition() {
        return samplesPlayed;
    }

    int getEllapsedTimeInSeconds() {
        return samplesPlayed / sampleRate;
    }

    int getNumberOfActiveVoices() {
        int activeVoices = 0;
        for (int i = 0; i < MAX_VOICES; ++i)
        {
            if (voices[i]->isVoiceActive()) {
                activeVoices++;
            }
        }
        return activeVoices;
    }


private:
    juce::MidiKeyboardState& keyboardState;
    juce::Synthesiser synth;
    juce::SynthesiserVoice* voices[MAX_VOICES];
    AudioFormatManager audioFormatManager;

    MidiMessageCollector midiCollector;
    int samplesPlayed = 0;
};
