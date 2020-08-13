#include "constants.h"
#include "Propability.h"


void Propability::startProbability() {
    startTimer (1000);
    currentTimer = propTimer;
    currentProp = prop;
}

void Propability::timerCallback() {
    currentTimer--;
    currentProp = prop * currentTimer/propTimer;
}

int Propability::getPropability() {
    return currentProp;
}

String Propability::getPropabilityString() {
    return String(currentProp);
}

bool Propability::getChance() {
    int random = (int) (rand()%100+1);
    bool chance = (bool) (currentProp < random);
    String result = (chance)?"REMOVED":"ADDED";
    Logger::outputDebugString("Propability ["+String(currentProp)+"% >= "+String(random)+"% ] - " + result );
    return chance;
}

