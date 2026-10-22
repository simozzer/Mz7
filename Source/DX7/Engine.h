// Mz7 - Created by Simon Moscrop
// Free software under the GNU AGPL v3; see LICENSE.

#pragma once

#include "Algorithms.h"
#include "Envelopes.h"
#include "Lfo.h"
#include "Ops.h"
#include "Patch.h"
#include "Resampler.h"

#include <array>
#include <cstdint>

namespace dx7
{

//
// The DX7's FUNCTION settings: what the machine keeps for itself rather than in a voice.
//
struct Settings
{
    int  pitchBendRange = 2;      // 0-12 semitones
    int  pitchBendStep  = 0;      // 0 = smooth, else bends in steps of this many semitones
    bool mono           = false;
    int  portaMode      = 0;      // poly: 0 retain, 1 follow; mono: 0 fingered, 1 full time
    bool portaGliss     = false;
    int  portaTime      = 0;      // 0-99, 0 = no portamento

    // what each controller reaches, and how far (0-99)
    struct Route { int range = 0; bool pitch = false, amp = false, egBias = false; };
    Route wheel { 50, true, false, false };
    Route foot, breath, aftertouch;

    int    voices     = 16;       // 1-16
    double masterTune = 0.0;      // cents, +-100
    int    opMask     = 0x3f;     // the front panel's operator on/off switches, bit 0 = OP1
};

//
// The whole instrument: sixteen voices, the shared LFO, the controllers, and the output stage.
//
// Runs at the DX7's own rate (49,096 Hz) and resamples to the host's. Single-threaded: call
// everything from the audio thread; the plugin hands patches and settings across itself.
//
class Engine
{
public:
    static constexpr double NativeRate = 49096.0;
    static constexpr int    MaxVoices  = 16;

    Engine();

    void prepare(double hostRate);

    void setPatch(const Patch& p);        // held notes follow, as on the DX7 while editing
    const Patch& patch() const { return current; }
    void setSettings(const Settings& s);

    void noteOn(int note, int velocity);
    void noteOff(int note);
    void sustain(bool down);
    void pitchBend(int value);            // -8192..8191
    void controller(int cc, int value);   // 1 wheel, 2 breath, 4 foot, 5 porta time, 65 porta, 64 sustain, 123 all off
    void aftertouch(int value);
    void allNotesOff();
    void allSoundOff();

    void render(float* out, int n);

    // one sample at the native rate, before the output stage - for the checks
    float renderNative();

    int activeVoices() const;

private:
    struct Op
    {
        uint32_t    phase = 0, inc = 0;
        OpEnvelope  env;
        double      basePitch = 0, portaPitch = 0;    // 2^24ths of an octave
        int         outLevel = 0;
        int         ampSens = 0;
        bool        fixed = false;
        int32_t     out = 0;                          // this sample's output, for the ops below
        double      ampScale = 1.0;                   // the amp-mod factor for this block
    };

    struct Voice
    {
        std::array<Op, 7> op;                         // 1-6
        PitchEnvelope     pitchEnv;
        int     note = -1, velocity = 0, played = -1; // played: the key, before transpose
        bool    keyDown = false, sustained = false, active = false;
        int32_t fb[2] {};
        uint64_t age = 0;
    };

    void startVoice(Voice& v, int key, int velocity, bool retrigger);
    void setupOps(Voice& v, bool start);
    void releaseVoice(Voice& v);
    void updateBlock();
    int32_t voiceSample(Voice& v);
    double  sampleNative();
    int     findVoice();

    Patch    current;
    Settings settings;

    std::array<Voice, MaxVoices> voices;
    uint64_t clock = 0;

    // the shared modulation, worked out once a block
    Lfo     lfo;
    int32_t lfoValue = 1 << 23, lfoFade = 0;
    double  pitchMod = 0, pitchBase = 0;    // ratio ops get both, fixed ops the base only
    double  ampModDepth = 0;                // 0..2^24
    int     blockLeft = 0;

    int  bend = 0, wheel = 0, breath = 0, foot = 0, pressure = 0;
    bool pedal = false, portaOn = true;
    double lastPitch = -1;                  // for portamento

    // mono: the keys held, newest last
    int held[128] {}, heldCount = 0;

    // output: a two-pole low-pass at 16 kHz, as after the DX7's DAC, then the resampler
    double lpA[3] {}, lpB[3] {}, lpX[2] {}, lpY[2] {};
    Resampler resampler;
    double hostRate = 48000.0;
};

} // namespace dx7
