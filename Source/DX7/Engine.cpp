// Mz7 - Created by Simon Moscrop
// Free software under the GNU AGPL v3; see LICENSE.

#include "Engine.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>

namespace dx7
{

namespace
{
    constexpr int    Block   = 32;               // native samples between control updates
    constexpr double Octave  = 16777216.0;       // 2^24: pitch and level are in 2^24ths of an octave
    constexpr double VoiceGain = 0.45;           // a voice at the chip's full scale, into the host

    //
    // KEYBOARD AND VELOCITY. The curves the DX7 applies to an operator's level and rate
    // across the keyboard and with how hard a key is struck - MSFA's measurements of the
    // machine, reimplemented here.
    //

    // Key velocity sensitivity: the DX7's velocity curve, in 64 steps.
    int velocityScale(int velocity, int sensitivity)
    {
        static const uint8_t curve[64] = {
            0, 70, 86, 97, 106, 114, 121, 126, 132, 138, 142, 148, 152, 156, 160, 163,
            166, 170, 173, 174, 178, 181, 184, 186, 189, 190, 194, 196, 198, 200, 202,
            205, 206, 209, 211, 214, 216, 218, 220, 222, 224, 225, 227, 229, 230, 232,
            233, 235, 237, 238, 240, 241, 242, 243, 244, 246, 246, 248, 249, 250, 251,
            252, 253, 254
        };
        const int v = curve[std::clamp(velocity, 0, 127) >> 1] - 239;
        return ((sensitivity * v + 7) >> 3) << 4;
    }

    // Rate scaling: higher keys run their envelopes faster.
    int rateScale(int note, int sensitivity)
    {
        const int x = std::clamp(note / 3 - 7, 0, 31);
        return (sensitivity * x) >> 3;
    }

    // Level scaling: either side of the break point, a linear or exponential curve up or down.
    int scaleCurve(int group, int depth, int curve)
    {
        static const uint8_t expo[] = {
            0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 11, 14, 16, 19, 23, 27, 33, 39, 47, 56, 66,
            80, 94, 110, 126, 142, 158, 174, 190, 206, 222, 238, 250
        };

        int scale;
        if (curve == 0 || curve == 3)
            scale = (group * depth * 329) >> 12;
        else
            scale = (expo[std::min(group, (int) sizeof(expo) - 1)] * depth * 329) >> 15;

        return curve < 2 ? -scale : scale;
    }

    int levelScale(int note, int breakPoint, int left, int right, int leftCurve, int rightCurve)
    {
        const int offset = note - breakPoint - 17;          // break point 0 is A-1, MIDI 21
        return offset >= 0 ? scaleCurve((offset + 1) / 3, right, rightCurve)
                           : scaleCurve(-(offset - 1) / 3, left, leftCurve);
    }

    //
    // PITCH. A key's pitch as 2^24ths of an octave, so that ratios, detune and modulation are
    // all additions. MIDI 69 is 440 Hz.
    //
    double keyPitch(int note)
    {
        return 50857777.0 + note * (Octave / 12.0);
    }

    double opPitch(int note, bool fixed, int coarse, int fine, int detune)
    {
        if (fixed)
        {
            // 1, 10, 100 or 1000 Hz, times 10^(fine/100) - in log2: 2^24 * log2(10) / 100 a step
            double p = (4458616.0 * ((coarse & 3) * 100 + fine)) / 8.0;
            if (detune > 7) p += 13457.0 * (detune - 7);
            return p;
        }

        double p = keyPitch(note);

        // Detune is a few cents, and less of a fraction of the pitch higher up: measured.
        const double ratio = 0.0209 * std::exp(-0.396 * (p / Octave)) / 7.0;
        p += ratio * p * (detune - 7);

        // coarse 0 is a ratio of 0.5, then 1, 2, 3 ... 31
        p += coarse == 0 ? -Octave : std::round(std::log2((double) coarse) * Octave);
        if (fine) p += std::floor(24204406.323123 * std::log(1.0 + 0.01 * fine) + 0.5);
        return p;
    }

    uint32_t phaseIncrement(double pitch)
    {
        const double hz = std::exp2(pitch / Octave);
        const double inc = hz / Engine::NativeRate * 4294967296.0;
        return (uint32_t) (uint64_t) std::fmod(inc, 4294967296.0);
    }

