// Mz7 - Created by Simon Moscrop
// Free software under the GNU AGPL v3; see LICENSE.

#pragma once

#include <cmath>
#include <cstdint>

namespace dx7
{

//
// The DX7's LFO. There is one, shared by every voice - which is why a chord's vibrato moves
// together, and why key sync restarts it for all of them at the next key.
//
// Speeds are the ones MSFA measured, 0.06 Hz at 0 to about 50 Hz at 99, and the delay is the
// DX7's two-part one: a wait, then a fade in. The output is 0..1 (in 2^24ths), centred on a
// half for the pitch and read from the top for the amplitude, as the chip uses it.
//
class Lfo
{
public:
    enum Wave { Triangle, SawDown, SawUp, Square, Sine, SampleHold };

    void prepare(double sampleRate)
    {
        fs = sampleRate;
        unit = 25190424.0 / sampleRate;
        set(speed, delay, wave, sync);
    }

    void set(int speed_, int delay_, int wave_, bool sync_)
    {
        static const double hz[100] = {
            0.062541, 0.125031, 0.312393, 0.437120, 0.624610,
            0.750694, 0.936330, 1.125302, 1.249609, 1.436782,
            1.560915, 1.752081, 1.875117, 2.062494, 2.247191,
            2.374451, 2.560492, 2.686728, 2.873976, 2.998950,
            3.188013, 3.369840, 3.500175, 3.682224, 3.812065,
            4.000800, 4.186202, 4.310716, 4.501260, 4.623209,
            4.814636, 4.930480, 5.121901, 5.315191, 5.434783,
            5.617346, 5.750431, 5.946717, 6.062811, 6.248438,
            6.431695, 6.564264, 6.749460, 6.868132, 7.052186,
            7.250580, 7.375719, 7.556294, 7.687577, 7.877738,
            7.993605, 8.181967, 8.372405, 8.504848, 8.685079,
            8.810573, 8.986341, 9.122423, 9.300595, 9.500285,
            9.607994, 9.798158, 9.950249, 10.117361, 11.251125,
            11.384335, 12.562814, 13.676149, 13.904338, 15.092062,
            16.366612, 16.638935, 17.869907, 19.193858, 19.425019,
            20.833333, 21.034918, 22.502250, 24.003841, 24.260068,
            25.746653, 27.173913, 27.578599, 29.052876, 30.693677,
            31.191516, 32.658393, 34.317090, 34.674064, 36.416606,
            38.197097, 38.550501, 40.387722, 40.749796, 42.625746,
            44.326241, 44.883303, 46.772685, 48.590865, 49.261084
        };

        speed = speed_ < 0 ? 0 : (speed_ > 99 ? 99 : speed_);
        delay = delay_ < 0 ? 0 : (delay_ > 99 ? 99 : delay_);
        wave = wave_ < 0 ? 0 : (wave_ > 5 ? 5 : wave_);
        sync = sync_;

        delta = (uint32_t) (hz[speed] * 4437500000.0 / fs);

        int a = 99 - delay;
        if (a == 99) { delayInc = delayInc2 = -1.0; return; }

        a = (16 + (a & 15)) << (1 + (a >> 4));
        delayInc = unit * a;
        int a2 = a & 0xff80;
        if (a2 < 0x80) a2 = 0x80;
        delayInc2 = unit * a2;
    }

    void keyDown()
    {
        if (sync) phase = (1u << 31) - 1;
        delayState = 0.0;
    }

    // one sample on: the LFO (0..2^24) and how far its delay has faded in (0..2^24)
    void next(int32_t& value, int32_t& fadeIn)
    {
        const uint32_t before = phase;
        phase += delta;

        switch (wave)
        {
            case Triangle:
            {
                int32_t x = (int32_t) (phase >> 7);
                x ^= -(int32_t) (phase >> 31);
                value = x & ((1 << 24) - 1);
                break;
            }
            case SawDown:    value = (int32_t) ((~phase ^ (1u << 31)) >> 8); break;
            case SawUp:      value = (int32_t) ((phase ^ (1u << 31)) >> 8); break;
            case Square:     value = (int32_t) (((~phase) >> 7) & (1u << 24)); break;
            case Sine:
                value = (1 << 23) + (int32_t) std::lround(std::sin(phase * (6.283185307179586 / 4294967296.0)) * 8388607.0);
                break;
            default:
                if (phase < before) rand = (rand * 179 + 17) & 0xff;   // a new step each cycle
                value = ((rand ^ 0x80) + 1) << 16;
                break;
        }

        fadeIn = fade();
    }

private:
    int32_t fade()
    {
        if (delayInc < 0.0) return 1 << 24;

        const double d = delayState + (delayState < 2147483648.0 ? delayInc : delayInc2);
        if (d > 4294967295.0) return 1 << 24;

        delayState = d;
        if (d < 2147483648.0) return 0;
        return (int32_t) (((uint64_t) d >> 7) & ((1u << 24) - 1));
    }

    double   fs = 49096.0, unit = 0;
    int      speed = 35, delay = 0, wave = 0;
    bool     sync = true;
    uint32_t phase = 0, delta = 0;
    double   delayInc = -1.0, delayInc2 = -1.0, delayState = 0;
    int32_t  rand = 0;
};

} // namespace dx7
