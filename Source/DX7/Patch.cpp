// Mz7 - Created by Simon Moscrop
// Free software under the GNU AGPL v3; see LICENSE.

#include "Patch.h"

#include <algorithm>
#include <cstring>

namespace dx7
{

namespace
{
    constexpr int opMax[Patch::OpSize] = {
        99, 99, 99, 99, 99, 99, 99, 99,   // rates, levels
        99, 99, 99, 3, 3,                 // break point, depths, curves
        7, 3, 7, 99,                      // rate scaling, AMS, KVS, output level
        1, 31, 99, 14                     // mode, coarse, fine, detune
    };

    constexpr int voiceMax[] = {
        99, 99, 99, 99, 99, 99, 99, 99,   // pitch EG
        31, 7, 1,                         // algorithm, feedback, osc key sync
        99, 99, 99, 99, 1, 5, 7,          // LFO
        48                                // transpose
    };

    uint8_t checksum(const uint8_t* data, size_t n)
    {
        int sum = 0;
        for (size_t i = 0; i < n; ++i) sum += data[i];
        return (uint8_t) ((128 - (sum & 127)) & 127);
    }
}

std::string Patch::name() const
{
    std::string s(reinterpret_cast<const char*>(&v[Name]), 10);
    for (auto& c : s)
        if (c < 32 || c > 126) c = ' ';
    return s;
}

void Patch::setName(const std::string& n)
{
    for (int i = 0; i < 10; ++i)
        v[(size_t) (Name + i)] = (uint8_t) (i < (int) n.size() ? n[(size_t) i] : ' ');
}

int Patch::maxOf(int index)
{
    if (index < 126)  return opMax[index % OpSize];
    if (index < Name) return voiceMax[index - 126];
    return 127;
}

void Patch::clamp()
{
    for (int i = 0; i < Name; ++i)
        v[(size_t) i] = (uint8_t) std::min<int>(v[(size_t) i], maxOf(i));

    for (int i = Name; i < Size; ++i)
        if (v[(size_t) i] < 32 || v[(size_t) i] > 126) v[(size_t) i] = ' ';
}

Patch Patch::init()
{
    Patch p;
    for (int op = 1; op <= 6; ++op)
    {
        p.op(op, R1) = p.op(op, R2) = p.op(op, R3) = p.op(op, R4) = 99;
        p.op(op, L1) = p.op(op, L2) = p.op(op, L3) = 99;
        p.op(op, L4) = 0;
        p.op(op, BreakPoint) = 39;
        p.op(op, OutputLevel) = op == 1 ? 99 : 0;
        p.op(op, Coarse) = 1;
        p.op(op, Detune) = 7;
    }

    for (int i = 0; i < 4; ++i)
    {
        p[PR1 + i] = 99;
        p[PL1 + i] = 50;
    }

    p[OscKeySync]   = 1;
    p[LfoSpeed]     = 35;
    p[LfoKeySync]   = 1;
    p[PitchModSens] = 3;
    p[Transpose]    = 24;
    p.setName("INIT VOICE");
    return p;
}

// --------------------------------------------------------------------------- packing

void packVoice(const Patch& p, uint8_t out[128])
{
    for (int op = 6; op >= 1; --op)
    {
        uint8_t* o = out + (6 - op) * 17;
        for (int i = 0; i < 11; ++i) o[i] = p.op(op, (Patch::Op) i);   // rates .. right depth

        o[11] = (uint8_t) ((p.op(op, Patch::RightCurve) << 2) | p.op(op, Patch::LeftCurve));
        o[12] = (uint8_t) ((p.op(op, Patch::Detune) << 3) | p.op(op, Patch::RateScaling));
        o[13] = (uint8_t) ((p.op(op, Patch::VelocitySens) << 2) | p.op(op, Patch::AmpModSens));
        o[14] = p.op(op, Patch::OutputLevel);
        o[15] = (uint8_t) ((p.op(op, Patch::Coarse) << 1) | p.op(op, Patch::Mode));
        o[16] = p.op(op, Patch::Fine);
    }

    for (int i = 0; i < 8; ++i) out[102 + i] = p[Patch::PR1 + i];
    out[110] = p[Patch::Algorithm];
    out[111] = (uint8_t) ((p[Patch::OscKeySync] << 3) | p[Patch::Feedback]);
    out[112] = p[Patch::LfoSpeed];
    out[113] = p[Patch::LfoDelay];
    out[114] = p[Patch::LfoPitchDepth];
    out[115] = p[Patch::LfoAmpDepth];
    out[116] = (uint8_t) ((p[Patch::PitchModSens] << 4) | (p[Patch::LfoWave] << 1) | p[Patch::LfoKeySync]);
    out[117] = p[Patch::Transpose];
    for (int i = 0; i < 10; ++i) out[118 + i] = p[Patch::Name + i];

    for (int i = 0; i < 128; ++i) out[i] &= 0x7f;   // seven-bit sysex data, always
}

Patch unpackVoice(const uint8_t in[128])
{
    Patch p;
    for (int op = 6; op >= 1; --op)
    {
        const uint8_t* o = in + (6 - op) * 17;
        for (int i = 0; i < 11; ++i) p.op(op, (Patch::Op) i) = o[i];

        p.op(op, Patch::LeftCurve)    = o[11] & 3;
        p.op(op, Patch::RightCurve)   = (o[11] >> 2) & 3;
        p.op(op, Patch::RateScaling)  = o[12] & 7;
        p.op(op, Patch::Detune)       = (o[12] >> 3) & 15;
        p.op(op, Patch::AmpModSens)   = o[13] & 3;
        p.op(op, Patch::VelocitySens) = (o[13] >> 2) & 7;
        p.op(op, Patch::OutputLevel)  = o[14];
        p.op(op, Patch::Mode)         = o[15] & 1;
        p.op(op, Patch::Coarse)       = (o[15] >> 1) & 31;
        p.op(op, Patch::Fine)         = o[16];
    }

    for (int i = 0; i < 8; ++i) p[Patch::PR1 + i] = in[102 + i];
    p[Patch::Algorithm]     = in[110] & 31;
    p[Patch::Feedback]      = in[111] & 7;
    p[Patch::OscKeySync]    = (in[111] >> 3) & 1;
    p[Patch::LfoSpeed]      = in[112];
    p[Patch::LfoDelay]      = in[113];
    p[Patch::LfoPitchDepth] = in[114];
    p[Patch::LfoAmpDepth]   = in[115];
    p[Patch::LfoKeySync]    = in[116] & 1;
    p[Patch::LfoWave]       = (in[116] >> 1) & 7;
    p[Patch::PitchModSens]  = (in[116] >> 4) & 7;
    p[Patch::Transpose]     = in[117];
    for (int i = 0; i < 10; ++i) p[Patch::Name + i] = in[118 + i];

    p.clamp();
    return p;
}

// ----------------------------------------------------------------------------- sysex

bool readSysex(const uint8_t* data, size_t size, Cartridge& out, int& count,
               std::string& error, std::string& warning)
{
    count = 0;
    warning.clear();

    // a bare cartridge, no sysex framing at all
    if (size == 4096)
    {
        for (int i = 0; i < 32; ++i) out.voices[(size_t) i] = unpackVoice(data + i * 128);
        count = 32;
        return true;
    }

    // Find the first Yamaha message: some files carry a header, or more than one dump.
    for (size_t at = 0; at + 6 <= size; ++at)
    {
        if (data[at] != 0xF0 || data[at + 1] != 0x43) continue;

        const uint8_t format = data[at + 3];
        const int     length = (data[at + 4] << 7) | data[at + 5];
        const uint8_t* body  = data + at + 6;

        if (format == 9 && length == 4096 && at + 6 + 4096 + 1 <= size)
        {
            if (checksum(body, 4096) != data[at + 6 + 4096])
                warning = "the cartridge's checksum is wrong; it was loaded anyway";

            for (int i = 0; i < 32; ++i) out.voices[(size_t) i] = unpackVoice(body + i * 128);
            count = 32;
            return true;
        }

        if (format == 0 && length == Patch::Size && at + 6 + Patch::Size + 1 <= size)
        {
            if (checksum(body, Patch::Size) != data[at + 6 + Patch::Size])
                warning = "the voice's checksum is wrong; it was loaded anyway";

            Patch p;
            std::memcpy(p.v.data(), body, Patch::Size);
            p.clamp();
            out.voices[0] = p;
            count = 1;
            return true;
        }
    }

    error = "not a DX7 voice or 32-voice cartridge";
    return false;
}

std::vector<uint8_t> cartridgeSysex(const Cartridge& c)
{
    std::vector<uint8_t> s { 0xF0, 0x43, 0x00, 0x09, 0x20, 0x00 };
    uint8_t body[4096];
    for (int i = 0; i < 32; ++i) packVoice(c.voices[(size_t) i], body + i * 128);
    s.insert(s.end(), body, body + 4096);
    s.push_back(checksum(body, 4096));
    s.push_back(0xF7);
    return s;
}

std::vector<uint8_t> voiceSysex(const Patch& p)
{
    std::vector<uint8_t> s { 0xF0, 0x43, 0x00, 0x00, 0x01, 0x1B };
    s.insert(s.end(), p.v.begin(), p.v.end());
    s.push_back(checksum(p.v.data(), Patch::Size));
    s.push_back(0xF7);
    return s;
}

} // namespace dx7