    // a controller's reach, as MSFA scales it: its value times range/100
    int reach(int value, const Settings::Route& r) { return (int) (value * (0.01 * r.range)); }
}

Engine::Engine()
{
    current = Patch::init();
    prepare(48000.0);
}

void Engine::prepare(double rate)
{
    hostRate = rate;

    for (auto& v : voices)
    {
        for (int o = 1; o <= 6; ++o) v.op[(size_t) o].env.prepare(NativeRate);
        v.pitchEnv.prepare(NativeRate / Block);
        v.active = false;
    }

    lfo.prepare(NativeRate / Block);
    lfo.set(current[Patch::LfoSpeed], current[Patch::LfoDelay], current[Patch::LfoWave], current[Patch::LfoKeySync] != 0);

    // the 16 kHz two-pole after the DAC
    const double k = std::tan(3.141592653589793 * 16000.0 / NativeRate);
    const double n = 1.0 / (1.0 + std::sqrt(2.0) * k + k * k);
    lpB[0] = k * k * n; lpB[1] = 2.0 * lpB[0]; lpB[2] = lpB[0];
    lpA[1] = 2.0 * (k * k - 1.0) * n; lpA[2] = (1.0 - std::sqrt(2.0) * k + k * k) * n;
    lpX[0] = lpX[1] = lpY[0] = lpY[1] = 0.0;

    resampler.prepare(NativeRate, hostRate);
    blockLeft = 0;
}

void Engine::setPatch(const Patch& p)
{
    current = p;
    lfo.set(p[Patch::LfoSpeed], p[Patch::LfoDelay], p[Patch::LfoWave], p[Patch::LfoKeySync] != 0);

    for (auto& v : voices)
        if (v.active) setupOps(v, false);
}

void Engine::setSettings(const Settings& s)
{
    settings = s;
    settings.voices = std::clamp(settings.voices, 1, MaxVoices);
}

// ---------------------------------------------------------------------------------- keys

int Engine::findVoice()
{
    const int limit = settings.mono ? 1 : settings.voices;

    for (int i = 0; i < limit; ++i)
        if (! voices[(size_t) i].active) return i;

    // all busy: the oldest released, else the oldest
    int best = -1;
    for (int i = 0; i < limit; ++i)
        if (! voices[(size_t) i].keyDown && (best < 0 || voices[(size_t) i].age < voices[(size_t) best].age))
            best = i;
    if (best >= 0) return best;

    best = 0;
    for (int i = 1; i < limit; ++i)
        if (voices[(size_t) i].age < voices[(size_t) best].age) best = i;
    return best;
}

void Engine::noteOn(int key, int velocity)
{
    if (velocity <= 0) { noteOff(key); return; }

    if (settings.mono)
    {
        // last-note priority; a key pressed while another is held is legato - one trigger
        for (int i = 0; i < heldCount; ++i)
            if (held[i] == key) { std::copy(held + i + 1, held + heldCount, held + i); --heldCount; break; }
        const bool legato = heldCount > 0 && voices[0].active && voices[0].keyDown;
        if (heldCount < 128) held[heldCount++] = key;

        startVoice(voices[0], key, velocity, ! legato);
        return;
    }

    // the same key again takes its own voice back, as the DX7 does
    for (auto& v : voices)
        if (v.active && v.played == key) { startVoice(v, key, velocity, true); return; }

    startVoice(voices[(size_t) findVoice()], key, velocity, true);
}

void Engine::noteOff(int key)
{
    if (settings.mono)
    {
        for (int i = 0; i < heldCount; ++i)
            if (held[i] == key) { std::copy(held + i + 1, held + heldCount, held + i); --heldCount; break; }

        auto& v = voices[0];
        if (! v.active || v.played != key) return;

        if (heldCount > 0)
            startVoice(v, held[heldCount - 1], v.velocity, false);   // back to the key still held
        else
            releaseVoice(v);
        return;
    }

    for (auto& v : voices)
        if (v.active && v.keyDown && v.played == key) releaseVoice(v);
}

void Engine::releaseVoice(Voice& v)
{
    v.keyDown = false;
    if (pedal) { v.sustained = true; return; }

    for (int o = 1; o <= 6; ++o) v.op[(size_t) o].env.key(false);
    v.pitchEnv.key(false);
}

void Engine::sustain(bool down)
{
    pedal = down;
    if (down) return;

    for (auto& v : voices)
        if (v.active && v.sustained)
        {
            v.sustained = false;
            for (int o = 1; o <= 6; ++o) v.op[(size_t) o].env.key(false);
            v.pitchEnv.key(false);
        }
}

void Engine::startVoice(Voice& v, int key, int velocity, bool retrigger)
{
    const int note = std::clamp(key + (int) current[Patch::Transpose] - 24, 0, 127);
    const bool wasActive = v.active;

    v.played   = key;
    v.note     = note;
    v.velocity = velocity;
    v.keyDown  = true;
    v.sustained = false;
    v.active   = true;
    v.age      = ++clock;

    // Portamento starts from wherever the last note was heading. Mono "fingered" glides
    // only between held keys; everything else glides whenever there is a last note.
    const bool glide = settings.portaTime > 0 && portaOn && lastPitch >= 0.0
                       && (! settings.mono || settings.portaMode == 1 || ! retrigger);
    const double from = lastPitch;
    lastPitch = keyPitch(note);

    setupOps(v, retrigger);

    for (int o = 1; o <= 6; ++o)
    {
        auto& op = v.op[(size_t) o];
        op.portaPitch = glide ? op.basePitch + (from - lastPitch) : op.basePitch;
    }

    if (retrigger)
    {
        if (current[Patch::OscKeySync] || ! wasActive)
        {
            for (int o = 1; o <= 6; ++o) v.op[(size_t) o].phase = 0;
            v.fb[0] = v.fb[1] = 0;
        }

        int r[4], l[4];
        for (int i = 0; i < 4; ++i) { r[i] = current[Patch::PR1 + i]; l[i] = current[Patch::PL1 + i]; }
        v.pitchEnv.start(r, l);
        lfo.keyDown();
    }
}

//
// An operator's settings for this note: pitch, level, envelope. `start` begins the envelopes;
// otherwise a sounding note takes up the patch as it now is.
//
void Engine::setupOps(Voice& v, bool start)
{
    const Patch& p = current;

    for (int o = 1; o <= 6; ++o)
    {
        auto& op = v.op[(size_t) o];

        int r[4], l[4];
        for (int i = 0; i < 4; ++i) { r[i] = p.op(o, (Patch::Op) (Patch::R1 + i)); l[i] = p.op(o, (Patch::Op) (Patch::L1 + i)); }

        int out = env::scaleOutputLevel(p.op(o, Patch::OutputLevel))
                + levelScale(v.note, p.op(o, Patch::BreakPoint), p.op(o, Patch::LeftDepth), p.op(o, Patch::RightDepth),
                             p.op(o, Patch::LeftCurve), p.op(o, Patch::RightCurve));
        out = std::min(out, 127) << 5;
        out += velocityScale(v.velocity, p.op(o, Patch::VelocitySens));
        out = std::max(out, 0);

        const int rs = rateScale(v.note, p.op(o, Patch::RateScaling));

        if (start) op.env.start(r, l, out, rs);
        else       op.env.update(r, l, out, rs);

        op.fixed     = p.op(o, Patch::Mode) != 0;
        op.basePitch = opPitch(v.note, op.fixed, p.op(o, Patch::Coarse), p.op(o, Patch::Fine), p.op(o, Patch::Detune));
        op.ampSens   = p.op(o, Patch::AmpModSens);
        if (! start && op.portaPitch == 0.0) op.portaPitch = op.basePitch;
    }

    if (! start)
    {
        int r[4], l[4];
        for (int i = 0; i < 4; ++i) { r[i] = p[Patch::PR1 + i]; l[i] = p[Patch::PL1 + i]; }
        v.pitchEnv.update(r, l);
    }
}

// ----------------------------------------------------------------------------- controls

void Engine::pitchBend(int value) { bend = std::clamp(value, -8192, 8191); }
void Engine::aftertouch(int value) { pressure = std::clamp(value, 0, 127); }

void Engine::controller(int cc, int value)
{
    value = std::clamp(value, 0, 127);
    switch (cc)
    {
        case 1:   wheel  = value; break;
        case 2:   breath = value; break;
        case 4:   foot   = value; break;
        case 64:  sustain(value >= 64); break;
        case 65:  portaOn = value >= 64; break;
        case 120: allSoundOff(); break;
        case 123: allNotesOff(); break;
        default:  break;
    }
}

void Engine::allNotesOff()
{
    heldCount = 0;
    pedal = false;
    for (auto& v : voices)
        if (v.active && v.keyDown) { v.sustained = false; releaseVoice(v); }
    for (auto& v : voices)
        if (v.sustained) { v.sustained = false; releaseVoice(v); }
}

void Engine::allSoundOff()
{
    heldCount = 0;
    pedal = false;
    for (auto& v : voices) { v.active = false; v.keyDown = false; v.sustained = false; }
}

int Engine::activeVoices() const
{
    int n = 0;
    for (const auto& v : voices) n += v.active ? 1 : 0;
    return n;
}

// ----------------------------------------------------------------------------- the block

//
// What changes slowly - the LFO, the controllers, bend and portamento - worked out once
// every 32 samples, as the DX7's own processor updates it a few hundred times a second
// rather than every sample.
//
void Engine::updateBlock()
{
    const Patch& p = current;
    lfo.next(lfoValue, lfoFade);

    // ---- pitch: the LFO, or a controller routed to pitch, whichever reaches further
    static const int pmsTab[8] = { 0, 10, 20, 33, 55, 92, 153, 255 };
    const int64_t senslfo = (int64_t) pmsTab[p[Patch::PitchModSens] & 7] * (lfoValue - (1 << 23));
    const uint32_t pmd = (uint32_t) ((p[Patch::LfoPitchDepth] * 165) >> 6) * (uint32_t) lfoFade;

    int ctrlPitch = 0, ctrlAmp = 0, ctrlEg = -1;
    auto route = [&] (int value, const Settings::Route& r)
    {
        const int t = reach(value, r);
        if (r.pitch)  ctrlPitch = std::max(ctrlPitch, t);
        if (r.amp)    ctrlAmp   = std::max(ctrlAmp, t);
        if (r.egBias) ctrlEg    = std::max(ctrlEg, t);
    };
    route(wheel, settings.wheel);
    route(foot, settings.foot);
    route(breath, settings.breath);
    route(pressure, settings.aftertouch);
    if (ctrlEg < 0) ctrlEg = 127;                     // no EG bias routed: none applied

    const int64_t pm1 = std::llabs(((int64_t) pmd * senslfo) >> 39);
    const int64_t pm2 = std::llabs(((int64_t) ctrlPitch * senslfo) >> 14);
    const double  lfoPitch = (double) std::max(pm1, pm2) * (senslfo < 0 ? -1.0 : 1.0);

    // ---- bend and tune
    double semis;
    if (settings.pitchBendStep > 0)
    {
        const double raw = bend / 8192.0 * settings.pitchBendRange;
        semis = std::trunc(raw / settings.pitchBendStep) * settings.pitchBendStep;
    }
    else
        semis = bend / 8192.0 * settings.pitchBendRange;

    pitchBase = semis / 12.0 * Octave + settings.masterTune / 1200.0 * Octave;
    pitchMod  = lfoPitch + pitchBase;

    // ---- amplitude: the LFO, a controller routed to amplitude, and EG bias
    const int64_t lfoAmp = (1 << 24) - lfoValue;
    const int64_t amd    = (p[Patch::LfoAmpDepth] * 165) >> 6;
    const int64_t a1 = (((amd * lfoFade) >> 8) * lfoAmp) >> 24;
    const int64_t a2 = ((int64_t) ctrlAmp * lfoAmp) >> 7;
    const int64_t a3 = (int64_t) (ctrlEg + 1) << 17;
    ampModDepth = (double) std::max<int64_t>((1 << 24) - a3, std::max(a1, a2));

    // ---- each voice's pitches, and each operator's share of the amplitude modulation
    static const double amsTab[4] = { 0.0, 4342338.0, 7171437.0, 16777216.0 };

    double portaStep = 0.0;
    if (settings.portaTime > 0 && portaOn)
    {
        const int idx = settings.portaTime * 127 / 99;
        const double semisPerSecond = (settings.portaGliss ? 1300.0 : 2100.0) * std::pow(2.0, -0.062 * idx);
        portaStep = semisPerSecond / NativeRate * Block * (Octave / 12.0);
    }

    for (auto& v : voices)
    {
        if (! v.active) continue;

        const double env = v.pitchEnv.next();

        for (int o = 1; o <= 6; ++o)
        {
            auto& op = v.op[(size_t) o];

            if (op.fixed)
                op.inc = phaseIncrement(op.basePitch + pitchBase);
            else
            {
                if (op.portaPitch != op.basePitch)
                {
                    if (portaStep <= 0.0) op.portaPitch = op.basePitch;
                    else if (op.portaPitch < op.basePitch) op.portaPitch = std::min(op.portaPitch + portaStep, op.basePitch);
                    else                                   op.portaPitch = std::max(op.portaPitch - portaStep, op.basePitch);
                }

                double pitch = op.portaPitch;
                if (settings.portaGliss && op.portaPitch != op.basePitch)
                {
                    // glissando: the slide in semitone steps
                    const double step = Octave / 12.0;
                    pitch = op.basePitch + std::round((op.portaPitch - op.basePitch) / step) * step;
                }
                op.inc = phaseIncrement(pitch + env + pitchMod);
            }

            // MSFA's model of amplitude modulation: a share of the operator's level taken
            // away, growing exponentially with depth. Approximate - MSFA says as much.
            op.ampScale = 1.0;
            if (op.ampSens != 0)
            {
                const double sens = ampModDepth * amsTab[op.ampSens] / Octave;
                const double pt = (double) (uint32_t) std::exp(sens / 262144.0 * 0.07 + 12.2);
                op.ampScale = 1.0 - pt * 16.0 / 268435456.0;
            }
        }
    }
}

// --------------------------------------------------------------------------- the voices

int32_t Engine::voiceSample(Voice& v)
{
    const Algorithm& alg = algorithms[current[Patch::Algorithm] & 31];
    const int32_t com = ops::carrierAttenuation(carrierCount(alg));
    const int fbAmount = current[Patch::Feedback] & 7;

    int32_t sum = 0;
    bool sounding = false;

    for (int o = 6; o >= 1; --o)
    {
        auto& op = v.op[(size_t) o];
        const double level = op.env.next() * op.ampScale;
        const bool carrier = (alg.carriers >> o) & 1;

        if (carrier && op.env.active()) sounding = true;

        if (! ((settings.opMask >> (o - 1)) & 1))
        {
            op.out = 0;
            op.phase += op.inc;
            if (o == alg.fbFrom) { v.fb[0] = v.fb[1]; v.fb[1] = 0; }
            continue;
        }

        int32_t atten = 16384 - (int32_t) (level / 16384.0);
        if (carrier) atten += com;

        int32_t mod = 0;
        for (int m = o + 1; m <= 6; ++m)
            if ((alg.mods[o] >> m) & 1) mod += v.op[(size_t) m].out;

        if (o == alg.fbTo && fbAmount > 0)
            mod += (v.fb[0] + v.fb[1]) >> (9 - fbAmount);

        op.out = ops::sample(op.phase + ((uint32_t) mod << 20), atten);
        op.phase += op.inc;

        if (o == alg.fbFrom) { v.fb[0] = v.fb[1]; v.fb[1] = op.out; }
        if (carrier) sum += op.out;
    }

    if (! sounding && ! v.keyDown && ! v.sustained) v.active = false;
    return sum;
}

double Engine::sampleNative()
{
    if (blockLeft <= 0) { updateBlock(); blockLeft = Block; }
    --blockLeft;

    // Each voice goes through the DAC on its own - the DX7 converts the voices one after
    // another and sums them as voltages - so each keeps its own quantisation.
    double mix = 0.0;
    for (auto& v : voices)
        if (v.active) mix += ops::dac(voiceSample(v));

    return mix * (VoiceGain / 16384.0);
}

float Engine::renderNative()
{
    return (float) sampleNative();
}

void Engine::render(float* out, int n)
{
    resampler.process(out, n, [this]
    {
        const double x = sampleNative();
        const double y = lpB[0] * x + lpB[1] * lpX[0] + lpB[2] * lpX[1] - lpA[1] * lpY[0] - lpA[2] * lpY[1];
        lpX[1] = lpX[0]; lpX[0] = x;
        lpY[1] = lpY[0]; lpY[0] = y;
        return (float) y;
    });
}

} // namespace dx7
