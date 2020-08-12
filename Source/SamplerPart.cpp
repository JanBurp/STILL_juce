#include "constants.h"
#include "SamplerPart.h"

void SamplerPart::loadSampleFile( AudioFormatManager *audioFormatManager, Synthesiser *synth ) {
    File* file = new File(constants::samplesPath + sampleFile);
    std::unique_ptr<AudioFormatReader> reader (audioFormatManager->createReaderFor(*file));
    BigInteger noteRange;
    noteRange.setRange(noteRangeLow, noteRangeHigh, true);
    synth->addSound(new SamplerSound(
        name,
        *reader,
        noteRange,
        baseNote,
        attack, release, length
    ));
    Logger::outputDebugString("DEBUG - Added sound '" + name + "' to sampler ["+sampleFile+"]");
}
