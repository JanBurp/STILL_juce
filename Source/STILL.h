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


        // Load MIDI file
        File* theFile = new File("/Users/jan/JUCE/Projects/STILL/Resources/midi/piano.mid");
        FileInputStream theStream( *theFile );
        theMIDIFile.readFrom(theStream);
        theMIDIFile.convertTimestampTicksToSeconds();
        Logger::outputDebugString("DEBUG - loaded MIDI file. Tracks: " +std::to_string(theMIDIFile.getNumTracks()) );

        // MIDI EVENTS
        const MidiMessageSequence *midiSequence = theMIDIFile.getTrack(0);
        int numEvents = midiSequence->getNumEvents();
        Logger::outputDebugString("DEBUG - MIDI file has events: " + std::to_string(numEvents) );

        for (auto i = 0; i < numEvents; i++)
        {
            for (const auto midiEvent : *midiSequence ) {
                const auto midiMessage = midiEvent->message;
                Logger::outputDebugString( midiMessage.getDescription() + " - " + std::to_string(i));
            }
        }

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

        auto cpu = deviceManager.getCpuUsage() * 100;
        cpuUsageText.setText (juce::String (cpu, 6) + " %", juce::dontSendNotification);
    }

    //==========================================================================
    juce::Label cpuUsageText;

    juce::MidiKeyboardState keyboardState;

    MidiFile theMIDIFile; // The current MIDI file content

    Sampler sampler;
    juce::MidiKeyboardComponent keyboardComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainContentComponent)
};
