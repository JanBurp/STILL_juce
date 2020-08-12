#pragma once

#include <JuceHeader.h>

class SamplerPart {
public:
    const String    name;
    const String    sampleFile;
    const int       baseNote;
    const int       noteRangeLow;
    const int       noteRangeHigh;
    const double    attack;
    const double    release;
    const double    length;

    void loadSampleFile( AudioFormatManager *audioFormatManager, Synthesiser *synth );
};
