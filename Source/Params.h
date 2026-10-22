// Mz7 - Created by Simon Moscrop
// Free software under the GNU AGPL v3; see LICENSE.

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>

#include "DX7/Engine.h"
#include "DX7/Patch.h"

#include <vector>

namespace params
{

//
// Every parameter: the 145 numbers of a DX7 voice (all of it but the name), the FUNCTION
// settings the DX7 keeps for itself, and the few things a plugin adds - output volume and
// the operator on/off switches.
//
// A voice parameter is one byte of the VCED dump, so `byte` says which; the reader builds a
// dx7::Patch straight from them. IDs are saved in every host project. Never rename one.
//
struct Spec
{
    juce::String id, name;
    int byte = -1;                 // the VCED byte, or -1 for a function setting
    int min = 0, max = 99, def = 0;
};

const std::vector<Spec>& voice();        // the 145 voice parameters, in VCED order
const std::vector<Spec>& function();     // FUNCTION and plugin settings

// "op3R1" and the like: operator n's parameter p
juce::String opId(int op, dx7::Patch::Op p);

juce::AudioProcessorValueTreeState::ParameterLayout createLayout();

// A note as the DX7 names it: middle C is C3.
juce::String noteName(int midi);

//
// Reads every parameter on the audio thread, without a lookup by name: the voice as a
// dx7::Patch (with `name` for the ten characters a parameter cannot hold), and the settings.
//
class Reader
{
public:
    explicit Reader(juce::AudioProcessorValueTreeState& state);

    void readPatch(dx7::Patch& p) const;
    dx7::Settings readSettings() const;
    float volumeDb() const;

private:
    juce::AudioProcessorValueTreeState& state;
    std::vector<std::atomic<float>*> voiceValues, fnValues;   // fnValues in function() order
    std::atomic<float>* volume = nullptr;
};

// Write a whole voice into the parameters - a program change, a cartridge loaded, INIT.
void writePatch(const dx7::Patch& p, juce::AudioProcessorValueTreeState& state);

} // namespace params
