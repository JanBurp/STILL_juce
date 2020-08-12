#include "constants.h"
#include "MidiLoop.h"

MidiLoop::MidiLoop(String name, String midiFile) {
     midiBuffer = loadMidiFileToBuffer( constants::midiPath + midiFile);
     bufferStart = 0;
     bufferLength = midiBuffer.getLastEventTime();
}

juce::MidiBuffer MidiLoop::loadMidiFileToBuffer(String file) {
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
                int sampleOffset = (int)(constants::sampleRate * midiMessage.getTimeStamp());
                midiBuffer.addEvent(midiMessage, sampleOffset);
            }
        }
    }
    return midiBuffer;
}

void MidiLoop::addEventsFromLoop( juce::MidiBuffer *playingMidi, int samplesPlayed, int numSamples ) {
    playingMidi->addEvents(midiBuffer, samplesPlayed - bufferStart, numSamples, 0);
    if ( isLoopEnded(samplesPlayed) ) {
        resetLoop();
    }
}

bool MidiLoop::isLoopEnded(int samplesPlayed) {
    return samplesPlayed > ( bufferStart + bufferLength);
}

void MidiLoop::resetLoop() {
    bufferStart += bufferLength + constants::sampleRate; // add some extra (1sec) to make sure, first event is playing
    Logger::outputDebugString("LOOP : " + name + "[" + String(bufferStart) + "," + String(bufferLength) + "]");
}

