// Mz7 - Created by Simon Moscrop
// Free software under the GNU AGPL v3; see LICENSE.
//
// The engine's own checks, with no JUCE: pitch, level, the sysex round trip, and that
// nothing comes out as a NaN or runs away. Run before the plugin is built.
//

#include "Engine.h"
#include "Patch.h"

#include <cmath>
#include <cstdio>
#include <vector>

using namespace dx7;

namespace
{
    int checks = 0, failures = 0;

    void check(bool ok, const char* what, const char* detail = "")
    {
        ++checks;
        if (! ok) ++failures;
        std::printf("%s  %-58s %s\n", ok ? "ok  " : "FAIL", what, detail);
    }

    // the frequency of a steady tone, by counting rising zero crossings
    double frequency(const std::vector<float>& x, double rate)
    {
        int first = -1, last = -1, n = 0;
        for (size_t i = 1; i < x.size(); ++i)
            if (x[i - 1] < 0.0f && x[i] >= 0.0f)
            {
                if (first < 0) first = (int) i;
                last = (int) i;
                ++n;
            }
        return n > 1 ? (n - 1) * rate / (last - first) : 0.0;
    }

    double peak(const std::vector<float>& x, size_t from = 0)
    {
        double p = 0;
        for (size_t i = from; i < x.size(); ++i) p = std::fmax(p, std::fabs(x[i]));
        return p;
    }

    std::vector<float> play(Engine& e, int note, int velocity, int samples, int releaseAt = -1)
    {
        std::vector<float> out((size_t) samples);
        e.noteOn(note, velocity);
        if (releaseAt < 0) releaseAt = samples;
        e.render(out.data(), releaseAt);
        e.noteOff(note);
        if (releaseAt < samples) e.render(out.data() + releaseAt, samples - releaseAt);
        return out;
    }
}

int main()
{
    const double rate = 48000.0;
    char buf[160];

    // ---- INIT VOICE: OP1 alone, a sine
    {
        Engine e;
        e.prepare(rate);
        e.setPatch(Patch::init());
        auto x = play(e, 69, 100, 48000);
        std::vector<float> steady(x.begin() + 4800, x.end());
        const double f = frequency(steady, rate);
        std::snprintf(buf, sizeof buf, "%.3f Hz", f);
        check(std::fabs(f - 440.0) < 0.05, "INIT VOICE at A4 is 440 Hz", buf);
        std::snprintf(buf, sizeof buf, "peak %.3f", peak(x));
        // OP1 alone, but algorithm 1 has two carriers, so the chip halves it: a quarter of full scale
        check(peak(x) > 0.09 && peak(x) < 0.14, "and at a quarter of a voice's full scale", buf);
    }

    // ---- every algorithm, every operator at full level with feedback: finite and bounded
    {
        bool fine = true;
        double loudest = 0;
        for (int alg = 0; alg < 32; ++alg)
        {
            Patch p = Patch::init();
            p[Patch::Algorithm] = (uint8_t) alg;
            p[Patch::Feedback] = 7;
            for (int o = 1; o <= 6; ++o) p.op(o, Patch::OutputLevel) = 99;

            Engine e;
            e.prepare(rate);
            e.setPatch(p);
            e.noteOn(48, 127); e.noteOn(60, 127); e.noteOn(67, 127);
            std::vector<float> x(9600);
            e.render(x.data(), (int) x.size());
            for (float s : x) if (! std::isfinite(s)) fine = false;
            loudest = std::fmax(loudest, peak(x));
        }
        std::snprintf(buf, sizeof buf, "loudest %.3f", loudest);
        check(fine && loudest < 2.0, "all 32 algorithms, full level and feedback, stay finite", buf);
    }

    // ---- a release dies away
    {
        Engine e;
        e.prepare(rate);
        Patch p = Patch::init();
        p.op(1, Patch::R4) = 60;
        e.setPatch(p);
        auto x = play(e, 60, 100, 96000, 24000);
        std::vector<float> tail(x.end() - 4800, x.end());
        std::snprintf(buf, sizeof buf, "tail %.2e, %d voices", peak(tail), e.activeVoices());
        check(peak(tail) < 1e-3 && e.activeVoices() == 0, "a released note dies and frees its voice", buf);
    }

    // ---- sysex: a cartridge out and back in is the same cartridge
    {
        Cartridge c;
        for (int i = 0; i < 32; ++i)
        {
            Patch p = Patch::init();
            for (int b = 0; b < Patch::Size; ++b)
                p[b] = (uint8_t) ((b * 7 + i * 13) % (Patch::maxOf(b) + 1));
            p.setName("VOICE " + std::to_string(i));
            p.clamp();
            c.voices[(size_t) i] = p;
        }
        const auto s = cartridgeSysex(c);
        Cartridge back;
        int count = 0;
        std::string error, warning;
        const bool read = readSysex(s.data(), s.size(), back, count, error, warning);
        bool same = read && count == 32 && warning.empty();
        for (int i = 0; i < 32 && same; ++i) same = back.voices[(size_t) i] == c.voices[(size_t) i];
        std::snprintf(buf, sizeof buf, "%zu bytes", s.size());
        check(same && s.size() == 4104, "a 32-voice cartridge survives the sysex round trip", buf);

        const auto one = voiceSysex(c.voices[5]);
        Cartridge single;
        const bool r1 = readSysex(one.data(), one.size(), single, count, error, warning);
        check(r1 && count == 1 && single.voices[0] == c.voices[5], "and so does a single voice");
    }

    std::printf("\n%d checks, %d failed\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
