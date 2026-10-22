// Mz7 - Created by Simon Moscrop
// Free software under the GNU AGPL v3; see LICENSE.

#include "Params.h"

namespace params
{

using P = dx7::Patch;

namespace
{
    struct OpField { const char* id; const char* name; };

    // in VCED order within an operator
    const OpField opFields[P::OpSize] = {
        { "R1", "EG Rate 1" }, { "R2", "EG Rate 2" }, { "R3", "EG Rate 3" }, { "R4", "EG Rate 4" },
        { "L1", "EG Level 1" }, { "L2", "EG Level 2" }, { "L3", "EG Level 3" }, { "L4", "EG Level 4" },
        { "BP", "Break Point" }, { "LD", "Left Depth" }, { "RD", "Right Depth" },
        { "LC", "Left Curve" }, { "RC", "Right Curve" },
        { "RS", "Rate Scaling" }, { "AMS", "Amp Mod Sens" }, { "KVS", "Velocity Sens" },
        { "OL", "Output Level" }, { "Mode", "Osc Mode" }, { "Coarse", "Freq Coarse" },
        { "Fine", "Freq Fine" }, { "Detune", "Detune" }
    };

    struct VoiceField { const char* id; const char* name; };

    // bytes 126 to 144
    const VoiceField voiceFields[] = {
        { "PR1", "Pitch EG Rate 1" }, { "PR2", "Pitch EG Rate 2" }, { "PR3", "Pitch EG Rate 3" }, { "PR4", "Pitch EG Rate 4" },
        { "PL1", "Pitch EG Level 1" }, { "PL2", "Pitch EG Level 2" }, { "PL3", "Pitch EG Level 3" }, { "PL4", "Pitch EG Level 4" },
        { "alg", "Algorithm" }, { "fb", "Feedback" }, { "oks", "Osc Key Sync" },
        { "lfoSpeed", "LFO Speed" }, { "lfoDelay", "LFO Delay" }, { "lfoPmd", "LFO Pitch Mod Depth" },
        { "lfoAmd", "LFO Amp Mod Depth" }, { "lfoSync", "LFO Key Sync" }, { "lfoWave", "LFO Wave" },
        { "pms", "Pitch Mod Sens" }, { "transpose", "Transpose" }
    };
}

juce::String opId(int op, P::Op p)
{
    return "op" + juce::String(op) + opFields[p].id;
}

const std::vector<Spec>& voice()
{
    static const std::vector<Spec> list = []
    {
        std::vector<Spec> v;
        const P init = P::init();

        for (int op = 6; op >= 1; --op)
            for (int f = 0; f < P::OpSize; ++f)
            {
                const int byte = P::opBase(op) + f;
                v.push_back({ opId(op, (P::Op) f), "OP" + juce::String(op) + " " + opFields[f].name,
                              byte, 0, P::maxOf(byte), init[byte] });
            }

        for (int i = 0; i < (int) (sizeof(voiceFields) / sizeof(voiceFields[0])); ++i)
        {
            const int byte = 126 + i;
            v.push_back({ voiceFields[i].id, voiceFields[i].name, byte, 0, P::maxOf(byte), init[byte] });
        }
        return v;
    }();
    return list;
}

const std::vector<Spec>& function()
{
    static const std::vector<Spec> list = []
    {
        std::vector<Spec> f = {
            { "pbRange",    "Pitch Bend Range", -1, 0, 12, 2 },
            { "pbStep",     "Pitch Bend Step",  -1, 0, 12, 0 },
            { "mono",       "Mono",             -1, 0, 1, 0 },
            { "portaMode",  "Portamento Mode",  -1, 0, 1, 0 },
            { "portaGliss", "Glissando",        -1, 0, 1, 0 },
            { "portaTime",  "Portamento Time",  -1, 0, 99, 0 },
            { "voices",     "Polyphony",        -1, 1, 16, 16 },
            { "tune",       "Master Tune",      -1, -100, 100, 0 },
        };

        for (const char* c : { "wheel", "foot", "breath", "at" })
        {
            const juce::String id(c);
            const juce::String name = id == "wheel" ? "Mod Wheel" : id == "foot" ? "Foot"
                                    : id == "breath" ? "Breath" : "Aftertouch";
            f.push_back({ id + "Range", name + " Range", -1, 0, 99, id == "wheel" ? 50 : 0 });
            f.push_back({ id + "Pitch", name + " Pitch", -1, 0, 1, id == "wheel" ? 1 : 0 });
            f.push_back({ id + "Amp",   name + " Amp",   -1, 0, 1, 0 });
            f.push_back({ id + "Eg",    name + " EG Bias", -1, 0, 1, 0 });
        }

        for (int op = 1; op <= 6; ++op)
            f.push_back({ "op" + juce::String(op) + "On", "OP" + juce::String(op) + " On", -1, 0, 1, 1 });

        return f;
    }();
    return list;
}

juce::String noteName(int midi)
{
    // Yamaha's octave numbering: middle C, MIDI 60, is C3
    static const char* names[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
    return juce::String(names[((midi % 12) + 12) % 12]) + juce::String(midi / 12 - 2);
}

std::function<juce::String(int)> textFor(const juce::String& id)
{
    auto pick = [] (juce::StringArray names)
    {
        return [names] (int v) { return juce::isPositiveAndBelow(v, names.size()) ? names[v] : juce::String(v); };
    };

    if (id.endsWith("LC") || id.endsWith("RC")) return pick({ "-LIN", "-EXP", "+EXP", "+LIN" });
    if (id.startsWith("op") && id.endsWith("Mode")) return pick({ "RATIO", "FIXED" });
    if (id.endsWith("Detune")) return [] (int v) { return v == 7 ? juce::String("0") : (v > 7 ? "+" : "") + juce::String(v - 7); };
    if (id.endsWith("BP"))     return [] (int v) { return noteName(v + 21); };
    if (id == "alg")           return [] (int v) { return juce::String(v + 1); };
    if (id == "transpose")     return [] (int v) { return noteName(v + 36); };
    if (id == "lfoWave")       return pick({ "TRIANGLE", "SAW DOWN", "SAW UP", "SQUARE", "SINE", "S/HOLD" });
    if (id == "oks" || id == "lfoSync" || id == "portaGliss" || id.endsWith("On")
        || id.endsWith("Pitch") || id.endsWith("Amp") || id.endsWith("Eg"))
        return pick({ "OFF", "ON" });
    if (id == "mono")          return pick({ "POLY", "MONO" });
    if (id == "portaMode")     return pick({ "RETAIN", "FOLLOW" });
    if (id == "pbStep")        return [] (int v) { return v == 0 ? juce::String("OFF") : juce::String(v); };
    if (id == "tune")          return [] (int v) { return (v > 0 ? "+" : "") + juce::String(v) + " ct"; };
    return [] (int v) { return juce::String(v); };
}

juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    for (const auto* list : { &voice(), &function() })
        for (const auto& s : *list)
        {
            auto text = textFor(s.id);
            layout.add(std::make_unique<juce::AudioParameterInt>(
                juce::ParameterID { s.id, 1 }, s.name, s.min, s.max, s.def,
                juce::AudioParameterIntAttributes().withStringFromValueFunction(
                    [text] (int v, int) { return text(v); })));
        }

    layout.add(std::make_unique<juce::AudioParameterFloat>(
        juce::ParameterID { "volume", 1 }, "Volume",
        juce::NormalisableRange<float>(-40.0f, 12.0f, 0.1f), 0.0f,
        juce::AudioParameterFloatAttributes().withLabel("dB")));

    return layout;
}

// -------------------------------------------------------------------------------- reader

Reader::Reader(juce::AudioProcessorValueTreeState& s) : state(s)
{
    for (const auto& spec : voice())
        voiceValues.push_back(state.getRawParameterValue(spec.id));
    for (const auto& spec : function())
        fnValues.push_back(state.getRawParameterValue(spec.id));
    volume = state.getRawParameterValue("volume");
}

void Reader::readPatch(dx7::Patch& p) const
{
    const auto& specs = voice();
    for (size_t i = 0; i < specs.size(); ++i)
        p[specs[i].byte] = (uint8_t) juce::roundToInt(voiceValues[i]->load(std::memory_order_relaxed));
}

dx7::Settings Reader::readSettings() const
{
    // by position in function(): no names looked up on the audio thread
    auto i = [this] (int at) { return juce::roundToInt(fnValues[(size_t) at]->load(std::memory_order_relaxed)); };

    dx7::Settings s;
    s.pitchBendRange = i(0);
    s.pitchBendStep  = i(1);
    s.mono           = i(2) != 0;
    s.portaMode      = i(3);
    s.portaGliss     = i(4) != 0;
    s.portaTime      = i(5);
    s.voices         = i(6);
    s.masterTune     = i(7);

    auto route = [&] (int at, dx7::Settings::Route& r)
    {
        r.range  = i(at);
        r.pitch  = i(at + 1) != 0;
        r.amp    = i(at + 2) != 0;
        r.egBias = i(at + 3) != 0;
    };
    route(8, s.wheel);
    route(12, s.foot);
    route(16, s.breath);
    route(20, s.aftertouch);

    s.opMask = 0;
    for (int op = 1; op <= 6; ++op)
        if (i(23 + op) != 0) s.opMask |= 1 << (op - 1);

    return s;
}

float Reader::volumeDb() const
{
    return volume->load(std::memory_order_relaxed);
}

void writePatch(const dx7::Patch& p, juce::AudioProcessorValueTreeState& state)
{
    for (const auto& spec : voice())
        if (auto* param = state.getParameter(spec.id))
            param->setValueNotifyingHost(param->convertTo0to1((float) p[spec.byte]));
}

} // namespace params
