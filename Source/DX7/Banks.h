// Mz7 - Created by Simon Moscrop
// Free software under the GNU AGPL v3; see LICENSE.

#pragma once

#include "Patch.h"

#include <memory>
#include <string>

//
// The four cartridges Mz7 comes with: 128 original voices, programmed for it in the DX7's own
// voice format - none of them is Yamaha's. Each is a dx7::Cartridge, so it saves out as an
// ordinary .syx a real DX7 or any DX7 editor will load.
//
namespace banks
{
    int count();
    std::string name(int bank);
    const dx7::Cartridge& get(int bank);
}

namespace dx7
{

//
// A voice, written the way a DX7 programming sheet reads: the algorithm and feedback, then
// each operator's frequency, level and envelope, then the LFO and pitch envelope. Anything
// not mentioned stays as INIT VOICE has it.
//
class VoiceBuilder
{
public:
    VoiceBuilder(const char* name, int algorithm, int feedback = 0)
    {
        p = Patch::init();
        p.setName(name);
        p[Patch::Algorithm] = (uint8_t) (algorithm - 1);
        p[Patch::Feedback]  = (uint8_t) feedback;
        for (int o = 1; o <= 6; ++o) p.op(o, Patch::OutputLevel) = 0;
    }

    class Op
    {
    public:
        Op(VoiceBuilder& v, int n) : vb(v), index(n) {}

        // frequency ratio: coarse 0 is 0.5, then 1-31; fine adds fine% of that; detune 0-14, 7 none
        Op& ratio(int coarse, int fine = 0, int detune = 7)
        {
            set(Patch::Mode, 0); set(Patch::Coarse, coarse); set(Patch::Fine, fine); set(Patch::Detune, detune);
            return *this;
        }
        // fixed frequency: 10^coarse Hz (coarse 0-3) times 10^(fine/100)
        Op& fixed(int coarse, int fine, int detune = 7)
        {
            set(Patch::Mode, 1); set(Patch::Coarse, coarse); set(Patch::Fine, fine); set(Patch::Detune, detune);
            return *this;
        }
        Op& level(int ol)   { set(Patch::OutputLevel, ol); return *this; }
        Op& vel(int kvs)    { set(Patch::VelocitySens, kvs); return *this; }
        Op& rate(int rs)    { set(Patch::RateScaling, rs); return *this; }
        Op& ams(int a)      { set(Patch::AmpModSens, a); return *this; }

        Op& eg(int r1, int r2, int r3, int r4, int l1, int l2, int l3, int l4)
        {
            const int v[8] = { r1, r2, r3, r4, l1, l2, l3, l4 };
            for (int i = 0; i < 8; ++i) set((Patch::Op) (Patch::R1 + i), v[i]);
            return *this;
        }

        // level scaling: break point (39 = C3), depths, curves (0 -LIN, 1 -EXP, 2 +EXP, 3 +LIN)
        Op& scale(int bp, int left, int right, int leftCurve = 0, int rightCurve = 0)
        {
            set(Patch::BreakPoint, bp); set(Patch::LeftDepth, left); set(Patch::RightDepth, right);
            set(Patch::LeftCurve, leftCurve); set(Patch::RightCurve, rightCurve);
            return *this;
        }

        Op& op(int n) { return vb.op(n); }
        VoiceBuilder& done() { return vb; }

    private:
        void set(Patch::Op f, int v) { vb.p.op(index, f) = (uint8_t) v; }
        VoiceBuilder& vb;
        int index;
    };

    Op& op(int n)
    {
        ops[n] = std::make_unique<Op>(*this, n);
        return *ops[n];
    }

    // speed, delay, pitch depth, amp depth, wave (0 tri, 1 saw down, 2 saw up, 3 square, 4 sine, 5 S&H), PMS, key sync
    VoiceBuilder& lfo(int speed, int delay, int pmd, int amd, int wave = 0, int pms = 3, int sync = 0)
    {
        p[Patch::LfoSpeed] = (uint8_t) speed; p[Patch::LfoDelay] = (uint8_t) delay;
        p[Patch::LfoPitchDepth] = (uint8_t) pmd; p[Patch::LfoAmpDepth] = (uint8_t) amd;
        p[Patch::LfoWave] = (uint8_t) wave; p[Patch::PitchModSens] = (uint8_t) pms; p[Patch::LfoKeySync] = (uint8_t) sync;
        return *this;
    }

    VoiceBuilder& pitchEg(int r1, int r2, int r3, int r4, int l1, int l2, int l3, int l4)
    {
        const int v[8] = { r1, r2, r3, r4, l1, l2, l3, l4 };
        for (int i = 0; i < 8; ++i) p[Patch::PR1 + i] = (uint8_t) v[i];
        return *this;
    }

    VoiceBuilder& transpose(int semitones) { p[Patch::Transpose] = (uint8_t) (24 + semitones); return *this; }
    VoiceBuilder& keySync(bool on)         { p[Patch::OscKeySync] = on ? 1 : 0; return *this; }

    operator Patch() const { Patch q = p; q.clamp(); return q; }

private:
    friend class Op;
    Patch p;
    std::unique_ptr<Op> ops[7];
};

} // namespace dx7
