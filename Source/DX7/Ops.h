// Mz7 - Created by Simon Moscrop
// Free software under the GNU AGPL v3; see LICENSE.

#pragma once

#include <cmath>
#include <cstdint>

namespace dx7
{

//
// The arithmetic of the DX7's sound chip, the YM21280 "OPS", as Ken Shirriff read it off the
// die (righto.com, "Yamaha DX7 reverse-engineering", parts I-VI, 2021-22).
//
// THE CHIP NEVER MULTIPLIES. An operator's sine comes out of a ROM as a logarithm, the
// envelope is a logarithm too, so applying the envelope is an addition; a second ROM and a
// shifter turn the sum back into a level. Everything here is that pipeline, integer for
// integer, which is where the DX7's particular grain comes from - the steps in a quiet tail,
// the way a modulator's level moves in 0.75 dB notches.
//
// UNITS. Attenuation is in 1/1024ths of an octave (6.02 dB / 1024): the exponent ROM's ten
// fractional bits. An operator's output is a 15-bit two's-complement number, at most 16376.
// A phase is a full 32-bit turn, of which the chip's sine looks at the top twelve bits.
// Modulation is added to the phase with an operator's output lined up on those twelve bits,
// so a full-scale modulator swings the phase two whole turns either way - which is the
// DX7's familiar maximum index of about 4 pi at output level 99.
//
namespace ops
{
    //
    // The log-sine ROM: a quarter wave, 1024 entries, each -log2(sin) in 1/1024 octave, taken
    // half a step in so the mirrored half never repeats the peak and nothing is log(0). The
    // odd rounding constant is the ROM's own - Shirriff found one entry that only .5002
    // reproduces.
    //
    struct Tables
    {
        uint16_t logsin[1024];
        uint16_t exp[1024];

        Tables()
        {
            const double pi = 3.14159265358979323846;
            for (int n = 0; n < 1024; ++n)
            {
                const double w = (n + 0.5) / 1024.0 * pi / 2.0;
                logsin[n] = (uint16_t) (int) (-std::log2(std::sin(w)) * 1024.0 + 0.5002);

                // The exponent ROM: 2^(n/1024) as a 12-bit number with its leading one.
                exp[n] = (uint16_t) std::lround(std::pow(2.0, n / 1024.0) * 2048.0);
            }
        }
    };

    inline const Tables& tables()
    {
        static const Tables t;
        return t;
    }

    //
    // One operator sample: the sine at `phase`, attenuated by `atten`.
    //
    // The top bit of the twelve is the sign, the next mirrors the quarter wave. The sign
    // comes back by inverting the bits, as the chip does it, so a negative half is one
    // count further from zero than the positive one - a bias of half a count, too small to
    // hear and kept because it is the hardware's.
    //
    inline int32_t sample(uint32_t phase, int32_t atten)
    {
        const auto& t = tables();
        const uint32_t p12 = phase >> 20;

        int idx = (int) (p12 & 1023);
        if (p12 & 1024) idx ^= 1023;

        int32_t a = (int32_t) t.logsin[idx] + (atten < 0 ? 0 : atten);
        const int shift = a >> 10;
        if (shift >= 15) return (p12 & 2048) ? -1 : 0;

        // 2^(-a/1024) * 16384, by the ROM and the shifter
        const int32_t mag = ((int32_t) t.exp[(~a) & 1023] << 2) >> shift;
        return (p12 & 2048) ? ~mag : mag;
    }

    //
    // The carriers of an algorithm share the output: each is attenuated by log2 of how many
    // there are, added in the log domain like everything else - so algorithm 32's six
    // carriers together are no louder than algorithm 1's two. In the chip this is the COM
    // field of the algorithm ROM.
    //
    inline int32_t carrierAttenuation(int carriers)
    {
        static const int32_t com[7] = { 0, 0, 1024, 1623, 2048, 2378, 2647 };
        return com[carriers < 1 ? 1 : (carriers > 6 ? 6 : carriers)];
    }

    //
    // THE OUTPUT DAC. The chip hands each voice's sum to a 12-bit DAC (a BA9221) with an
    // analogue scaler after it that can divide by 1, 2, 4 or 8. A quiet voice is shifted up
    // before the twelve bits are taken and scaled back down after, so it keeps its
    // resolution: in effect a sign, an 11-bit mantissa and a 2-bit exponent. The shift is
    // chosen from the leading sign bits of the 15-bit sum, always leaving two, so a loud
    // signal never switches scale where the resistors are least exact.
    //
    // What comes out is the 15-bit value rounded down to the step that survives - which is
    // what gives a DX7 its slightly gritty quiet tails. Returned in the 15-bit units.
    //
    inline int32_t dac(int32_t v)
    {
        if (v >  16383) v =  16383;
        if (v < -16384) v = -16384;

        // leading bits equal to the sign, in a 15-bit word
        const uint32_t u = (uint32_t) (v < 0 ? ~v : v) & 0x7fff;
        int lead = 0;
        for (int b = 14; b >= 0 && ! (u & (1u << b)); --b) ++lead;

        int s = lead - 2;
        s = s < 0 ? 0 : (s > 3 ? 3 : s);

        const int step = 3 - s;                  // bits lost below the twelve
        return (v >> step) << step;
    }
}

} // namespace dx7
