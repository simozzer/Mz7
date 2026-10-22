// Mz7 - Created by Simon Moscrop
// Free software under the GNU AGPL v3; see LICENSE.

#pragma once

#include <juce_audio_utils/juce_audio_utils.h>

#include "DX7/Engine.h"
#include "Params.h"

#include <atomic>
#include <mutex>

//
// The plugin: one DX7, a cartridge of 32 voices to pick from, and the voice being played.
//
// THE VOICE is the 145 voice parameters plus its name. Turning a knob edits the voice, as on
// the DX7; nothing changes in the cartridge until STORE writes the voice into a slot - so a
// program change discards an unstored edit, again as on the DX7.
//
// THE CARTRIDGE is 32 voices: one of the four built in, or a .syx loaded from disk or sent
// over MIDI. The host's program list is its 32 slots.
//
class Mz7Processor final : public juce::AudioProcessor, private juce::AsyncUpdater
{
public:
    Mz7Processor();
    ~Mz7Processor() override;

    void prepareToPlay(double sampleRate, int samplesPerBlock) override;
    void releaseResources() override {}
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

    void processBlock(juce::AudioBuffer<float>&, juce::MidiBuffer&) override;
    using AudioProcessor::processBlock;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override { return true; }

    const juce::String getName() const override { return JucePlugin_Name; }
    bool acceptsMidi() const override { return true; }
    bool producesMidi() const override { return false; }
    bool isMidiEffect() const override { return false; }
    double getTailLengthSeconds() const override { return 8.0; }

    int getNumPrograms() override { return 32; }
    int getCurrentProgram() override { return program; }
    void setCurrentProgram(int index) override;
    const juce::String getProgramName(int index) override;
    void changeProgramName(int, const juce::String&) override {}

    void getStateInformation(juce::MemoryBlock& destData) override;
    void setStateInformation(const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState state;
    juce::MidiKeyboardState keyboardState;

    // ---- the cartridge, and the voice: message thread only
    const dx7::Cartridge& cartridge() const { return cart; }
    juce::String cartridgeName() const      { return cartName; }
    int builtInBank() const                 { return bank; }       // -1 if loaded from a file

    void selectBuiltIn(int bankIndex);
    bool loadSyx(const juce::File& file, juce::String& message);
    bool saveCartridge(const juce::File& file) const;
    bool saveVoice(const juce::File& file) const;

    juce::String voiceName() const;
    void setVoiceName(const juce::String& name);
    void store(int slot);                   // the voice being played into slot 0-31
    void initVoice();
    dx7::Patch currentVoice() const;        // the voice as the knobs have it, name and all

    int changeCount() const { return changes.load(); }   // bumped on anything the panel shows

private:
    void handle(const juce::MidiMessage& m);
    void handleAsyncUpdate() override;
    void adoptCartridge(const dx7::Cartridge& c, const juce::String& title, int builtIn);

    params::Reader reader;
    dx7::Engine engine;
    dx7::Patch playing;                     // what the engine has

    dx7::Cartridge cart;
    juce::String cartName;
    int bank = 0, program = 0;

    std::mutex nameLock;
    char name[10] {};

    // from the audio thread to the message thread: a program change, a sysex dump
    std::atomic<int> pendingProgram { -1 };
    std::mutex sysexLock;
    std::vector<uint8_t> pendingSysex;

    std::atomic<int> changes { 0 };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Mz7Processor)
};
