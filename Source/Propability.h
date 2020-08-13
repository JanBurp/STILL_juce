#pragma once

#include <JuceHeader.h>

class Propability : private juce::Timer {
public:
    void    startProbability();
    int     getPropability();
    String  getPropabilityString();
    bool    getChance();
private:
    void timerCallback() override;
    int     const prop = 100;
    int     const propTimer = 90;
    int     currentProp;
    int     currentTimer;
};

inline Propability propability;
