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
        samplePosition.setText ("SamplePos", juce::dontSendNotification);
        addAndMakeVisible (samplePosition);
        ellapsedTime.setText ("ellapsedTime", juce::dontSendNotification);
        addAndMakeVisible (ellapsedTime);


        setSize (600, 200);
        startTimer (400);
    }

    ~MainContentComponent() override
    {
        shutdownAudio();
    }

    void resized() override
    {
        keyboardComponent.setLowestVisibleKey(42);
        keyboardComponent.setBounds (0, 40, getWidth(), getHeight() - 40);

        cpuUsageText.setBounds (10, 10, getWidth()/2 -20, 20);
        samplePosition.setBounds (getWidth()/2, 10, getWidth()/4 -20, 20);
        ellapsedTime.setBounds (getWidth()/4*3, 10, getWidth()/4 -20, 20);
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

        auto cpu = deviceManager.getCpuUsage() * 100;
        cpuUsageText.setText (juce::String(cpu, 2) + " %", juce::dontSendNotification);

        samplePosition.setText( juce::String(sampler.getSamplePosition()) , juce::dontSendNotification);
        ellapsedTime.setText( juce::String(sampler.getEllapsedTimeInSeconds()) , juce::dontSendNotification);
    }

    //==========================================================================
    juce::Label cpuUsageText;
    juce::Label samplePosition;
    juce::Label ellapsedTime;

    juce::MidiKeyboardState keyboardState;

    Sampler sampler;
    juce::MidiKeyboardComponent keyboardComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};
