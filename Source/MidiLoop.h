#pragma once

#include <JuceHeader.h>

class MidiLoop {
public:
    const String        name;
    const String        midiFile;
    int                 bufferStart;
    int                 bufferLength;
    juce::MidiBuffer    midiBuffer;

    MidiLoop(String name, String midiFile);
    juce::MidiBuffer loadMidiFileToBuffer(String file);
    void addEventsFromLoop( juce::MidiBuffer *playingMidi, int samplesPlayed, int numSamples );
    bool isLoopEnded(int samplesPlayed);
    void resetLoop();
};
