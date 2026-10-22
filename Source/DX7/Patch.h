// Mz7 - Created by Simon Moscrop
// Free software under the GNU AGPL v3; see LICENSE.

#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace dx7
{

//
// A DX7 voice, in the machine's own layout: the 155 bytes of a single-voice dump (VCED), in
// the order the service manual lists them. Keeping the voice in this form, rather than in a
// struct of named fields, means a .syx goes in and out byte for byte, and the engine reads
// exactly what the hardware would have been sent.
//
// OPERATORS ARE STORED BACKWARDS. The first 21 bytes are OP6, the last 21 OP1 - the DX7 sends
// them in the order its sound chip computes them. op(n) hides that: op(1) is OP1.
//
// Per operator (21 bytes, offset within the operator's block):
//
//     0-3   EG rates R1-R4          0-99
//     4-7   EG levels L1-L4         0-99
//     8     level scaling break point   0-99 (0 = A-1, 39 = C3, 99 = C8)
//     9     left depth              0-99
//    10     right depth             0-99
//    11     left curve              0-3  (-LIN, -EXP, +EXP, +LIN)
//    12     right curve             0-3
//    13     rate scaling            0-7
//    14     amp mod sensitivity     0-3
//    15     key velocity sensitivity 0-7
//    16     output level            0-99
//    17     oscillator mode         0 ratio, 1 fixed
//    18     frequency coarse        0-31
//    19     frequency fine          0-99
//    20     detune                  0-14 (7 = none)
//
// Then the voice (offset from 126):
//
//    126-129  pitch EG rates        0-99
//    130-133  pitch EG levels       0-99 (50 = no shift)
//    134      algorithm             0-31 (shown as 1-32)
//    135      feedback              0-7
//    136      oscillator key sync   0-1
//    137      LFO speed             0-99
//    138      LFO delay             0-99
//    139      LFO pitch mod depth   0-99
//    140      LFO amp mod depth     0-99
//    141      LFO key sync          0-1
//    142      LFO wave              0-5  (triangle, saw down, saw up, square, sine, S&H)
//    143      pitch mod sensitivity 0-7
//    144      transpose             0-48 (24 = C3, no transpose)
//    145-154  name, ten ASCII characters
//
struct Patch
{
    static constexpr int Size = 155;
    static constexpr int OpSize = 21;

    // operator parameters, offsets within an operator
    enum Op : int
    {
        R1 = 0, R2, R3, R4, L1, L2, L3, L4,
        BreakPoint, LeftDepth, RightDepth, LeftCurve, RightCurve,
        RateScaling, AmpModSens, VelocitySens, OutputLevel,
        Mode, Coarse, Fine, Detune
    };

    // voice parameters, absolute offsets
    enum Voice : int
    {
        PR1 = 126, PR2, PR3, PR4, PL1, PL2, PL3, PL4,
        Algorithm, Feedback, OscKeySync,
        LfoSpeed, LfoDelay, LfoPitchDepth, LfoAmpDepth, LfoKeySync, LfoWave, PitchModSens,
        Transpose,
        Name
    };

    std::array<uint8_t, Size> v {};

    static int opBase(int op) { return (6 - op) * OpSize; }   // op is 1-6

    uint8_t  op(int op, Op p) const        { return v[(size_t) (opBase(op) + p)]; }
    uint8_t& op(int op, Op p)              { return v[(size_t) (opBase(op) + p)]; }
    uint8_t  operator[](int i) const       { return v[(size_t) i]; }
    uint8_t& operator[](int i)             { return v[(size_t) i]; }

    std::string name() const;
    void setName(const std::string& n);

    // The largest value each byte may hold, as the DX7's own panel allows.
    static int maxOf(int index);

    // Every byte pulled into range, and a name of printable characters. A cartridge from the
    // wild sometimes carries junk in the unused bits; this is what the DX7 would make of it.
    void clamp();

    // The DX7's INIT VOICE: algorithm 1, OP1 alone at full level, everything else neutral.
    static Patch init();

    bool operator==(const Patch& o) const { return v == o.v; }
    bool operator!=(const Patch& o) const { return v != o.v; }
};

//
// The packed form a 32-voice cartridge holds: 128 bytes a voice, several small fields to a
// byte. Exactly the DX7's VMEM layout.
//
void packVoice(const Patch& p, uint8_t out[128]);
Patch unpackVoice(const uint8_t in[128]);

struct Cartridge
{
    std::array<Patch, 32> voices;
    Cartridge() { voices.fill(Patch::init()); }
};

//
// .syx in and out.
//
// Read: a 32-voice bulk dump (4104 bytes, F0 43 0n 09 20 00 ... F7), a single voice
// (163 bytes, F0 43 0n 00 01 1B ... F7), or the bare 4096 bytes of a cartridge with no sysex
// around them - all three turn up in the wild. A single voice fills the first slot and
// `count` says 1. A bad checksum is accepted, since plenty of good cartridges were saved by
// tools that got it wrong, but `warning` says so.
//
bool readSysex(const uint8_t* data, size_t size, Cartridge& out, int& count,
               std::string& error, std::string& warning);

std::vector<uint8_t> cartridgeSysex(const Cartridge& c);
std::vector<uint8_t> voiceSysex(const Patch& p);

} // namespace dx7
