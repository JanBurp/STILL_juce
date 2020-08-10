#pragma once
#include "Sampler.h"

//==============================================================================
class MainContentComponent   : public juce::AudioAppComponent,
                               private juce::Timer
{
public:
    MainContentComponent()
        : sampler  (keyboardState),
          keyboardComponent (keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
    {
        addAndMakeVisible (keyboardComponent);
        setAudioChannels (0, 2);

        cpuUsageText.setText ("CPU Usage", juce::dontSendNotification);
        addAndMakeVisible (cpuUsageText);

        setSize (600, 200);
        startTimer (400);
    }

    ~MainContentComponent() override
    {
        shutdownAudio();
    }

    void resized() override
    {
        keyboardComponent.setBounds (0, 40, getWidth(), getHeight() - 40);
        cpuUsageText.setBounds (10, 10, getWidth() - 20, 20);
    }

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override
    {
        sampler.prepareToPlay (samplesPerBlockExpected, sampleRate);
    }

    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override
    {
        sampler.getNextAudioBlock (bufferToFill);
    }

    void releaseResources() override
    {
        sampler.releaseResources();
    }

private:
    void timerCallback() override
    {
        keyboardComponent.grabKeyboardFocus();
        stopTimer();

        auto cpu = deviceManager.getCpuUsage() * 100;
        cpuUsageText.setText (juce::String (cpu, 6) + " %", juce::dontSendNotification);
    }

    //==========================================================================
    juce::Label cpuUsageText;

    juce::MidiKeyboardState keyboardState;
    Sampler sampler;
    juce::MidiKeyboardComponent keyboardComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};
