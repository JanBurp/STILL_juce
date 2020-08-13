#include <JuceHeader.h>

#ifndef CONSTANTS_H
#define CONSTANTS_H

namespace constants
{
    inline const int MAX_VOICES = 16;
    inline const double sampleRate = 44100; // We know the samplerate, because the samples are known?? //synth.getSampleRate();
    inline const String samplesPath = "/Users/jan/JUCE/Projects/STILL/Resources/samples/";
    inline const String midiPath = "/Users/jan/JUCE/Projects/STILL/Resources/midi/";

    inline const int numOfMidiLoops = 3;
    inline const int numOfSampleParts = 3;
}
#endif
