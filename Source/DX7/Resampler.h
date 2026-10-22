// Mz7 - Created by Simon Moscrop
// Free software under the GNU AGPL v3; see LICENSE.

#pragma once

#include <cmath>
#include <vector>

namespace dx7
{

//
// From the DX7's own sample rate to the host's.
//
// The engine runs at 49,096 Hz, as the chip did, so its aliasing - an FM spectrum folding
// back off that rate - is the DX7's rather than whatever the host's rate would make of it.
// This brings the result to the host's rate: a windowed sinc, 32 taps, interpolated between
// 512 phases, cutting off a little under the lower of the two Nyquists. At 49,096 to 44,100
// that is about 20 kHz, above the DX7's own 16 kHz output filter.
//
class Resampler
{
public:
    void prepare(double inRate, double outRate)
    {
        step = inRate / outRate;
        const double cutoff = 0.5 * std::fmin(1.0, outRate / inRate) * 0.92;   // of the input rate

        table.assign((size_t) (Phases + 1) * Taps, 0.0f);
        for (int p = 0; p <= Phases; ++p)
        {
            const double frac = (double) p / Phases;
            double sum = 0.0;
            for (int k = 0; k < Taps; ++k)
            {
                const double x = (k - Taps / 2 + 1) - frac;      // distance in input samples
                const double s = x == 0.0 ? 2.0 * cutoff
                                          : std::sin(2.0 * 3.141592653589793 * cutoff * x) / (3.141592653589793 * x);
                const double w = kaiser(x / (Taps / 2.0));
                table[(size_t) (p * Taps + k)] = (float) (s * w);
                sum += s * w;
            }
            for (int k = 0; k < Taps; ++k) table[(size_t) (p * Taps + k)] = (float) (table[(size_t) (p * Taps + k)] / sum);
        }

        history.assign(Taps * 2, 0.0f);
        write = 0;
        position = 0.0;
    }

    //
    // Fill `out` with `n` samples, pulling input from `source()` as it is needed.
    //
    template <class Source>
    void process(float* out, int n, Source&& source)
    {
        for (int i = 0; i < n; ++i)
        {
            while (position >= 1.0)
            {
                push(source());
                position -= 1.0;
            }

            const double fp   = position * Phases;
            const int    p    = (int) fp;
            const float  mix  = (float) (fp - p);
            const float* a    = &table[(size_t) (p * Taps)];
            const float* b    = &table[(size_t) ((p + 1) * Taps)];
            const float* h    = &history[(size_t) write];      // the newest Taps samples, oldest first

            float acc = 0.0f;
            for (int k = 0; k < Taps; ++k)
                acc += h[k] * (a[k] + (b[k] - a[k]) * mix);

            out[i] = acc;
            position += step;
        }
    }

private:
    static constexpr int Taps = 32, Phases = 512;

    static double kaiser(double x)
    {
        if (std::fabs(x) > 1.0) return 0.0;
        const double beta = 8.0;
        return bessel(beta * std::sqrt(1.0 - x * x)) / bessel(beta);
    }

    static double bessel(double x)
    {
        double sum = 1.0, term = 1.0;
        for (int k = 1; k < 30; ++k)
        {
            term *= (x / (2.0 * k)) * (x / (2.0 * k));
            sum += term;
        }
        return sum;
    }

    // a doubled ring: every sample is written twice, so the latest Taps are always contiguous
    void push(float v)
    {
        history[(size_t) write] = v;
        history[(size_t) (write + Taps)] = v;
        write = (write + 1) % Taps;
    }

    std::vector<float> table, history;
    int    write = 0;
    double step = 1.0, position = 0.0;
};

} // namespace dx7
