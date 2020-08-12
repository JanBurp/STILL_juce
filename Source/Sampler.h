#pragma once

#include "constants.h"
#include "MidiLoop.h"

MidiLoop midiLoops[constants::numOfMidiLoops] = {
    { "Bass", "bass.mid" },
    { "Piano", "piano.mid" },
    { "Afterglow", "afterglow.mid" }
};

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


//==============================================================================
class Sampler   : public juce::AudioSource
{
public:
    Sampler (juce::MidiKeyboardState& keyState)
        : keyboardState (keyState)
    {
        // Add Voices
        for (auto i = 0; i < constants::MAX_VOICES; ++i) {
            voices[i] = new SamplerVoice();
            synth.addVoice (voices[i]);
        }

        // Add SampleSounds
        Logger::outputDebugString( "Samplerate = " +std::to_string(constants::sampleRate) );
        audioFormatManager.registerBasicFormats();
        for (int i = 0; i < numOfSampleParts; ++i)
        {
            File* sampleFile = new File(constants::samplesPath + samplerParts[i].sampleFile);
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
        for (int i = 0; i < constants::numOfMidiLoops; ++i) {
            midiLoops[i].addEventsFromLoop( &playingMidi, samplesPlayed, bufferToFill.numSamples );
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
        return samplesPlayed / constants::sampleRate;
    }

    int getNumberOfActiveVoices() {
        int activeVoices = 0;
        for (int i = 0; i < constants::MAX_VOICES; ++i)
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
    juce::SynthesiserVoice* voices[constants::MAX_VOICES];
    AudioFormatManager audioFormatManager;

    int samplesPlayed = 0;
    MidiMessageCollector midiCollector;
};
