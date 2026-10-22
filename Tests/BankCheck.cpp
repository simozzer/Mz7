// Mz7 - Created by Simon Moscrop
// Free software under the GNU AGPL v3; see LICENSE.
//
// Every voice in the four cartridges, played: it must sound, stay finite, stay under full
// scale and - unless it is meant to hold, like an organ - die away once let go.
//
//     BankCheck                   the checks
//     BankCheck --wav <folder>    and a preview of each voice: a chord, a phrase, a low note
//     BankCheck --syx <folder>    the four cartridges as .syx files
//

#include "Banks.h"
#include "Engine.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

using namespace dx7;

namespace
{
    const double Rate = 48000.0;

    void writeWav(const std::string& path, const std::vector<float>& x)
    {
        std::ofstream f(path, std::ios::binary);
        auto u32 = [&] (uint32_t v) { f.write(reinterpret_cast<const char*>(&v), 4); };
        auto u16 = [&] (uint16_t v) { f.write(reinterpret_cast<const char*>(&v), 2); };
        const uint32_t bytes = (uint32_t) x.size() * 2;
        f.write("RIFF", 4); u32(36 + bytes); f.write("WAVEfmt ", 8);
        u32(16); u16(1); u16(1); u32((uint32_t) Rate); u32((uint32_t) Rate * 2); u16(2); u16(16);
        f.write("data", 4); u32(bytes);
        for (float s : x)
        {
            const int v = (int) std::lround(std::fmax(-1.0f, std::fmin(1.0f, s)) * 32767.0f);
            u16((uint16_t) (int16_t) v);
        }
    }

    // a chord, a short phrase and a low note, with gaps to hear the releases
    std::vector<float> audition(const Patch& p)
    {
        Engine e;
        e.prepare(Rate);
        e.setPatch(p);
        std::vector<float> out;

        auto run = [&] (double seconds)
        {
            const size_t n = (size_t) (seconds * Rate);
            const size_t at = out.size();
            out.resize(at + n);
            e.render(out.data() + at, (int) n);
        };

        for (int k : { 60, 64, 67, 71 }) e.noteOn(k, 100);
        run(1.6);
        for (int k : { 60, 64, 67, 71 }) e.noteOff(k);
        run(1.2);

        for (int k : { 60, 62, 64, 67, 69, 72, 76, 79 })
        {
            e.noteOn(k, k % 3 == 0 ? 120 : 85);
            run(0.22);
            e.noteOff(k);
        }
        run(1.0);

        e.noteOn(36, 110);
        run(1.4);
        e.noteOff(36);
        run(1.4);
        return out;
    }

    std::string fileName(int bank, int slot, const std::string& name)
    {
        std::string s = std::to_string(bank + 1) + "-" + (slot < 9 ? "0" : "") + std::to_string(slot + 1) + "-";
        for (char ch : name)
        {
            if (std::isalnum((unsigned char) ch)) s += (char) std::tolower((unsigned char) ch);
            else if (! s.empty() && s.back() != '-') s += '-';
        }
        while (! s.empty() && s.back() == '-') s.pop_back();
        return s;
    }
}

int main(int argc, char** argv)
{
    std::string wavDir, syxDir;
    for (int a = 1; a + 1 < argc; ++a)
    {
        if (std::strcmp(argv[a], "--wav") == 0) wavDir = argv[a + 1];
        if (std::strcmp(argv[a], "--syx") == 0) syxDir = argv[a + 1];
    }

    if (! syxDir.empty())
    {
        for (int b = 0; b < banks::count(); ++b)
        {
            std::string n = "Mz7-" + std::to_string(b + 1) + "-";
            for (char ch : banks::name(b)) n += std::isalnum((unsigned char) ch) ? ch : (n.back() == '-' ? '\0' : '-');
            n.erase(std::remove(n.begin(), n.end(), '\0'), n.end());
            const auto s = cartridgeSysex(banks::get(b));
            std::ofstream(syxDir + "/" + n + ".syx", std::ios::binary).write(reinterpret_cast<const char*>(s.data()), (std::streamsize) s.size());
            std::printf("  %s.syx\n", n.c_str());
        }
        return 0;
    }

    int checks = 0, failures = 0;

    for (int b = 0; b < banks::count(); ++b)
    {
        std::printf("\n  %s\n", banks::name(b).c_str());
        const auto& cart = banks::get(b);

        for (int s = 0; s < 32; ++s)
        {
            const Patch& p = cart.voices[(size_t) s];
            const auto x = audition(p);

            double peak = 0, energy = 0;
            bool finite = true;
            for (float v : x)
            {
                if (! std::isfinite(v)) finite = false;
                peak = std::fmax(peak, std::fabs(v));
                energy += (double) v * v;
            }
            const double rms = std::sqrt(energy / (double) x.size());

            // Does it die away once let go? A pad or a gong takes its time, so one note is
            // held for a second and then given twenty to fall below -60 dB.
            double tail = 0;
            {
                Engine e;
                e.prepare(Rate);
                e.setPatch(p);
                std::vector<float> y((size_t) (21.0 * Rate));
                e.noteOn(60, 100);
                e.render(y.data(), (int) Rate);
                e.noteOff(60);
                e.render(y.data() + (size_t) Rate, (int) (y.size() - (size_t) Rate));
                for (size_t i = y.size() - (size_t) (0.5 * Rate); i < y.size(); ++i) tail = std::fmax(tail, std::fabs(y[i]));
            }

            const bool ok = finite && peak > 0.02 && peak < 0.99 && tail < 0.001;
            ++checks;
            if (! ok) ++failures;

            std::printf("%s  %2d %-10s peak %5.2f  rms %6.1f dB  tail %7.4f\n", ok ? "ok  " : "FAIL",
                        s + 1, p.name().c_str(), peak, 20 * std::log10(rms + 1e-9), tail);

            if (! wavDir.empty())
                writeWav(wavDir + "/" + fileName(b, s, p.name()) + ".wav", x);
        }
    }

    std::printf("\n%d voices, %d failed\n", checks, failures);
    return failures == 0 ? 0 : 1;
}
