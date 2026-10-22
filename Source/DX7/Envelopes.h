// Mz7 - Created by Simon Moscrop
// Free software under the GNU AGPL v3; see LICENSE.

#pragma once

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace dx7
{

//
// The DX7's envelopes, as the EGS chip runs them.
//
// The behaviour is the one measured on real machines for MSFA (Raph Levien's
// music-synthesizer-for-android, as carried on in Dexed): the rate curve, the way an attack
// jumps the first stretch and then slows as it nears the top, the 0.75 dB steps of the level
// scale, and the pause a stage takes even when it has nowhere to go. MSFA steps its
// envelopes once every 64 samples; these step every sample, with the same rates, so a fast
// attack lands where MSFA's does without the staircase.
//
// LEVELS are in octaves of gain, as a fixed-point count with 2^24 to the octave - the same
// scale as the sound chip's attenuation, which is where they end up.
//
namespace env
{
    // output levels 0-19 are not evenly spaced on the DX7; 20 and up are a straight line
    inline int scaleOutputLevel(int level)
    {
        static const int lut[20] = { 0, 5, 9, 13, 17, 20, 23, 25, 27, 29, 31, 33, 35, 37, 39, 41, 42, 43, 45, 46 };
        return level >= 20 ? 28 + level : lut[level < 0 ? 0 : level];
    }

    //
    // How long a stage takes when its level does not move (seconds x 44100), measured on a
    // DX7 by MSFA: rates 0-76, then a straight line to 99.
    //
    inline int staticSamples44k(int rate)
    {
        static const int t[77] = {
            1764000, 1764000, 1411200, 1411200, 1190700, 1014300, 992250,
            882000, 705600, 705600, 584325, 507150, 502740, 441000, 418950,
            352800, 308700, 286650, 253575, 220500, 220500, 176400, 145530,
            145530, 125685, 110250, 110250, 88200, 88200, 74970, 61740,
            61740, 55125, 48510, 44100, 37485, 31311, 30870, 27562, 27562,
            22050, 18522, 17640, 15435, 14112, 13230, 11025, 9261, 9261, 7717,
            6615, 6615, 5512, 5512, 4410, 3969, 3969, 3439, 2866, 2690, 2249,
            1984, 1896, 1808, 1411, 1367, 1234, 1146, 926, 837, 837, 705,
            573, 573, 529, 441, 441
        };
        return rate < 77 ? t[rate] : 20 * (99 - rate);
    }
}

//
// An operator's amplitude envelope: four rates, four levels, L4 where it rests.
//
class OpEnvelope
{
public:
    void prepare(double sampleRate)
    {
        // MSFA's increments are per 64 samples at 44.1 kHz
        perSample = 44100.0 / sampleRate / 64.0;
        fs = sampleRate;
    }

    void start(const int r[4], const int l[4], int outlevel, int rateScaling)
    {
        set(r, l, outlevel, rateScaling);
        level = 0;
        down = true;
        advance(0);
    }

    // A held note following a patch edit: the new settings, from where it stands.
    void update(const int r[4], const int l[4], int outlevel, int rateScaling)
    {
        set(r, l, outlevel, rateScaling);
        if (down) advance(ix < 3 ? ix : 2);
    }

    void key(bool isDown)
    {
        if (down == isDown) return;
        down = isDown;
        advance(isDown ? 0 : 3);
    }

    // one sample on; the level, in 2^24ths of an octave
    double next()
    {
        if (holdSamples > 0)
        {
            holdSamples -= 1.0;
            if (holdSamples <= 0.0) { holdSamples = 0.0; advance(ix + 1); }
        }

        if (ix < 3 || (ix < 4 && ! down))
        {
            if (holdSamples > 0.0)
                ;
            else if (rising)
            {
                // The attack jumps straight past the bottom of its range, then climbs
                // faster the further it is from the top - in whole steps, as the chip does.
                const double jump = 1716.0 * 65536.0;
                if (level < jump) level = jump;
                level += std::floor((17.0 * 16777216.0 - level) / 16777216.0) * inc;
                if (level >= target) { level = target; advance(ix + 1); }
            }
            else
            {
                level -= inc;
                if (level <= target) { level = target; advance(ix + 1); }
            }
        }
        return level;
    }

    // still sounding: not yet at rest, or resting somewhere above silence
    bool active() const { return ix < 4 || levels[3] > 0; }
    int  stage() const  { return ix; }

private:
    void set(const int r[4], const int l[4], int outlevel, int rateScaling)
    {
        for (int i = 0; i < 4; ++i) { rates[i] = r[i]; levels[i] = l[i]; }
        out = outlevel;
        scaling = rateScaling;
    }

    void advance(int newIx)
    {
        ix = newIx;
        if (ix >= 4) return;

        const int newLevel = levels[ix];
        int actual = ((env::scaleOutputLevel(newLevel) >> 1) << 6) + out - 4256;
        if (actual < 16) actual = 16;
        target = actual * 65536.0;
        rising = target > level;

        int q = (rates[ix] * 41) >> 6;
        q = std::min(q + scaling, 63);
        inc = (double) ((4 + (q & 3)) << (2 + 6 + (q >> 2))) * perSample;

        // A stage with nowhere to go still takes its time - and an attack to level 0 too,
        // a twentieth as long.
        holdSamples = 0.0;
        if (target == level || (ix == 0 && newLevel == 0))
        {
            const int r = std::min(rates[ix] + scaling, 99);
            double t = env::staticSamples44k(r);
            if (r < 77 && ix == 0 && newLevel == 0) t = std::floor(t / 20.0);
            holdSamples = t * fs / 44100.0;
        }
    }

    int rates[4] {}, levels[4] {};
    int out = 0, scaling = 0;

    double level = 0, target = 0, inc = 0, holdSamples = 0;
    double perSample = 44100.0 / 49096.0 / 64.0, fs = 49096.0;
    int  ix = 4;
    bool down = false, rising = false;
};

//
// The pitch envelope: one for the whole voice, four rates and levels like the others, but
// moving in pitch - level 50 is no shift, 0 and 99 are four octaves down and up. Linear in
// pitch, at the rates MSFA measured.
//
class PitchEnvelope
{
public:
    void prepare(double sampleRate) { unit = 16777216.0 / (21.3 * sampleRate); }

    void start(const int r[4], const int l[4])
    {
        for (int i = 0; i < 4; ++i) { rates[i] = r[i]; levels[i] = l[i]; }
        level = shiftOf(levels[3]);
        down = true;
        advance(0);
    }

    void update(const int r[4], const int l[4])
    {
        for (int i = 0; i < 4; ++i) { rates[i] = r[i]; levels[i] = l[i]; }
        if (ix < 4) advance(ix);
    }

    void key(bool isDown)
    {
        if (down == isDown) return;
        down = isDown;
        advance(isDown ? 0 : 3);
    }

    // the shift, in 2^24ths of an octave
    double next()
    {
        if (ix < 3 || (ix < 4 && ! down))
        {
            if (rising)
            {
                level += inc;
                if (level >= target) { level = target; advance(ix + 1); }
            }
            else
            {
                level -= inc;
                if (level <= target) { level = target; advance(ix + 1); }
            }
        }
        return level;
    }

private:
    static double shiftOf(int l)
    {
        static const int8_t tab[100] = {
            -128, -116, -104, -95, -85, -76, -68, -61, -56, -52, -49, -46, -43,
            -41, -39, -37, -35, -33, -32, -31, -30, -29, -28, -27, -26, -25, -24,
            -23, -22, -21, -20, -19, -18, -17, -16, -15, -14, -13, -12, -11, -10,
            -9, -8, -7, -6, -5, -4, -3, -2, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
            11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27,
            28, 29, 30, 31, 32, 33, 34, 35, 38, 40, 43, 46, 49, 53, 58, 65, 73,
            82, 92, 103, 115, 127
        };
        return tab[std::clamp(l, 0, 99)] * 524288.0;   // << 19
    }

    void advance(int newIx)
    {
        ix = newIx;
        if (ix >= 4) return;

        static const uint8_t rate[100] = {
            1, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12,
            12, 13, 13, 14, 14, 15, 16, 16, 17, 18, 18, 19, 20, 21, 22, 23, 24,
            25, 26, 27, 28, 30, 31, 33, 34, 36, 37, 38, 39, 41, 42, 44, 46, 47,
            49, 51, 53, 54, 56, 58, 60, 62, 64, 66, 68, 70, 72, 74, 76, 79, 82,
            85, 88, 91, 94, 98, 102, 106, 110, 115, 120, 125, 130, 135, 141, 147,
            153, 159, 165, 171, 178, 185, 193, 202, 211, 232, 243, 254, 255
        };

        target = shiftOf(levels[ix]);
        rising = target > level;
        inc = rate[std::clamp(rates[ix], 0, 99)] * unit;
    }

    int rates[4] {}, levels[4] {};
    double level = 0, target = 0, inc = 0, unit = 0;
    int  ix = 4;
    bool down = false, rising = false;
};

} // namespace dx7
