// Mz7 - Created by Simon Moscrop
// Free software under the GNU AGPL v3; see LICENSE.

#include "Banks.h"

#include "Algorithms.h"

#include <array>
#include <functional>
#include <vector>

//
// THE FOUR CARTRIDGES
//
// Programmed for Mz7, voice by voice, with the classic FM recipes as the starting points -
// a 1:1 pair for an electric piano's body and a 1:14 pair for its tine, a feedback operator's
// sawtooth for brass and strings, inharmonic ratios for bells - and nothing taken from
// anyone's cartridge.
//
// A few numbers to read them by. A modulator's OUTPUT LEVEL sets how bright it makes its
// carrier: at 99 the modulation index is about 4 pi, and every 8 below halves it - 91 is about
// 2 pi, 83 about pi, 75 about pi/2, 67 about a quarter of pi. Envelope LEVELS move 0.75 dB a
// step. VELOCITY SENSITIVITY 7 makes a key struck at 64 about 10 dB softer than one at 100,
// and one at 127 about 5 dB louder; on a modulator that is brightness rather than volume.
// RATIOS: coarse 0 is 0.5; fine adds that percentage, so 3 and 17 is 3.51.
//
// Envelope rates: 99 is instant, 70 a few hundredths of a second, 50 a few tenths, 30 a few
// seconds, 20 and below many seconds. Rate scaling makes higher notes faster.
//

namespace
{
using dx7::Cartridge;
using V = dx7::VoiceBuilder;

// ===================================================================== BANK 1: CLASSIC

Cartridge classic()
{
    Cartridge c;
    int i = 0;
    auto put = [&] (const dx7::Patch& p) { c.voices[(size_t) i++] = p; };

    { V v("E.PIANO 1", 5, 6);          // the FM electric piano: body pair, tine pair, bark pair
      v.op(1).ratio(1).level(98).vel(2).rate(3).eg(96, 25, 25, 67, 99, 75, 0, 0);
      v.op(2).ratio(1).level(62).vel(6).rate(3).eg(95, 50, 35, 78, 99, 75, 0, 0).scale(39, 0, 15, 0, 0);
      v.op(3).ratio(1, 0, 8).level(94).vel(2).rate(3).eg(95, 29, 20, 50, 99, 95, 0, 0);
      v.op(4).ratio(14).level(58).vel(7).rate(3).eg(98, 62, 40, 72, 99, 68, 0, 0).scale(39, 0, 30, 0, 1);
      v.op(5).ratio(1, 0, 6).level(93).vel(1).rate(3).eg(95, 25, 25, 67, 99, 75, 0, 0);
      v.op(6).ratio(1).level(70).vel(6).rate(3).eg(95, 50, 35, 78, 99, 75, 0, 0);
      v.lfo(34, 33, 0, 0, 4, 3);
      put(v); }

    { V v("BASS 1", 1, 5);             // round, plucked, with a woody attack
      v.op(1).ratio(0).level(99).vel(1).rate(2).eg(99, 36, 28, 72, 99, 86, 70, 0);
      v.op(2).ratio(0).level(80).vel(4).rate(2).eg(99, 58, 40, 72, 99, 64, 45, 0);
      v.op(3).ratio(0, 0, 9).level(90).vel(1).rate(2).eg(99, 36, 28, 72, 99, 86, 70, 0);
      v.op(4).ratio(1).level(70).vel(4).rate(2).eg(99, 70, 45, 72, 99, 45, 0, 0);
      v.op(5).ratio(3).level(52).vel(5).rate(2).eg(99, 80, 50, 72, 99, 0, 0, 0);
      v.op(6).ratio(1).level(64).vel(3).rate(2).eg(99, 76, 50, 72, 99, 40, 0, 0);
      put(v); }

    { V v("BRASS 1", 22, 7);           // a feedback sawtooth across three carriers, swelling
      v.op(1).ratio(1).level(97).vel(1).eg(70, 40, 30, 62, 99, 93, 90, 0);
      v.op(2).ratio(1).level(79).vel(3).eg(55, 40, 30, 62, 99, 90, 86, 0).scale(48, 0, 20, 0, 0);
      v.op(3).ratio(1, 0, 6).level(97).vel(1).eg(70, 40, 30, 62, 99, 93, 90, 0);
      v.op(4).ratio(1, 0, 8).level(96).vel(1).eg(70, 40, 30, 62, 99, 93, 90, 0);
      v.op(5).ratio(1, 0, 9).level(94).vel(1).eg(70, 40, 30, 62, 99, 93, 90, 0);
      v.op(6).ratio(1).level(76).vel(3).eg(50, 40, 30, 62, 99, 92, 88, 0).scale(48, 0, 25, 0, 0);
      v.lfo(35, 40, 5, 0, 0, 3);
      v.pitchEg(84, 95, 95, 60, 46, 50, 50, 50);
      put(v); }

    { V v("STRINGS 1", 1, 6);          // two sawtooth stacks, slow and detuned
      v.op(1).ratio(1, 0, 5).level(96).eg(45, 30, 25, 44, 99, 95, 92, 0);
      v.op(2).ratio(1).level(72).eg(50, 35, 25, 44, 99, 92, 90, 0).scale(50, 0, 20, 0, 0);
      v.op(3).ratio(1, 0, 9).level(96).eg(45, 30, 25, 44, 99, 95, 92, 0);
      v.op(4).ratio(1).level(66).eg(50, 35, 25, 44, 99, 92, 90, 0);
      v.op(5).ratio(1).level(60).eg(50, 35, 25, 44, 99, 92, 90, 0);
      v.op(6).ratio(1).level(74).eg(50, 35, 25, 44, 99, 92, 90, 0).scale(50, 0, 25, 0, 0);
      v.lfo(30, 45, 8, 0, 0, 3);
      put(v); }

    { V v("ORGAN 1", 32, 0);           // drawbars: 16', 8', 5 1/3', 4', 2 2/3', 2'
      v.op(1).ratio(0).level(93).eg(99, 99, 99, 72, 99, 99, 99, 0);
      v.op(2).ratio(1).level(99).eg(99, 99, 99, 72, 99, 99, 99, 0);
      v.op(3).ratio(1, 50).level(86).eg(99, 99, 99, 72, 99, 99, 99, 0);
      v.op(4).ratio(2).level(90).eg(99, 99, 99, 72, 99, 99, 99, 0);
      v.op(5).ratio(3).level(82).eg(99, 99, 99, 72, 99, 99, 99, 0);
      v.op(6).ratio(4).level(84).eg(99, 99, 99, 72, 99, 99, 99, 0);
      v.lfo(36, 0, 6, 0, 4, 4);
      put(v); }

    { V v("TUB BELLS", 5, 0);          // a 1 : 3.5 pair is the bell; the others its overtones
      v.op(1).ratio(1).level(99).vel(2).rate(2).eg(98, 26, 22, 30, 99, 75, 0, 0);
      v.op(2).ratio(3, 17).level(76).vel(4).rate(2).eg(98, 30, 25, 35, 99, 70, 0, 0);
      v.op(3).ratio(1, 0, 10).level(94).vel(2).rate(2).eg(98, 26, 22, 30, 99, 75, 0, 0);
      v.op(4).ratio(3, 17, 4).level(72).vel(4).rate(2).eg(98, 30, 25, 35, 99, 70, 0, 0);
      v.op(5).ratio(2, 0, 8).level(80).vel(2).rate(2).eg(99, 40, 30, 40, 99, 60, 0, 0);
      v.op(6).ratio(7, 30).level(60).vel(5).rate(2).eg(99, 60, 40, 50, 99, 30, 0, 0);
      put(v); }

    { V v("MARIMBA", 7, 0);            // a mallet's knock, then the bar
      v.op(1).ratio(1).level(99).vel(2).rate(4).eg(99, 42, 30, 60, 99, 40, 0, 0);
      v.op(2).ratio(4).level(66).vel(6).rate(4).eg(99, 80, 50, 70, 99, 0, 0, 0);
      v.op(3).ratio(4, 0, 8).level(78).vel(3).rate(5).eg(99, 62, 40, 65, 99, 0, 0, 0);
      v.op(4).ratio(1).level(55).vel(6).rate(4).eg(99, 80, 50, 70, 99, 0, 0, 0);
      v.op(5).ratio(10).level(50).vel(6).rate(4).eg(99, 85, 50, 70, 99, 0, 0, 0);
      v.op(6).ratio(3).level(45).eg(99, 85, 50, 70, 99, 0, 0, 0);
      put(v); }

    { V v("CLAV", 3, 6);               // a bright pluck that holds while the key does
      v.op(1).ratio(1).level(99).vel(2).rate(3).eg(99, 50, 30, 80, 99, 88, 70, 0);
      v.op(2).ratio(3).level(80).vel(5).rate(3).eg(99, 60, 35, 80, 99, 72, 55, 0).scale(46, 0, 22, 0, 0);
      v.op(3).ratio(1).level(72).vel(5).rate(3).eg(99, 70, 40, 80, 99, 50, 40, 0);
      v.op(4).ratio(2, 0, 8).level(92).vel(2).rate(3).eg(99, 50, 30, 80, 99, 88, 70, 0);
      v.op(5).ratio(5).level(66).vel(5).rate(3).eg(99, 65, 40, 80, 99, 60, 40, 0);
      v.op(6).ratio(1).level(72).vel(4).rate(3).eg(99, 70, 40, 80, 99, 50, 30, 0);
      put(v); }

    { V v("HARPSICH", 5, 4);           // plucked quills, two registers
      v.op(1).ratio(1).level(99).vel(1).rate(3).eg(99, 38, 28, 68, 99, 70, 0, 0);
      v.op(2).ratio(5).level(74).vel(3).rate(3).eg(99, 52, 35, 68, 99, 60, 0, 0).scale(48, 0, 20, 0, 0);
      v.op(3).ratio(2, 0, 9).level(90).vel(1).rate(3).eg(99, 38, 28, 68, 99, 70, 0, 0);
      v.op(4).ratio(7).level(70).vel(3).rate(3).eg(99, 55, 35, 68, 99, 55, 0, 0).scale(48, 0, 25, 0, 0);
      v.op(5).ratio(1, 0, 5).level(92).vel(1).rate(3).eg(99, 38, 28, 68, 99, 70, 0, 0);
      v.op(6).ratio(3).level(78).vel(3).rate(3).eg(99, 60, 35, 68, 99, 50, 0, 0);
      put(v); }

    { V v("FLUTE 1", 2, 7);            // a soft tone, a little breath from the feedback
      v.op(1).ratio(1).level(97).vel(1).eg(62, 40, 30, 60, 99, 96, 93, 0);
      v.op(2).ratio(1).level(58).vel(2).eg(70, 45, 30, 60, 99, 90, 88, 0);
      v.op(3).ratio(2).level(70).eg(62, 40, 30, 60, 99, 92, 88, 0);
      v.op(4).ratio(9, 50).level(42).eg(80, 60, 40, 60, 99, 70, 60, 0);
      v.op(5).ratio(1).level(30).eg(80, 60, 40, 60, 99, 70, 60, 0);
      v.op(6).ratio(1).level(0);
      v.lfo(32, 55, 10, 0, 4, 4);
      v.pitchEg(90, 95, 95, 60, 48, 50, 50, 50);
      put(v); }

    { V v("SYN LEAD 1", 18, 7);        // a hard-edged lead: a sawtooth and two more for bite
      v.op(1).ratio(1).level(99).vel(1).eg(99, 50, 30, 70, 99, 95, 92, 0);
      v.op(2).ratio(1, 0, 9).level(76).eg(99, 50, 30, 70, 99, 92, 90, 0);
      v.op(3).ratio(1).level(84).vel(2).eg(99, 50, 30, 70, 99, 90, 88, 0);
      v.op(4).ratio(2, 0, 5).level(70).eg(99, 50, 30, 70, 99, 90, 85, 0);
      v.op(5).ratio(1).level(60).eg(99, 50, 30, 70, 99, 90, 85, 0);
      v.op(6).ratio(1).level(70).eg(99, 50, 30, 70, 99, 90, 85, 0);
      v.lfo(36, 50, 9, 0, 0, 4);
      put(v); }

    { V v("PIANO 1", 5, 0);            // a struck string: a hammer, then decaying overtones
      v.op(1).ratio(1).level(99).vel(2).rate(3).eg(98, 28, 24, 55, 99, 82, 0, 0);
      v.op(2).ratio(1).level(76).vel(5).rate(3).eg(98, 44, 33, 60, 99, 70, 0, 0).scale(48, 0, 24, 0, 0);
      v.op(3).ratio(1, 0, 9).level(95).vel(2).rate(3).eg(98, 30, 24, 55, 99, 80, 0, 0);
      v.op(4).ratio(3).level(68).vel(5).rate(3).eg(98, 50, 35, 60, 99, 60, 0, 0).scale(48, 0, 30, 0, 0);
      v.op(5).ratio(1, 0, 5).level(86).vel(2).rate(3).eg(98, 32, 25, 55, 99, 75, 0, 0);
      v.op(6).ratio(5).level(62).vel(6).rate(3).eg(99, 75, 40, 70, 99, 0, 0, 0);
      put(v); }

    { V v("GUITAR 1", 3, 5);           // nylon strings: a soft pluck and a long fade
      v.op(1).ratio(1).level(99).vel(2).rate(3).eg(99, 32, 26, 60, 99, 78, 0, 0);
      v.op(2).ratio(1).level(72).vel(5).rate(3).eg(99, 50, 35, 60, 99, 60, 0, 0).scale(46, 0, 20, 0, 0);
      v.op(3).ratio(3).level(62).vel(5).rate(3).eg(99, 65, 40, 60, 99, 40, 0, 0);
      v.op(4).ratio(1, 0, 8).level(90).vel(2).rate(3).eg(99, 34, 26, 60, 99, 75, 0, 0);
      v.op(5).ratio(2).level(66).vel(4).rate(3).eg(99, 55, 35, 60, 99, 50, 0, 0);
      v.op(6).ratio(1).level(64).vel(4).rate(3).eg(99, 60, 40, 60, 99, 40, 0, 0);
      put(v); }

    { V v("SLAP BASS", 16, 6);         // a thumb slap: the attack bright, then the string
      v.op(1).ratio(0).level(99).vel(2).rate(2).eg(99, 40, 30, 72, 99, 85, 60, 0);
      v.op(2).ratio(0).level(78).vel(5).rate(2).eg(99, 60, 40, 72, 99, 65, 40, 0);
      v.op(3).ratio(1).level(76).vel(6).rate(2).eg(99, 75, 45, 72, 99, 45, 0, 0);
      v.op(4).ratio(5).level(60).vel(6).rate(2).eg(99, 85, 50, 72, 99, 0, 0, 0);
      v.op(5).ratio(3).level(66).vel(6).rate(2).eg(99, 80, 45, 72, 99, 20, 0, 0);
      v.op(6).ratio(1).level(68).vel(4).rate(2).eg(99, 70, 45, 72, 99, 40, 0, 0);
      put(v); }

    { V v("KALIMBA", 5, 0);            // a plucked tine on a wooden box
      v.op(1).ratio(1).level(99).vel(2).rate(4).eg(99, 40, 30, 55, 99, 30, 0, 0);
      v.op(2).ratio(6).level(64).vel(5).rate(4).eg(99, 70, 45, 60, 99, 0, 0, 0);
      v.op(3).ratio(5, 70).level(70).vel(3).rate(5).eg(99, 60, 40, 55, 99, 0, 0, 0);
      v.op(4).ratio(1).level(50).vel(5).rate(4).eg(99, 80, 50, 60, 99, 0, 0, 0);
      v.op(5).ratio(1, 0, 9).level(80).vel(2).rate(4).eg(99, 45, 32, 55, 99, 20, 0, 0);
      v.op(6).ratio(3).level(55).vel(4).rate(4).eg(99, 75, 45, 60, 99, 0, 0, 0);
      put(v); }

    { V v("VIBES", 5, 0);              // vibraphone, with the motor's tremolo
      v.op(1).ratio(1).level(99).vel(2).rate(3).ams(2).eg(99, 30, 24, 40, 99, 70, 0, 0);
      v.op(2).ratio(4).level(64).vel(5).rate(3).eg(99, 60, 40, 50, 99, 20, 0, 0);
      v.op(3).ratio(1, 0, 9).level(94).vel(2).rate(3).ams(2).eg(99, 32, 24, 40, 99, 70, 0, 0);
      v.op(4).ratio(1).level(52).vel(5).rate(3).eg(99, 60, 40, 50, 99, 30, 0, 0);
      v.op(5).ratio(4).level(72).vel(3).rate(4).eg(99, 55, 35, 45, 99, 0, 0, 0);
      v.op(6).ratio(10).level(48).vel(5).rate(4).eg(99, 80, 50, 50, 99, 0, 0, 0);
      v.lfo(40, 0, 0, 28, 4, 0);
      put(v); }

    { V v("GLOCKEN", 5, 0);            // glockenspiel: high, hard and ringing
      v.op(1).ratio(2).level(99).vel(2).rate(4).eg(99, 32, 26, 40, 99, 60, 0, 0);
      v.op(2).ratio(5, 41).level(66).vel(5).rate(4).eg(99, 50, 35, 45, 99, 40, 0, 0);
      v.op(3).ratio(5, 41, 8).level(80).vel(2).rate(4).eg(99, 40, 30, 40, 99, 40, 0, 0);
      v.op(4).ratio(1).level(55).vel(5).rate(4).eg(99, 70, 45, 45, 99, 0, 0, 0);
      v.op(5).ratio(2, 0, 9).level(90).vel(2).rate(4).eg(99, 34, 26, 40, 99, 60, 0, 0);
      v.op(6).ratio(11).level(50).vel(5).rate(4).eg(99, 80, 50, 45, 99, 0, 0, 0);
      v.transpose(12);
      put(v); }

    { V v("XYLOPHONE", 7, 0);          // hard mallets on rosewood: short and woody
      v.op(1).ratio(1).level(99).vel(2).rate(5).eg(99, 55, 40, 65, 99, 0, 0, 0);
      v.op(2).ratio(3).level(70).vel(5).rate(5).eg(99, 80, 50, 70, 99, 0, 0, 0);
      v.op(3).ratio(3, 0, 8).level(84).vel(2).rate(5).eg(99, 62, 40, 65, 99, 0, 0, 0);
      v.op(4).ratio(7).level(58).vel(5).rate(5).eg(99, 85, 50, 70, 99, 0, 0, 0);
      v.op(5).ratio(1).level(55).vel(5).rate(5).eg(99, 85, 50, 70, 99, 0, 0, 0);
      v.op(6).ratio(2).level(50).eg(99, 85, 50, 70, 99, 0, 0, 0);
      v.transpose(12);
      put(v); }

    { V v("STEEL DRUM", 5, 0);         // a pan: the note, its octave, and a bent overtone
      v.op(1).ratio(1).level(99).vel(2).rate(3).eg(99, 40, 30, 55, 99, 50, 0, 0);
      v.op(2).ratio(2).level(70).vel(5).rate(3).eg(99, 50, 35, 55, 99, 40, 0, 0);
      v.op(3).ratio(2, 0, 9).level(84).vel(2).rate(3).eg(99, 45, 32, 55, 99, 40, 0, 0);
      v.op(4).ratio(3, 50).level(58).vel(5).rate(3).eg(99, 65, 40, 55, 99, 0, 0, 0);
      v.op(5).ratio(3, 0, 5).level(66).vel(3).rate(3).eg(99, 55, 35, 55, 99, 20, 0, 0);
      v.op(6).ratio(1).level(50).vel(5).rate(3).eg(99, 75, 45, 55, 99, 0, 0, 0);
      v.pitchEg(99, 70, 95, 60, 54, 50, 50, 50);
      put(v); }

    { V v("OBOE", 1, 5);               // a double reed: narrow, nasal, with vibrato
      v.op(1).ratio(1).level(98).vel(1).eg(65, 40, 30, 60, 99, 95, 92, 0);
      v.op(2).ratio(4).level(66).vel(3).eg(70, 45, 30, 60, 99, 90, 88, 0).scale(50, 0, 25, 0, 0);
      v.op(3).ratio(2).level(84).vel(1).eg(65, 40, 30, 60, 99, 95, 92, 0);
      v.op(4).ratio(1).level(74).vel(3).eg(70, 45, 30, 60, 99, 90, 88, 0);
      v.op(5).ratio(3).level(56).eg(70, 45, 30, 60, 99, 90, 88, 0);
      v.op(6).ratio(1).level(60).eg(70, 45, 30, 60, 99, 90, 88, 0);
      v.lfo(35, 50, 8, 0, 4, 4);
      put(v); }

    { V v("CHOIR", 2, 4);              // "aah": the vowel's formants as quiet carriers
      v.op(1).ratio(1, 0, 6).level(96).eg(40, 30, 25, 45, 99, 95, 93, 0);
      v.op(2).ratio(1).level(64).eg(45, 30, 25, 45, 99, 92, 90, 0);
      v.op(3).ratio(1, 0, 9).level(94).eg(40, 30, 25, 45, 99, 95, 93, 0);
      v.op(4).ratio(1).level(48).eg(45, 30, 25, 45, 99, 92, 90, 0);
      v.op(5).ratio(6).level(48).eg(45, 30, 25, 45, 99, 92, 90, 0);
      v.op(6).ratio(1).level(55).eg(45, 30, 25, 45, 99, 92, 90, 0);
      v.lfo(33, 40, 7, 0, 4, 3);
      put(v); }

    { V v("WARM PAD", 5, 4);           // three soft pairs, spread, slow in and out
      v.op(1).ratio(1, 0, 4).level(94).eg(38, 28, 25, 36, 99, 95, 92, 0);
      v.op(2).ratio(1).level(62).eg(40, 30, 25, 36, 99, 92, 90, 0);
      v.op(3).ratio(1, 0, 10).level(94).eg(38, 28, 25, 36, 99, 95, 92, 0);
      v.op(4).ratio(2).level(56).eg(40, 30, 25, 36, 99, 92, 90, 0);
      v.op(5).ratio(0, 0, 7).level(90).eg(38, 28, 25, 36, 99, 95, 92, 0);
      v.op(6).ratio(1).level(58).eg(40, 30, 25, 36, 99, 92, 90, 0);
      v.lfo(28, 50, 5, 0, 0, 3);
      put(v); }

    { V v("BELL PAD", 5, 2);           // a soft pad with a bell struck on top of it
      v.op(1).ratio(1, 0, 5).level(94).eg(40, 30, 25, 38, 99, 95, 92, 0);
      v.op(2).ratio(1).level(64).eg(42, 30, 25, 38, 99, 92, 90, 0);
      v.op(3).ratio(1, 0, 9).level(92).eg(40, 30, 25, 38, 99, 95, 92, 0);
      v.op(4).ratio(1).level(60).eg(42, 30, 25, 38, 99, 92, 90, 0);
      v.op(5).ratio(4).level(88).vel(2).rate(2).eg(99, 30, 25, 35, 99, 60, 0, 0);
      v.op(6).ratio(9, 50).level(68).vel(4).rate(2).eg(99, 40, 30, 35, 99, 50, 0, 0);
      v.lfo(30, 40, 4, 0, 0, 3);
      put(v); }

    { V v("SYN BRASS", 16, 7);         // the synthesizer kind: a filter-like sweep on attack
      v.op(1).ratio(1).level(99).vel(1).eg(80, 45, 30, 60, 99, 94, 90, 0);
      v.op(2).ratio(1).level(76).vel(3).eg(60, 45, 30, 60, 99, 86, 82, 0);
      v.op(3).ratio(1, 0, 10).level(78).vel(3).eg(60, 45, 30, 60, 99, 86, 82, 0);
      v.op(4).ratio(1).level(60).eg(70, 50, 30, 60, 99, 90, 85, 0);
      v.op(5).ratio(1, 0, 4).level(76).vel(3).eg(60, 45, 30, 60, 99, 86, 82, 0);
      v.op(6).ratio(1).level(70).eg(70, 50, 30, 60, 99, 90, 85, 0);
      v.lfo(35, 45, 5, 0, 0, 3);
      put(v); }

    { V v("PERC ORGAN", 32, 2);        // drawbars with a percussive second harmonic and a click
      v.op(1).ratio(0).level(90).eg(99, 99, 99, 75, 99, 99, 99, 0);
      v.op(2).ratio(1).level(99).eg(99, 99, 99, 75, 99, 99, 99, 0);
      v.op(3).ratio(2).level(88).eg(99, 99, 99, 75, 99, 99, 99, 0);
      v.op(4).ratio(3).level(92).vel(1).eg(99, 45, 99, 75, 99, 0, 0, 0);
      v.op(5).ratio(4).level(80).eg(99, 99, 99, 75, 99, 99, 99, 0);
      v.op(6).ratio(10).level(66).vel(2).eg(99, 85, 99, 75, 99, 0, 0, 0);
      v.lfo(38, 0, 7, 0, 4, 4);
      put(v); }

    { V v("SITAR", 3, 7);              // a buzzing bridge: bright, bending into the note
      v.op(1).ratio(1).level(99).vel(1).rate(2).eg(99, 30, 25, 50, 99, 80, 0, 0);
      v.op(2).ratio(1).level(84).vel(3).rate(2).eg(99, 32, 25, 50, 99, 78, 0, 0);
      v.op(3).ratio(3).level(66).vel(3).rate(2).eg(99, 40, 30, 50, 99, 60, 0, 0);
      v.op(4).ratio(1, 0, 9).level(90).rate(2).eg(99, 32, 25, 50, 99, 78, 0, 0);
      v.op(5).ratio(2).level(80).rate(2).eg(99, 32, 25, 50, 99, 78, 0, 0);
      v.op(6).ratio(5).level(74).rate(2).eg(99, 32, 25, 50, 99, 70, 0, 0);
      v.pitchEg(99, 60, 95, 60, 53, 50, 50, 50);
      put(v); }

    { V v("LOG DRUM", 5, 0);           // a slit drum, the pitch falling as it sounds
      v.op(1).ratio(1).level(99).vel(3).rate(3).eg(99, 48, 35, 55, 99, 0, 0, 0);
      v.op(2).ratio(2, 70).level(62).vel(5).rate(3).eg(99, 70, 45, 60, 99, 0, 0, 0);
      v.op(3).ratio(1, 50).level(80).vel(3).rate(3).eg(99, 55, 40, 55, 99, 0, 0, 0);
      v.op(4).ratio(1).level(54).vel(5).rate(3).eg(99, 75, 45, 60, 99, 0, 0, 0);
      v.op(5).ratio(4, 20).level(50).vel(3).rate(4).eg(99, 70, 45, 55, 99, 0, 0, 0);
      v.op(6).ratio(1).level(0);
      v.pitchEg(99, 55, 95, 60, 56, 49, 50, 50);
      put(v); }

    { V v("HARP", 5, 0);               // plucked gut strings, long and clear
      v.op(1).ratio(1).level(99).vel(2).rate(3).eg(99, 28, 24, 45, 99, 80, 0, 0);
      v.op(2).ratio(1).level(62).vel(5).rate(3).eg(99, 50, 35, 50, 99, 50, 0, 0);
      v.op(3).ratio(2, 0, 8).level(82).vel(2).rate(3).eg(99, 34, 26, 45, 99, 65, 0, 0);
      v.op(4).ratio(3).level(52).vel(5).rate(3).eg(99, 60, 40, 50, 99, 30, 0, 0);
      v.op(5).ratio(1, 0, 6).level(90).vel(2).rate(3).eg(99, 30, 24, 45, 99, 78, 0, 0);
      v.op(6).ratio(5).level(48).vel(6).rate(3).eg(99, 75, 45, 50, 99, 0, 0, 0);
      put(v); }

    { V v("CELESTA", 5, 0);            // little hammers on steel plates, over a resonator
      v.op(1).ratio(1).level(99).vel(2).rate(4).eg(99, 36, 28, 45, 99, 50, 0, 0);
      v.op(2).ratio(4).level(66).vel(5).rate(4).eg(99, 55, 38, 50, 99, 30, 0, 0);
      v.op(3).ratio(4, 0, 9).level(76).vel(2).rate(4).eg(99, 45, 32, 45, 99, 30, 0, 0);
      v.op(4).ratio(1).level(50).vel(5).rate(4).eg(99, 70, 45, 50, 99, 0, 0, 0);
      v.op(5).ratio(2).level(84).vel(2).rate(4).eg(99, 40, 30, 45, 99, 45, 0, 0);
      v.op(6).ratio(14).level(44).vel(6).rate(4).eg(99, 85, 50, 50, 99, 0, 0, 0);
      v.transpose(12);
      put(v); }

    { V v("ACCORDION", 2, 5);          // reeds in pairs, beating against each other
      v.op(1).ratio(1, 0, 3).level(97).eg(80, 50, 30, 70, 99, 96, 94, 0);
      v.op(2).ratio(1).level(76).vel(2).eg(80, 50, 30, 70, 99, 94, 92, 0);
      v.op(3).ratio(1, 0, 11).level(96).eg(80, 50, 30, 70, 99, 96, 94, 0);
      v.op(4).ratio(3).level(66).eg(80, 50, 30, 70, 99, 94, 92, 0);
      v.op(5).ratio(2).level(60).eg(80, 50, 30, 70, 99, 94, 92, 0);
      v.op(6).ratio(1).level(66).eg(80, 50, 30, 70, 99, 94, 92, 0);
      v.lfo(40, 0, 0, 12, 4, 0);
      v.op(1).ams(1); v.op(3).ams(1);
      put(v); }

    { V v("TRUMPET", 18, 7);           // one horn: a bright bell that opens with the breath
      v.op(1).ratio(1).level(99).vel(1).eg(72, 45, 30, 65, 99, 94, 90, 0);
      v.op(2).ratio(1).level(74).vel(3).eg(60, 45, 30, 65, 99, 88, 84, 0);
      v.op(3).ratio(2).level(66).vel(3).eg(60, 45, 30, 65, 99, 86, 82, 0);
      v.op(4).ratio(1).level(76).vel(3).eg(55, 45, 30, 65, 99, 88, 84, 0);
      v.op(5).ratio(1).level(66).eg(60, 45, 30, 65, 99, 88, 84, 0);
      v.op(6).ratio(1).level(66).eg(60, 45, 30, 65, 99, 88, 84, 0);
      v.lfo(36, 55, 7, 0, 4, 4);
      v.pitchEg(80, 95, 95, 60, 45, 50, 50, 50);
      put(v); }

    { V v("SPACE FX", 1, 7);           // a slow sweep, rising, with a wandering LFO
      v.op(1).ratio(1).level(95).eg(30, 25, 20, 30, 99, 90, 85, 0);
      v.op(2).ratio(3, 50).level(78).ams(2).eg(25, 20, 20, 30, 99, 85, 80, 0);
      v.op(3).ratio(1, 0, 10).level(93).eg(30, 25, 20, 30, 99, 90, 85, 0);
      v.op(4).ratio(2, 30).level(70).ams(3).eg(20, 20, 20, 30, 99, 80, 75, 0);
      v.op(5).ratio(1).level(66).eg(20, 20, 20, 30, 99, 80, 75, 0);
      v.op(6).ratio(7).level(70).eg(20, 20, 20, 30, 99, 80, 75, 0);
      v.lfo(20, 30, 20, 40, 5, 5);
      v.pitchEg(20, 30, 40, 40, 40, 55, 50, 45);
      put(v); }

    return c;
}

// ============================================================== BANK 2: KEYS & MALLETS

Cartridge keys()
{
    Cartridge c;
    int i = 0;
    auto put = [&] (const dx7::Patch& p) { c.voices[(size_t) i++] = p; };

    // an electric piano in three pairs, with the tine and bark as arguments
    auto ep = [&] (const char* name, int tineRatio, int tineLevel, int barkLevel, int bodyLevel,
                   int decay, int detune, int tremolo)
    {
        V v(name, 5, 6);
        v.op(1).ratio(1).level(98).vel(2).rate(3).ams(tremolo ? 1 : 0).eg(96, decay, 25, 67, 99, 75, 0, 0);
        v.op(2).ratio(1).level(bodyLevel).vel(6).rate(3).eg(95, 50, 35, 78, 99, 75, 0, 0).scale(39, 0, 15, 0, 0);
        v.op(3).ratio(1, 0, 7 + detune).level(94).vel(2).rate(3).ams(tremolo ? 1 : 0).eg(95, decay + 4, 20, 50, 99, 95, 0, 0);
        v.op(4).ratio(tineRatio).level(tineLevel).vel(7).rate(3).eg(98, 62, 40, 72, 99, 68, 0, 0).scale(39, 0, 30, 0, 1);
        v.op(5).ratio(1, 0, 7 - detune).level(93).vel(1).rate(3).ams(tremolo ? 1 : 0).eg(95, decay, 25, 67, 99, 75, 0, 0);
        v.op(6).ratio(1).level(barkLevel).vel(6).rate(3).eg(95, 50, 35, 78, 99, 75, 0, 0);
        v.lfo(38, 0, 0, tremolo, 4, 0);
        return v;
    };

    put(ep("E.PIANO 2", 14, 64, 74, 66, 25, 1, 0));     // brighter, more bark
    put(ep("SOFT EP", 14, 50, 60, 56, 22, 2, 0));       // a ballad, played gently
    put(ep("TREM EP", 14, 58, 68, 62, 25, 1, 30));      // with the amplifier's tremolo
    put(ep("GLASS EP", 9, 70, 62, 60, 24, 1, 0));       // a glassier tine
    put(ep("DYNO EP", 14, 68, 80, 70, 27, 2, 18));      // hard and chorused, with tremolo

    { V v("WURLY", 3, 5);              // a reed piano: hollow, with a growl when struck hard
      v.op(1).ratio(1).level(99).vel(2).rate(3).eg(99, 30, 25, 65, 99, 80, 0, 0);
      v.op(2).ratio(1).level(70).vel(7).rate(3).eg(99, 45, 30, 65, 99, 70, 0, 0);
      v.op(3).ratio(3).level(64).vel(7).rate(3).eg(99, 60, 40, 65, 99, 40, 0, 0);
      v.op(4).ratio(1, 0, 9).level(90).vel(2).rate(3).eg(99, 32, 25, 65, 99, 78, 0, 0);
      v.op(5).ratio(1).level(66).vel(6).rate(3).eg(99, 50, 35, 65, 99, 60, 0, 0);
      v.op(6).ratio(7).level(52).vel(7).rate(3).eg(99, 75, 45, 65, 99, 0, 0, 0);
      put(v); }

    { V v("PIANO 2", 3, 2);            // a brighter grand
      v.op(1).ratio(1).level(99).vel(2).rate(3).eg(98, 30, 24, 55, 99, 82, 0, 0);
      v.op(2).ratio(1).level(80).vel(5).rate(3).eg(98, 42, 30, 60, 99, 72, 0, 0).scale(48, 0, 26, 0, 0);
      v.op(3).ratio(1).level(64).vel(6).rate(3).eg(98, 55, 35, 60, 99, 50, 0, 0);
      v.op(4).ratio(1, 0, 9).level(94).vel(2).rate(3).eg(98, 30, 24, 55, 99, 80, 0, 0);
      v.op(5).ratio(4).level(66).vel(5).rate(3).eg(98, 50, 35, 60, 99, 55, 0, 0).scale(48, 0, 30, 0, 0);
      v.op(6).ratio(1).level(70).vel(5).rate(3).eg(98, 60, 40, 60, 99, 40, 0, 0);
      put(v); }

    { V v("UPRIGHT", 5, 0);            // an old upright, a little out of tune with itself
      v.op(1).ratio(1).level(99).vel(2).rate(3).eg(98, 30, 25, 58, 99, 78, 0, 0);
      v.op(2).ratio(1).level(72).vel(5).rate(3).eg(98, 46, 33, 60, 99, 68, 0, 0).scale(48, 0, 22, 0, 0);
      v.op(3).ratio(1, 0, 11).level(95).vel(2).rate(3).eg(98, 32, 25, 58, 99, 76, 0, 0);
      v.op(4).ratio(2).level(66).vel(5).rate(3).eg(98, 50, 35, 60, 99, 55, 0, 0);
      v.op(5).ratio(1, 0, 3).level(90).vel(2).rate(3).eg(98, 32, 25, 58, 99, 74, 0, 0);
      v.op(6).ratio(6).level(58).vel(6).rate(3).eg(99, 75, 40, 70, 99, 0, 0, 0);
      put(v); }

    { V v("HONKY TONK", 5, 0);         // a bar-room piano, its unisons well apart
      v.op(1).ratio(1, 0, 3).level(99).vel(2).rate(3).eg(98, 32, 25, 58, 99, 78, 0, 0);
      v.op(2).ratio(1).level(76).vel(5).rate(3).eg(98, 46, 33, 60, 99, 66, 0, 0);
      v.op(3).ratio(1, 0, 14).level(97).vel(2).rate(3).eg(98, 32, 25, 58, 99, 78, 0, 0);
      v.op(4).ratio(1).level(76).vel(5).rate(3).eg(98, 46, 33, 60, 99, 66, 0, 0);
      v.op(5).ratio(2, 0, 7).level(80).vel(2).rate(3).eg(98, 40, 30, 58, 99, 60, 0, 0);
      v.op(6).ratio(5).level(56).vel(6).rate(3).eg(99, 75, 40, 70, 99, 0, 0, 0);
      put(v); }

    // drawbar organs: six carriers at footages, levels as drawbar settings
    auto organ = [&] (const char* name, std::array<int, 6> ratios, std::array<int, 6> levels,
                      int vibrato, int click)
    {
        V v(name, 32, 0);
        for (int o = 1; o <= 6; ++o)
        {
            const int r = ratios[(size_t) (o - 1)];
            if (r == 15) v.op(o).ratio(1, 50); else v.op(o).ratio(r);
            v.op(o).level(levels[(size_t) (o - 1)]).eg(99, 99, 99, 74, 99, 99, 99, 0);
        }
        if (click) v.op(6).ratio(12).level(click).eg(99, 88, 99, 74, 99, 0, 0, 0);
        v.lfo(37, 0, vibrato, 0, 4, 4);
        return v;
    };

    put(organ("JAZZ ORGAN", { 0, 1, 15, 2, 3, 4 }, { 92, 99, 78, 72, 0, 0 }, 6, 70));
    put(organ("ROCK ORGAN", { 0, 1, 15, 2, 3, 4 }, { 96, 99, 94, 88, 84, 82 }, 8, 72));
    put(organ("GOSPEL ORG", { 0, 1, 2, 3, 4, 6 }, { 94, 99, 92, 80, 86, 70 }, 9, 0));
    put(organ("SOFT ORGAN", { 1, 2, 4, 1, 1, 1 }, { 99, 80, 66, 0, 0, 0 }, 5, 0));
    put(organ("CHURCH ORG", { 0, 1, 2, 3, 4, 8 }, { 99, 98, 92, 84, 88, 76 }, 0, 0));

    { V v("PIPE ORGAN", 2, 6);         // a principal rank: a slow speech and a little wind
      v.op(1).ratio(1).level(99).eg(70, 50, 30, 60, 99, 97, 95, 0);
      v.op(2).ratio(1).level(66).eg(65, 50, 30, 60, 99, 92, 90, 0);
      v.op(3).ratio(2).level(92).eg(70, 50, 30, 60, 99, 97, 95, 0);
      v.op(4).ratio(1).level(60).eg(65, 50, 30, 60, 99, 92, 90, 0);
      v.op(5).ratio(4).level(54).eg(65, 50, 30, 60, 99, 92, 90, 0);
      v.op(6).ratio(1).level(60).eg(65, 50, 30, 60, 99, 92, 90, 0);
      put(v); }

    { V v("CLAV 2", 5, 7);             // a darker clavinet, the pickup on the neck
      v.op(1).ratio(1).level(99).vel(2).rate(3).eg(99, 50, 30, 80, 99, 85, 65, 0);
      v.op(2).ratio(1).level(78).vel(5).rate(3).eg(99, 60, 35, 80, 99, 70, 55, 0);
      v.op(3).ratio(2).level(88).vel(2).rate(3).eg(99, 52, 30, 80, 99, 80, 60, 0);
      v.op(4).ratio(3).level(68).vel(5).rate(3).eg(99, 65, 40, 80, 99, 50, 40, 0);
      v.op(5).ratio(1, 0, 9).level(84).vel(2).rate(3).eg(99, 52, 30, 80, 99, 80, 60, 0);
      v.op(6).ratio(1).level(70).vel(4).rate(3).eg(99, 65, 40, 80, 99, 50, 40, 0);
      put(v); }

    { V v("HARPSI 2", 5, 3);           // a lute stop: the quills muted, short and nasal
      v.op(1).ratio(1).level(99).vel(1).rate(3).eg(99, 45, 32, 70, 99, 55, 0, 0);
      v.op(2).ratio(6).level(70).vel(3).rate(3).eg(99, 60, 40, 70, 99, 40, 0, 0);
      v.op(3).ratio(1, 0, 9).level(92).vel(1).rate(3).eg(99, 45, 32, 70, 99, 55, 0, 0);
      v.op(4).ratio(4).level(72).vel(3).rate(3).eg(99, 60, 40, 70, 99, 40, 0, 0);
      v.op(5).ratio(3).level(78).vel(1).rate(3).eg(99, 50, 35, 70, 99, 45, 0, 0);
      v.op(6).ratio(1).level(66).vel(3).rate(3).eg(99, 60, 40, 70, 99, 40, 0, 0);
      put(v); }

    { V v("MARIMBA 2", 5, 0);          // a softer marimba: rubber mallets, the bar ringing
      v.op(1).ratio(1).level(99).vel(2).rate(4).eg(99, 38, 30, 60, 99, 45, 0, 0);
      v.op(2).ratio(4).level(56).vel(6).rate(4).eg(99, 80, 50, 70, 99, 0, 0, 0);
      v.op(3).ratio(4).level(74).vel(3).rate(5).eg(99, 58, 40, 65, 99, 0, 0, 0);
      v.op(4).ratio(1).level(46).vel(6).rate(4).eg(99, 80, 50, 70, 99, 0, 0, 0);
      v.op(5).ratio(10).level(60).vel(4).rate(5).eg(99, 75, 45, 65, 99, 0, 0, 0);
      v.op(6).ratio(1).level(0);
      put(v); }

    { V v("VIBES 2", 5, 0);            // the motor off: a pure, long vibraphone
      v.op(1).ratio(1).level(99).vel(2).rate(3).eg(99, 26, 22, 38, 99, 72, 0, 0);
      v.op(2).ratio(4).level(58).vel(5).rate(3).eg(99, 60, 40, 50, 99, 20, 0, 0);
      v.op(3).ratio(1, 0, 8).level(94).vel(2).rate(3).eg(99, 28, 22, 38, 99, 72, 0, 0);
      v.op(4).ratio(1).level(46).vel(5).rate(3).eg(99, 60, 40, 50, 99, 30, 0, 0);
      v.op(5).ratio(4).level(68).vel(3).rate(4).eg(99, 52, 35, 45, 99, 0, 0, 0);
      v.op(6).ratio(1).level(0);
      put(v); }

    { V v("XYLO 2", 5, 0);             // a brighter xylophone
      v.op(1).ratio(1).level(99).vel(2).rate(5).eg(99, 52, 40, 65, 99, 0, 0, 0);
      v.op(2).ratio(3).level(74).vel(5).rate(5).eg(99, 78, 50, 70, 99, 0, 0, 0);
      v.op(3).ratio(3, 0, 9).level(88).vel(2).rate(5).eg(99, 58, 40, 65, 99, 0, 0, 0);
      v.op(4).ratio(1).level(60).vel(5).rate(5).eg(99, 80, 50, 70, 99, 0, 0, 0);
      v.op(5).ratio(6).level(70).vel(2).rate(5).eg(99, 68, 45, 65, 99, 0, 0, 0);
      v.op(6).ratio(9).level(50).vel(5).rate(5).eg(99, 85, 50, 70, 99, 0, 0, 0);
      v.transpose(12);
      put(v); }

    { V v("MUSIC BOX", 5, 0);          // a comb plucked by a drum's pins
      v.op(1).ratio(2).level(99).vel(2).rate(4).eg(99, 34, 28, 40, 99, 40, 0, 0);
      v.op(2).ratio(7).level(60).vel(5).rate(4).eg(99, 60, 40, 45, 99, 0, 0, 0);
      v.op(3).ratio(6, 0, 9).level(76).vel(2).rate(5).eg(99, 45, 32, 40, 99, 20, 0, 0);
      v.op(4).ratio(1).level(48).vel(5).rate(4).eg(99, 75, 45, 45, 99, 0, 0, 0);
      v.op(5).ratio(2, 0, 5).level(90).vel(2).rate(4).eg(99, 36, 28, 40, 99, 40, 0, 0);
      v.op(6).ratio(13).level(44).vel(5).rate(4).eg(99, 85, 50, 45, 99, 0, 0, 0);
      v.transpose(12);
      put(v); }

    { V v("TOY PIANO", 5, 0);          // hammers on metal rods: clangy, a little sour
      v.op(1).ratio(1).level(99).vel(2).rate(5).eg(99, 45, 35, 55, 99, 20, 0, 0);
      v.op(2).ratio(3, 41).level(66).vel(5).rate(5).eg(99, 65, 45, 55, 99, 0, 0, 0);
      v.op(3).ratio(2, 76).level(80).vel(2).rate(5).eg(99, 50, 38, 55, 99, 10, 0, 0);
      v.op(4).ratio(1).level(56).vel(5).rate(5).eg(99, 75, 45, 55, 99, 0, 0, 0);
      v.op(5).ratio(5, 40).level(70).vel(2).rate(5).eg(99, 60, 40, 55, 99, 0, 0, 0);
      v.op(6).ratio(1).level(0);
      v.transpose(12);
      put(v); }

    { V v("CHIMES", 5, 0);             // orchestral chimes: tubes, the strike tone ringing
      v.op(1).ratio(1).level(99).vel(2).rate(2).eg(99, 22, 20, 28, 99, 75, 0, 0);
      v.op(2).ratio(2, 76).level(78).vel(4).rate(2).eg(99, 28, 22, 30, 99, 65, 0, 0);
      v.op(3).ratio(2, 0, 9).level(90).vel(2).rate(2).eg(99, 24, 20, 28, 99, 70, 0, 0);
      v.op(4).ratio(5, 40).level(70).vel(4).rate(2).eg(99, 32, 25, 30, 99, 60, 0, 0);
      v.op(5).ratio(4, 25).level(82).vel(2).rate(3).eg(99, 35, 25, 30, 99, 50, 0, 0);
      v.op(6).ratio(1).level(56).vel(4).rate(2).eg(99, 50, 35, 30, 99, 20, 0, 0);
      put(v); }

    { V v("BELLS", 5, 0);              // hand bells, a cluster of inharmonic partials
      v.op(1).ratio(1).level(99).vel(2).rate(3).eg(99, 28, 22, 32, 99, 65, 0, 0);
      v.op(2).ratio(1, 41).level(76).vel(4).rate(3).eg(99, 34, 25, 35, 99, 55, 0, 0);
      v.op(3).ratio(2, 0, 9).level(86).vel(2).rate(3).eg(99, 32, 24, 32, 99, 55, 0, 0);
      v.op(4).ratio(3, 50).level(70).vel(4).rate(3).eg(99, 40, 28, 35, 99, 45, 0, 0);
      v.op(5).ratio(3, 0, 4).level(80).vel(2).rate(3).eg(99, 36, 26, 32, 99, 45, 0, 0);
      v.op(6).ratio(7, 20).level(60).vel(4).rate(3).eg(99, 50, 35, 35, 99, 20, 0, 0);
      v.transpose(12);
      put(v); }

    { V v("GAMELAN", 5, 0);            // a bronze gong-chime, tuned and beating
      v.op(1).ratio(1).level(99).vel(2).rate(3).eg(99, 30, 24, 35, 99, 60, 0, 0);
      v.op(2).ratio(2, 33).level(72).vel(4).rate(3).eg(99, 38, 28, 40, 99, 50, 0, 0);
      v.op(3).ratio(1, 0, 11).level(94).vel(2).rate(3).eg(99, 30, 24, 35, 99, 60, 0, 0);
      v.op(4).ratio(4, 61).level(64).vel(4).rate(3).eg(99, 45, 30, 40, 99, 30, 0, 0);
      v.op(5).ratio(2, 70).level(78).vel(2).rate(3).eg(99, 40, 28, 35, 99, 40, 0, 0);
      v.op(6).ratio(1).level(60).vel(4).rate(3).eg(99, 60, 40, 40, 99, 0, 0, 0);
      put(v); }

    { V v("CELESTE 2", 7, 0);          // a sweeter celesta
      v.op(1).ratio(2).level(99).vel(2).rate(4).eg(99, 38, 28, 45, 99, 45, 0, 0);
      v.op(2).ratio(8).level(56).vel(5).rate(4).eg(99, 70, 45, 50, 99, 0, 0, 0);
      v.op(3).ratio(1).level(92).vel(2).rate(4).eg(99, 36, 28, 45, 99, 55, 0, 0);
      v.op(4).ratio(4).level(56).vel(5).rate(4).eg(99, 60, 40, 50, 99, 20, 0, 0);
      v.op(5).ratio(1).level(48).vel(5).rate(4).eg(99, 70, 45, 50, 99, 0, 0, 0);
      v.op(6).ratio(1).level(0);
      v.transpose(12);
      put(v); }

    { V v("KOTO", 3, 5);               // a plucked zither, the string bending up as it starts
      v.op(1).ratio(1).level(99).vel(2).rate(3).eg(99, 36, 28, 55, 99, 60, 0, 0);
      v.op(2).ratio(3).level(72).vel(5).rate(3).eg(99, 55, 38, 55, 99, 40, 0, 0);
      v.op(3).ratio(1).level(64).vel(5).rate(3).eg(99, 70, 45, 55, 99, 20, 0, 0);
      v.op(4).ratio(1, 0, 9).level(90).vel(2).rate(3).eg(99, 38, 28, 55, 99, 58, 0, 0);
      v.op(5).ratio(4).level(66).vel(5).rate(3).eg(99, 55, 38, 55, 99, 30, 0, 0);
      v.op(6).ratio(1).level(66).vel(4).rate(3).eg(99, 65, 40, 55, 99, 20, 0, 0);
      v.pitchEg(99, 75, 95, 60, 47, 50, 50, 50);
      put(v); }

    { V v("BANJO", 5, 5);              // a bright, quick-dying pluck over a drum head
      v.op(1).ratio(1).level(99).vel(2).rate(4).eg(99, 48, 36, 60, 99, 40, 0, 0);
      v.op(2).ratio(3).level(76).vel(5).rate(4).eg(99, 60, 40, 60, 99, 30, 0, 0);
      v.op(3).ratio(2, 0, 9).level(86).vel(2).rate(4).eg(99, 50, 36, 60, 99, 35, 0, 0);
      v.op(4).ratio(5).level(70).vel(5).rate(4).eg(99, 65, 45, 60, 99, 20, 0, 0);
      v.op(5).ratio(1, 0, 5).level(88).vel(2).rate(4).eg(99, 50, 36, 60, 99, 35, 0, 0);
      v.op(6).ratio(1).level(76).vel(4).rate(4).eg(99, 60, 40, 60, 99, 30, 0, 0);
      put(v); }

    { V v("E.GRAND", 5, 3);            // an electric grand: strings and pickups
      v.op(1).ratio(1).level(99).vel(2).rate(3).eg(98, 30, 25, 60, 99, 80, 0, 0);
      v.op(2).ratio(1).level(78).vel(5).rate(3).eg(98, 45, 32, 60, 99, 70, 0, 0).scale(48, 0, 22, 0, 0);
      v.op(3).ratio(1, 0, 10).level(95).vel(2).rate(3).eg(98, 30, 25, 60, 99, 78, 0, 0);
      v.op(4).ratio(5).level(66).vel(6).rate(3).eg(98, 60, 40, 60, 99, 40, 0, 0).scale(48, 0, 26, 0, 0);
      v.op(5).ratio(1, 0, 4).level(92).vel(2).rate(3).eg(98, 32, 25, 60, 99, 76, 0, 0);
      v.op(6).ratio(1).level(72).vel(5).rate(3).eg(98, 48, 34, 60, 99, 66, 0, 0);
      v.lfo(36, 0, 0, 0, 4, 0);
      put(v); }

    { V v("STEEL GTR", 5, 4);          // a flat-top acoustic: bright strings, a big body
      v.op(1).ratio(1).level(99).vel(2).rate(3).eg(99, 34, 27, 58, 99, 74, 0, 0);
      v.op(2).ratio(2).level(72).vel(5).rate(3).eg(99, 52, 36, 60, 99, 55, 0, 0).scale(48, 0, 22, 0, 0);
      v.op(3).ratio(1, 0, 9).level(94).vel(2).rate(3).eg(99, 36, 27, 58, 99, 72, 0, 0);
      v.op(4).ratio(4).level(64).vel(5).rate(3).eg(99, 60, 40, 60, 99, 40, 0, 0).scale(48, 0, 26, 0, 0);
      v.op(5).ratio(2, 0, 5).level(84).vel(2).rate(3).eg(99, 38, 28, 58, 99, 62, 0, 0);
      v.op(6).ratio(1).level(70).vel(4).rate(3).eg(99, 56, 38, 60, 99, 45, 0, 0);
      put(v); }

    { V v("MANDOLIN", 5, 5);           // paired steel strings, played with a tremolo pick
      v.op(1).ratio(1, 0, 5).level(99).vel(2).rate(4).ams(2).eg(99, 42, 32, 62, 99, 55, 0, 0);
      v.op(2).ratio(3).level(72).vel(5).rate(4).eg(99, 58, 40, 62, 99, 35, 0, 0);
      v.op(3).ratio(1, 0, 10).level(97).vel(2).rate(4).ams(2).eg(99, 42, 32, 62, 99, 55, 0, 0);
      v.op(4).ratio(3).level(70).vel(5).rate(4).eg(99, 58, 40, 62, 99, 35, 0, 0);
      v.op(5).ratio(2).level(80).vel(2).rate(4).eg(99, 46, 34, 62, 99, 40, 0, 0);
      v.op(6).ratio(6).level(58).vel(5).rate(4).eg(99, 70, 45, 62, 99, 0, 0, 0);
      v.lfo(68, 0, 0, 0, 1, 0);
      put(v); }

    { V v("DULCIMER", 5, 2);           // a hammered dulcimer: courses of wire, ringing on
      v.op(1).ratio(1, 0, 4).level(99).vel(2).rate(3).eg(99, 28, 24, 40, 99, 70, 0, 0);
      v.op(2).ratio(3).level(70).vel(5).rate(3).eg(99, 45, 32, 45, 99, 45, 0, 0);
      v.op(3).ratio(1, 0, 10).level(97).vel(2).rate(3).eg(99, 28, 24, 40, 99, 70, 0, 0);
      v.op(4).ratio(5).level(60).vel(5).rate(3).eg(99, 55, 38, 45, 99, 30, 0, 0);
      v.op(5).ratio(2, 0, 7).level(84).vel(2).rate(3).eg(99, 32, 25, 40, 99, 55, 0, 0);
      v.op(6).ratio(1).level(66).vel(5).rate(3).eg(99, 50, 35, 45, 99, 40, 0, 0);
      put(v); }

    return c;
}

// ================================================================ BANK 3: BASS & LEAD

Cartridge bassLead()
{
    Cartridge c;
    int i = 0;
    auto put = [&] (const dx7::Patch& p) { c.voices[(size_t) i++] = p; };

    { V v("BASS 2", 5, 6);             // the classic FM bass: punchy, with a dark sustain
      v.op(1).ratio(0).level(99).vel(2).rate(2).eg(99, 40, 30, 72, 99, 85, 70, 0);
      v.op(2).ratio(0).level(82).vel(5).rate(2).eg(99, 62, 40, 72, 99, 66, 50, 0);
      v.op(3).ratio(0, 0, 9).level(92).vel(2).rate(2).eg(99, 40, 30, 72, 99, 85, 70, 0);
      v.op(4).ratio(1).level(74).vel(5).rate(2).eg(99, 70, 45, 72, 99, 45, 0, 0);
      v.op(5).ratio(1).level(80).vel(2).rate(2).eg(99, 50, 35, 72, 99, 60, 0, 0);
      v.op(6).ratio(1).level(66).vel(4).rate(2).eg(99, 75, 45, 72, 99, 30, 0, 0);
      put(v); }

    { V v("PICK BASS", 1, 5);          // a picked electric bass
      v.op(1).ratio(0).level(99).vel(2).rate(2).eg(99, 38, 30, 75, 99, 85, 72, 0);
      v.op(2).ratio(0).level(80).vel(5).rate(2).eg(99, 65, 40, 75, 99, 60, 45, 0);
      v.op(3).ratio(1).level(84).vel(2).rate(2).eg(99, 45, 32, 75, 99, 70, 50, 0);
      v.op(4).ratio(2).level(66).vel(6).rate(2).eg(99, 78, 50, 75, 99, 20, 0, 0);
      v.op(5).ratio(5).level(54).vel(6).rate(2).eg(99, 85, 50, 75, 99, 0, 0, 0);
      v.op(6).ratio(1).level(60).vel(4).rate(2).eg(99, 75, 45, 75, 99, 30, 0, 0);
      put(v); }

    { V v("FRETLESS", 1, 4);           // a fretless: a soft attack, a singing middle
      v.op(1).ratio(0).level(99).vel(1).rate(2).eg(85, 35, 28, 70, 99, 90, 78, 0);
      v.op(2).ratio(0).level(74).vel(3).rate(2).eg(80, 45, 32, 70, 99, 82, 70, 0);
      v.op(3).ratio(0, 0, 9).level(90).vel(1).rate(2).eg(85, 35, 28, 70, 99, 90, 78, 0);
      v.op(4).ratio(1).level(66).vel(3).rate(2).eg(80, 50, 35, 70, 99, 75, 60, 0);
      v.op(5).ratio(1).level(50).rate(2).eg(80, 50, 35, 70, 99, 70, 60, 0);
      v.op(6).ratio(1).level(56).rate(2).eg(80, 50, 35, 70, 99, 70, 60, 0);
      v.lfo(34, 50, 5, 0, 4, 3);
      v.pitchEg(85, 95, 95, 60, 48, 50, 50, 50);
      put(v); }

    { V v("UPRT BASS", 5, 3);          // acoustic upright: woody, a quick decay
      v.op(1).ratio(0).level(99).vel(2).rate(2).eg(99, 42, 32, 65, 99, 70, 0, 0);
      v.op(2).ratio(0).level(72).vel(5).rate(2).eg(99, 60, 40, 65, 99, 55, 0, 0);
      v.op(3).ratio(1).level(80).vel(2).rate(2).eg(99, 48, 35, 65, 99, 50, 0, 0);
      v.op(4).ratio(2).level(60).vel(5).rate(2).eg(99, 70, 45, 65, 99, 20, 0, 0);
      v.op(5).ratio(0, 0, 9).level(86).vel(2).rate(2).eg(99, 44, 32, 65, 99, 65, 0, 0);
      v.op(6).ratio(3).level(52).vel(6).rate(2).eg(99, 85, 50, 65, 99, 0, 0, 0);
      put(v); }

    { V v("SYN BASS 1", 18, 7);        // a resonant synth bass: the sweep on the attack
      v.op(1).ratio(0).level(99).vel(1).rate(2).eg(99, 45, 30, 75, 99, 90, 80, 0);
      v.op(2).ratio(0).level(80).vel(4).rate(2).eg(99, 55, 35, 75, 99, 70, 55, 0);
      v.op(3).ratio(1).level(78).vel(4).rate(2).eg(99, 65, 40, 75, 99, 50, 30, 0);
      v.op(4).ratio(0).level(78).vel(3).rate(2).eg(99, 50, 35, 75, 99, 75, 65, 0);
      v.op(5).ratio(0).level(70).rate(2).eg(99, 50, 35, 75, 99, 75, 65, 0);
      v.op(6).ratio(0).level(72).rate(2).eg(99, 50, 35, 75, 99, 75, 65, 0);
      put(v); }

    { V v("SYN BASS 2", 16, 6);        // rubbery, with a pitch drop on every note
      v.op(1).ratio(0).level(99).vel(1).rate(2).eg(99, 42, 30, 75, 99, 88, 76, 0);
      v.op(2).ratio(0).level(78).vel(4).rate(2).eg(99, 62, 38, 75, 99, 60, 45, 0);
      v.op(3).ratio(1).level(70).vel(4).rate(2).eg(99, 70, 40, 75, 99, 40, 0, 0);
      v.op(4).ratio(0).level(64).rate(2).eg(99, 60, 40, 75, 99, 60, 45, 0);
      v.op(5).ratio(2).level(62).vel(4).rate(2).eg(99, 72, 45, 75, 99, 30, 0, 0);
      v.op(6).ratio(1).level(68).rate(2).eg(99, 60, 40, 75, 99, 60, 45, 0);
      v.pitchEg(99, 70, 95, 60, 53, 50, 50, 50);
      put(v); }

    { V v("SUB BASS", 1, 0);           // a near-sine under everything
      v.op(1).ratio(0).level(99).vel(1).rate(1).eg(95, 40, 30, 72, 99, 92, 85, 0);
      v.op(2).ratio(0).level(58).vel(3).rate(1).eg(99, 55, 40, 72, 99, 70, 60, 0);
      v.op(3).ratio(1).level(66).vel(1).rate(1).eg(95, 50, 35, 72, 99, 70, 50, 0);
      v.op(4).ratio(1).level(40).rate(1).eg(99, 60, 40, 72, 99, 50, 30, 0);
      v.op(5).ratio(1).level(0);
      v.op(6).ratio(1).level(0);
      put(v); }

    { V v("WOOD BASS", 7, 0);          // a hollow, knocking bass
      v.op(1).ratio(0).level(99).vel(2).rate(2).eg(99, 45, 34, 70, 99, 70, 0, 0);
      v.op(2).ratio(1).level(66).vel(5).rate(2).eg(99, 70, 45, 70, 99, 30, 0, 0);
      v.op(3).ratio(0, 0, 9).level(90).vel(2).rate(2).eg(99, 45, 34, 70, 99, 70, 0, 0);
      v.op(4).ratio(3).level(56).vel(5).rate(2).eg(99, 80, 50, 70, 99, 0, 0, 0);
      v.op(5).ratio(1).level(62).vel(5).rate(2).eg(99, 75, 45, 70, 99, 20, 0, 0);
      v.op(6).ratio(1).level(60).rate(2).eg(99, 75, 45, 70, 99, 20, 0, 0);
      put(v); }

    { V v("DIGI BASS", 5, 7);          // hard and metallic, for the eighties
      v.op(1).ratio(0).level(99).vel(2).rate(2).eg(99, 45, 32, 75, 99, 80, 65, 0);
      v.op(2).ratio(1).level(84).vel(5).rate(2).eg(99, 65, 40, 75, 99, 60, 45, 0);
      v.op(3).ratio(0, 0, 9).level(92).vel(2).rate(2).eg(99, 45, 32, 75, 99, 80, 65, 0);
      v.op(4).ratio(3).level(72).vel(5).rate(2).eg(99, 75, 45, 75, 99, 30, 0, 0);
      v.op(5).ratio(1).level(84).vel(2).rate(2).eg(99, 50, 35, 75, 99, 55, 30, 0);
      v.op(6).ratio(1).level(78).vel(4).rate(2).eg(99, 70, 45, 75, 99, 40, 20, 0);
      put(v); }

    { V v("ACID FM", 16, 7);           // a squelch: the index swept down on every note
      v.op(1).ratio(0).level(99).vel(1).rate(2).eg(99, 40, 30, 78, 99, 86, 76, 0);
      v.op(2).ratio(0).level(84).vel(5).rate(2).eg(99, 48, 35, 78, 99, 60, 40, 0);
      v.op(3).ratio(1).level(76).vel(5).rate(2).eg(99, 55, 38, 78, 99, 50, 30, 0);
      v.op(4).ratio(2).level(68).rate(2).eg(99, 60, 40, 78, 99, 40, 20, 0);
      v.op(5).ratio(0).level(80).vel(5).rate(2).eg(99, 50, 35, 78, 99, 55, 35, 0);
      v.op(6).ratio(0).level(78).rate(2).eg(99, 55, 38, 78, 99, 60, 40, 0);
      put(v); }

    { V v("OCTAVE BASS", 5, 5);        // a bass doubled an octave up
      v.op(1).ratio(0).level(99).vel(2).rate(2).eg(99, 40, 30, 72, 99, 85, 70, 0);
      v.op(2).ratio(0).level(78).vel(5).rate(2).eg(99, 62, 40, 72, 99, 62, 45, 0);
      v.op(3).ratio(1).level(90).vel(2).rate(2).eg(99, 40, 30, 72, 99, 85, 70, 0);
      v.op(4).ratio(1).level(70).vel(5).rate(2).eg(99, 62, 40, 72, 99, 62, 45, 0);
      v.op(5).ratio(0, 0, 10).level(84).vel(2).rate(2).eg(99, 40, 30, 72, 99, 85, 70, 0);
      v.op(6).ratio(1).level(60).vel(4).rate(2).eg(99, 70, 45, 72, 99, 30, 0, 0);
      put(v); }

    { V v("MOOGY BASS", 16, 7);        // a fat, round analogue-style bass
      v.op(1).ratio(0).level(99).vel(1).rate(2).eg(99, 38, 30, 75, 99, 90, 82, 0);
      v.op(2).ratio(0, 0, 9).level(76).vel(3).rate(2).eg(99, 50, 35, 75, 99, 80, 70, 0);
      v.op(3).ratio(0, 0, 5).level(76).vel(3).rate(2).eg(99, 50, 35, 75, 99, 80, 70, 0);
      v.op(4).ratio(1).level(56).rate(2).eg(99, 60, 40, 75, 99, 60, 40, 0);
      v.op(5).ratio(0).level(72).vel(3).rate(2).eg(99, 55, 38, 75, 99, 70, 60, 0);
      v.op(6).ratio(0).level(74).rate(2).eg(99, 55, 38, 75, 99, 75, 65, 0);
      put(v); }

    { V v("LEAD 2", 16, 7);            // a sawtooth lead with a little sweep
      v.op(1).ratio(1).level(99).vel(1).eg(99, 50, 30, 70, 99, 95, 92, 0);
      v.op(2).ratio(1, 0, 9).level(74).vel(2).eg(99, 50, 30, 70, 99, 90, 88, 0);
      v.op(3).ratio(1).level(74).vel(2).eg(90, 50, 30, 70, 99, 88, 85, 0);
      v.op(4).ratio(2).level(50).eg(99, 50, 30, 70, 99, 90, 85, 0);
      v.op(5).ratio(1, 0, 5).level(72).vel(2).eg(99, 50, 30, 70, 99, 90, 88, 0);
      v.op(6).ratio(1).level(74).eg(99, 50, 30, 70, 99, 90, 88, 0);
      v.lfo(36, 50, 9, 0, 0, 4);
      put(v); }

    { V v("SQUARE LEAD", 7, 6);        // hollow: odd harmonics from a 2:1 pair
      v.op(1).ratio(1).level(99).vel(1).eg(99, 50, 30, 70, 99, 95, 92, 0);
      v.op(2).ratio(2).level(72).vel(2).eg(99, 50, 30, 70, 99, 92, 90, 0);
      v.op(3).ratio(1, 0, 9).level(94).vel(1).eg(99, 50, 30, 70, 99, 95, 92, 0);
      v.op(4).ratio(2).level(70).vel(2).eg(99, 50, 30, 70, 99, 92, 90, 0);
      v.op(5).ratio(2).level(50).eg(99, 50, 30, 70, 99, 90, 85, 0);
      v.op(6).ratio(2).level(56).eg(99, 50, 30, 70, 99, 90, 85, 0);
      v.lfo(36, 50, 8, 0, 4, 4);
      put(v); }

    { V v("SYNC LEAD", 1, 7);          // a hard-sync tear, the modulator sweeping the ratio
      v.op(1).ratio(1).level(99).vel(1).eg(99, 50, 30, 70, 99, 95, 92, 0);
      v.op(2).ratio(4).level(82).vel(3).eg(99, 40, 25, 70, 99, 70, 60, 0);
      v.op(3).ratio(1, 0, 9).level(90).vel(1).eg(99, 50, 30, 70, 99, 95, 92, 0);
      v.op(4).ratio(1).level(76).vel(2).eg(99, 50, 30, 70, 99, 90, 85, 0);
      v.op(5).ratio(3).level(66).eg(99, 45, 30, 70, 99, 80, 70, 0);
      v.op(6).ratio(1).level(68).eg(99, 50, 30, 70, 99, 90, 85, 0);
      v.lfo(36, 55, 9, 0, 0, 4);
      v.pitchEg(99, 80, 95, 60, 52, 50, 50, 50);
      put(v); }

    { V v("WHISTLE", 1, 0);            // a pure, breathy whistle with a lot of vibrato
      v.op(1).ratio(2).level(96).eg(70, 40, 30, 65, 99, 96, 94, 0);
      v.op(2).ratio(2).level(40).eg(70, 40, 30, 65, 99, 90, 88, 0);
      v.op(3).ratio(2).level(40).eg(75, 50, 30, 65, 99, 70, 60, 0);
      v.op(4).ratio(13).level(56).eg(75, 50, 30, 65, 99, 60, 50, 0);
      v.op(5).ratio(1).level(0);
      v.op(6).ratio(1).level(0);
      v.lfo(38, 40, 14, 0, 4, 4);
      put(v); }

    { V v("BRASS LEAD", 18, 7);        // a lead that blares like a horn section
      v.op(1).ratio(1).level(99).vel(1).eg(75, 45, 30, 65, 99, 94, 90, 0);
      v.op(2).ratio(1).level(78).vel(3).eg(65, 45, 30, 65, 99, 90, 86, 0);
      v.op(3).ratio(2).level(66).vel(3).eg(65, 45, 30, 65, 99, 86, 82, 0);
      v.op(4).ratio(1).level(80).vel(3).eg(60, 45, 30, 65, 99, 90, 86, 0);
      v.op(5).ratio(1).level(70).eg(65, 45, 30, 65, 99, 88, 84, 0);
      v.op(6).ratio(1).level(70).eg(65, 45, 30, 65, 99, 88, 84, 0);
      v.lfo(36, 50, 8, 0, 4, 4);
      v.pitchEg(82, 95, 95, 60, 46, 50, 50, 50);
      put(v); }

    { V v("SOLO VIOLIN", 2, 7);        // a bowed string: a sawtooth with a vibrato
      v.op(1).ratio(1).level(99).vel(1).eg(55, 40, 30, 55, 99, 95, 92, 0);
      v.op(2).ratio(1).level(76).vel(2).eg(58, 40, 30, 55, 99, 92, 90, 0).scale(55, 0, 20, 0, 0);
      v.op(3).ratio(3).level(66).eg(58, 40, 30, 55, 99, 88, 85, 0);
      v.op(4).ratio(1).level(66).eg(58, 40, 30, 55, 99, 88, 85, 0);
      v.op(5).ratio(4).level(50).eg(58, 40, 30, 55, 99, 85, 80, 0);
      v.op(6).ratio(1).level(60).eg(58, 40, 30, 55, 99, 85, 80, 0);
      v.lfo(36, 45, 10, 0, 4, 4);
      put(v); }

    { V v("CLARINET", 7, 5);           // odd harmonics, woody in the low register
      v.op(1).ratio(1).level(98).vel(1).eg(65, 40, 30, 60, 99, 96, 93, 0);
      v.op(2).ratio(2).level(70).vel(2).eg(68, 40, 30, 60, 99, 92, 90, 0).scale(55, 0, 20, 0, 0);
      v.op(3).ratio(3).level(72).eg(65, 40, 30, 60, 99, 90, 88, 0);
      v.op(4).ratio(2).level(58).eg(68, 40, 30, 60, 99, 88, 85, 0);
      v.op(5).ratio(2).level(50).eg(68, 40, 30, 60, 99, 85, 80, 0);
      v.op(6).ratio(1).level(50).eg(68, 40, 30, 60, 99, 85, 80, 0);
      v.lfo(34, 55, 6, 0, 4, 3);
      put(v); }

    { V v("SAX", 18, 7);               // a tenor sax: reedy, with a growl when pushed
      v.op(1).ratio(1).level(99).vel(1).eg(72, 45, 30, 65, 99, 94, 90, 0);
      v.op(2).ratio(1).level(74).vel(4).eg(70, 45, 30, 65, 99, 90, 86, 0);
      v.op(3).ratio(3).level(64).vel(4).eg(70, 45, 30, 65, 99, 86, 82, 0);
      v.op(4).ratio(1).level(72).vel(3).eg(68, 45, 30, 65, 99, 90, 86, 0);
      v.op(5).ratio(5).level(52).eg(70, 45, 30, 65, 99, 80, 75, 0);
      v.op(6).ratio(1).level(66).eg(70, 45, 30, 65, 99, 88, 84, 0);
      v.lfo(36, 55, 7, 0, 4, 4);
      v.pitchEg(85, 95, 95, 60, 47, 50, 50, 50);
      put(v); }

    { V v("HARMONICA", 1, 6);          // a blues harp: reedy, bent in
      v.op(1).ratio(1).level(99).vel(1).eg(78, 45, 30, 65, 99, 95, 92, 0);
      v.op(2).ratio(1).level(76).vel(3).eg(75, 45, 30, 65, 99, 90, 86, 0);
      v.op(3).ratio(2).level(86).vel(1).eg(78, 45, 30, 65, 99, 92, 90, 0);
      v.op(4).ratio(3).level(66).vel(3).eg(75, 45, 30, 65, 99, 86, 82, 0);
      v.op(5).ratio(1).level(56).eg(75, 45, 30, 65, 99, 86, 82, 0);
      v.op(6).ratio(1).level(60).eg(75, 45, 30, 65, 99, 86, 82, 0);
      v.lfo(40, 45, 8, 10, 4, 4);
      v.op(1).ams(1);
      v.pitchEg(80, 95, 95, 60, 46, 50, 50, 50);
      put(v); }

    { V v("PAN FLUTE", 2, 7);          // breathy pipes, the chiff from the feedback
      v.op(1).ratio(1).level(97).vel(1).eg(60, 40, 30, 60, 99, 96, 93, 0);
      v.op(2).ratio(1).level(52).vel(2).eg(65, 45, 30, 60, 99, 90, 88, 0);
      v.op(3).ratio(3).level(50).eg(60, 40, 30, 60, 99, 85, 80, 0);
      v.op(4).ratio(11).level(54).vel(2).eg(90, 60, 40, 60, 99, 60, 50, 0);
      v.op(5).ratio(1).level(36).eg(90, 60, 40, 60, 99, 60, 50, 0);
      v.op(6).ratio(1).level(0);
      v.lfo(34, 45, 8, 0, 4, 4);
      put(v); }

    { V v("CHIP LEAD", 5, 0);          // a game console's square wave
      v.op(1).ratio(1).level(99).eg(99, 99, 99, 85, 99, 99, 99, 0);
      v.op(2).ratio(2).level(80).eg(99, 99, 99, 85, 99, 99, 99, 0);
      v.op(3).ratio(1).level(0);
      v.op(4).ratio(1).level(0);
      v.op(5).ratio(2).level(80).eg(99, 99, 99, 85, 99, 99, 99, 0);
      v.op(6).ratio(4).level(66).eg(99, 99, 99, 85, 99, 99, 99, 0);
      v.lfo(40, 60, 10, 0, 4, 4);
      put(v); }

    { V v("DIST GUITAR", 1, 7);        // an overdriven guitar: a dense, clipped sawtooth
      v.op(1).ratio(1).level(99).vel(1).rate(2).eg(99, 30, 25, 70, 99, 92, 85, 0);
      v.op(2).ratio(1).level(90).vel(2).rate(2).eg(99, 35, 25, 70, 99, 88, 82, 0);
      v.op(3).ratio(0, 0, 9).level(92).vel(1).rate(2).eg(99, 30, 25, 70, 99, 92, 85, 0);
      v.op(4).ratio(1).level(84).vel(2).rate(2).eg(99, 35, 25, 70, 99, 88, 82, 0);
      v.op(5).ratio(2).level(80).rate(2).eg(99, 35, 25, 70, 99, 85, 80, 0);
      v.op(6).ratio(1).level(84).rate(2).eg(99, 35, 25, 70, 99, 85, 80, 0);
      v.lfo(36, 60, 8, 0, 4, 3);
      put(v); }

    { V v("MUTE GUITAR", 3, 5);        // palm-muted: a short, chunky pluck
      v.op(1).ratio(1).level(99).vel(2).rate(3).eg(99, 55, 40, 75, 99, 30, 0, 0);
      v.op(2).ratio(1).level(76).vel(5).rate(3).eg(99, 65, 45, 75, 99, 20, 0, 0);
      v.op(3).ratio(3).level(62).vel(5).rate(3).eg(99, 75, 50, 75, 99, 0, 0, 0);
      v.op(4).ratio(1, 0, 9).level(90).vel(2).rate(3).eg(99, 55, 40, 75, 99, 30, 0, 0);
      v.op(5).ratio(2).level(66).vel(5).rate(3).eg(99, 70, 45, 75, 99, 10, 0, 0);
      v.op(6).ratio(1).level(66).vel(4).rate(3).eg(99, 70, 45, 75, 99, 10, 0, 0);
      put(v); }

    { V v("FUNK GUITAR", 5, 6);        // a clean, spanky single-coil
      v.op(1).ratio(1).level(99).vel(2).rate(3).eg(99, 42, 32, 75, 99, 70, 40, 0);
      v.op(2).ratio(2).level(74).vel(5).rate(3).eg(99, 55, 38, 75, 99, 55, 30, 0);
      v.op(3).ratio(2, 0, 9).level(86).vel(2).rate(3).eg(99, 45, 34, 75, 99, 60, 30, 0);
      v.op(4).ratio(5).level(62).vel(5).rate(3).eg(99, 65, 45, 75, 99, 30, 0, 0);
      v.op(5).ratio(1, 0, 5).level(90).vel(2).rate(3).eg(99, 42, 32, 75, 99, 70, 40, 0);
      v.op(6).ratio(1).level(70).vel(4).rate(3).eg(99, 60, 40, 75, 99, 40, 20, 0);
      put(v); }

    { V v("BRASS 2", 22, 7);           // a brighter, punchier section
      v.op(1).ratio(1).level(97).vel(1).eg(78, 45, 30, 62, 99, 93, 90, 0);
      v.op(2).ratio(1).level(82).vel(3).eg(65, 45, 30, 62, 99, 90, 86, 0);
      v.op(3).ratio(1, 0, 5).level(97).vel(1).eg(78, 45, 30, 62, 99, 93, 90, 0);
      v.op(4).ratio(1, 0, 9).level(96).vel(1).eg(78, 45, 30, 62, 99, 93, 90, 0);
      v.op(5).ratio(2).level(86).vel(1).eg(78, 45, 30, 62, 99, 90, 86, 0);
      v.op(6).ratio(1).level(80).vel(4).eg(62, 45, 30, 62, 99, 92, 88, 0);
      v.lfo(35, 45, 5, 0, 0, 3);
      v.pitchEg(86, 95, 95, 60, 46, 50, 50, 50);
      put(v); }

    { V v("HORNS", 22, 6);             // French horns: dark and round
      v.op(1).ratio(1).level(97).vel(1).eg(55, 40, 30, 55, 99, 94, 90, 0);
      v.op(2).ratio(1).level(66).vel(3).eg(50, 40, 30, 55, 99, 88, 84, 0);
      v.op(3).ratio(1, 0, 6).level(96).vel(1).eg(55, 40, 30, 55, 99, 94, 90, 0);
      v.op(4).ratio(1, 0, 9).level(95).vel(1).eg(55, 40, 30, 55, 99, 94, 90, 0);
      v.op(5).ratio(0).level(90).vel(1).eg(55, 40, 30, 55, 99, 94, 90, 0);
      v.op(6).ratio(1).level(64).vel(3).eg(45, 40, 30, 55, 99, 88, 84, 0);
      v.lfo(33, 50, 4, 0, 4, 3);
      put(v); }

    { V v("TROMBONE", 18, 7);          // a slide trombone, sliding in from below
      v.op(1).ratio(1).level(99).vel(1).eg(65, 45, 30, 60, 99, 94, 90, 0);
      v.op(2).ratio(1).level(76).vel(3).eg(55, 45, 30, 60, 99, 88, 84, 0);
      v.op(3).ratio(2).level(60).vel(3).eg(55, 45, 30, 60, 99, 84, 80, 0);
      v.op(4).ratio(1).level(72).vel(3).eg(50, 45, 30, 60, 99, 88, 84, 0);
      v.op(5).ratio(1).level(64).eg(55, 45, 30, 60, 99, 86, 82, 0);
      v.op(6).ratio(1).level(64).eg(55, 45, 30, 60, 99, 86, 82, 0);
      v.lfo(35, 55, 6, 0, 4, 3);
      v.pitchEg(70, 95, 95, 60, 44, 50, 50, 50);
      v.transpose(-12);
      put(v); }

    { V v("TUBA", 22, 5);              // a tuba: the low end of the section
      v.op(1).ratio(0).level(98).vel(1).eg(60, 40, 30, 58, 99, 94, 90, 0);
      v.op(2).ratio(0).level(70).vel(3).eg(52, 40, 30, 58, 99, 88, 84, 0);
      v.op(3).ratio(0, 0, 6).level(96).vel(1).eg(60, 40, 30, 58, 99, 94, 90, 0);
      v.op(4).ratio(0, 0, 9).level(95).vel(1).eg(60, 40, 30, 58, 99, 94, 90, 0);
      v.op(5).ratio(1).level(80).vel(1).eg(60, 40, 30, 58, 99, 90, 86, 0);
      v.op(6).ratio(0).level(70).vel(3).eg(50, 40, 30, 58, 99, 88, 84, 0);
      v.lfo(32, 55, 4, 0, 4, 3);
      v.pitchEg(75, 95, 95, 60, 46, 50, 50, 50);
      put(v); }

    { V v("STAB", 22, 7);              // an orchestra hit: everything at once, then gone
      v.op(1).ratio(1).level(97).vel(2).eg(99, 50, 35, 60, 99, 70, 0, 0);
      v.op(2).ratio(1).level(84).vel(4).eg(99, 55, 40, 60, 99, 60, 0, 0);
      v.op(3).ratio(1, 0, 4).level(96).vel(2).eg(99, 50, 35, 60, 99, 70, 0, 0);
      v.op(4).ratio(1, 0, 10).level(96).vel(2).eg(99, 50, 35, 60, 99, 70, 0, 0);
      v.op(5).ratio(2).level(92).vel(2).eg(99, 50, 35, 60, 99, 70, 0, 0);
      v.op(6).ratio(1).level(84).vel(4).eg(99, 55, 40, 60, 99, 60, 0, 0);
      v.pitchEg(99, 60, 95, 60, 50, 48, 50, 50);
      put(v); }

    { V v("BASSOON", 7, 5);            // a double reed in the bass: buzzing, a little nasal
      v.op(1).ratio(1).level(98).vel(1).eg(62, 40, 30, 58, 99, 95, 92, 0);
      v.op(2).ratio(3).level(64).vel(2).eg(65, 40, 30, 58, 99, 90, 88, 0).scale(45, 0, 20, 0, 0);
      v.op(3).ratio(2).level(86).vel(1).eg(62, 40, 30, 58, 99, 92, 90, 0);
      v.op(4).ratio(1).level(70).vel(2).eg(65, 40, 30, 58, 99, 90, 88, 0);
      v.op(5).ratio(4).level(56).eg(65, 40, 30, 58, 99, 85, 80, 0);
      v.op(6).ratio(1).level(60).eg(65, 40, 30, 58, 99, 88, 85, 0);
      v.lfo(33, 55, 5, 0, 4, 3);
      v.transpose(-12);
      put(v); }

    return c;
}

// ================================================================== BANK 4: PADS & FX

Cartridge padsFx()
{
    Cartridge c;
    int i = 0;
    auto put = [&] (const dx7::Patch& p) { c.voices[(size_t) i++] = p; };

    // a slow pad on three pairs, spread
    auto pad = [&] (const char* name, int alg, int fb, std::array<int, 6> ratios, std::array<int, 6> levels,
                    int attack, int release, int detune, int lfoSpeed, int pmd, int amd)
    {
        V v(name, alg, fb);
        const auto& a = dx7::algorithms[alg - 1];
        for (int o = 1; o <= 6; ++o)
        {
            const bool carrier = (a.carriers >> o) & 1;
            const int r = ratios[(size_t) (o - 1)];
            const int det = carrier ? (o % 2 ? 7 - detune : 7 + detune) : 7;
            if (r == 15) v.op(o).ratio(1, 50, det); else v.op(o).ratio(r, 0, det);
            v.op(o).level(levels[(size_t) (o - 1)]).ams(carrier && amd ? 1 : 0)
                   .eg(attack + (carrier ? 0 : 2), 28, 25, release, 99, 95, 92, 0);
        }
        v.lfo(lfoSpeed, 45, pmd, amd, 4, 3);
        return v;
    };

    put(pad("STRINGS 2", 2, 7, { 1, 1, 1, 1, 1, 1 }, { 96, 72, 95, 66, 60, 74 }, 42, 42, 3, 30, 7, 0));
    put(pad("SLOW STRNG", 1, 6, { 1, 1, 1, 1, 1, 1 }, { 95, 70, 95, 64, 58, 72 }, 32, 36, 4, 28, 6, 0));
    put(pad("ANALOG PAD", 5, 5, { 1, 1, 1, 1, 0, 1 }, { 94, 66, 94, 62, 90, 60 }, 40, 38, 3, 26, 4, 0));
    put(pad("GLASS PAD", 5, 2, { 1, 3, 1, 5, 2, 7 }, { 94, 62, 92, 56, 88, 50 }, 38, 36, 2, 30, 3, 0));
    put(pad("DARK PAD", 5, 3, { 0, 0, 1, 1, 0, 1 }, { 96, 58, 90, 50, 92, 52 }, 34, 34, 4, 24, 4, 0));
    put(pad("SWEEP PAD", 1, 7, { 1, 2, 1, 3, 1, 1 }, { 95, 70, 94, 66, 62, 70 }, 30, 32, 4, 18, 0, 0));
    put(pad("TREM PAD", 5, 4, { 1, 1, 1, 2, 1, 1 }, { 94, 64, 94, 58, 92, 60 }, 40, 38, 3, 42, 0, 40));
    put(pad("AIR PAD", 2, 7, { 1, 1, 2, 9, 1, 1 }, { 95, 56, 82, 54, 40, 0 }, 34, 34, 2, 30, 5, 0));
    put(pad("WARM BRASS", 22, 6, { 1, 1, 1, 1, 1, 1 }, { 96, 70, 95, 94, 92, 70 }, 44, 40, 3, 32, 4, 0));
    put(pad("VOX PAD", 2, 4, { 1, 1, 1, 1, 6, 1 }, { 96, 62, 94, 50, 48, 56 }, 40, 40, 3, 33, 6, 0));
    put(pad("ORGAN PAD", 32, 2, { 0, 1, 2, 3, 4, 1 }, { 92, 96, 88, 76, 72, 90 }, 45, 40, 2, 30, 4, 0));
    put(pad("CELLO SECT", 2, 7, { 0, 1, 0, 1, 1, 1 }, { 96, 74, 94, 66, 58, 72 }, 44, 40, 3, 30, 7, 0));

    { V v("SLOW SWEEP", 1, 7);         // the index rising for ten seconds
      v.op(1).ratio(1).level(95).eg(30, 20, 20, 30, 99, 95, 92, 0);
      v.op(2).ratio(1).level(82).eg(18, 15, 20, 30, 99, 95, 90, 0);
      v.op(3).ratio(1, 0, 10).level(94).eg(30, 20, 20, 30, 99, 95, 92, 0);
      v.op(4).ratio(1).level(76).eg(16, 15, 20, 30, 99, 95, 90, 0);
      v.op(5).ratio(2).level(66).eg(16, 15, 20, 30, 99, 95, 90, 0);
      v.op(6).ratio(1).level(72).eg(16, 15, 20, 30, 99, 95, 90, 0);
      v.lfo(25, 50, 4, 0, 0, 3);
      put(v); }

    { V v("CRYSTALS", 5, 0);           // falling shards of glass, an echo of each
      v.op(1).ratio(3).level(97).vel(2).rate(3).eg(99, 30, 25, 30, 99, 50, 0, 0);
      v.op(2).ratio(7, 30).level(66).vel(4).rate(3).eg(99, 40, 30, 35, 99, 30, 0, 0);
      v.op(3).ratio(5, 0, 9).level(86).vel(2).rate(3).eg(99, 32, 25, 30, 99, 40, 0, 0);
      v.op(4).ratio(11, 50).level(56).vel(4).rate(3).eg(99, 45, 30, 35, 99, 20, 0, 0);
      v.op(5).ratio(1).level(84).ams(3).eg(45, 25, 25, 30, 99, 80, 70, 0);
      v.op(6).ratio(2).level(56).eg(45, 25, 25, 30, 99, 80, 70, 0);
      v.lfo(55, 0, 0, 40, 5, 0);
      put(v); }

    { V v("WIND", 1, 7);               // noise from the feedback, filtered by the stack
      v.op(1).fixed(2, 60).level(90).ams(2).eg(25, 20, 20, 25, 99, 90, 85, 0);
      v.op(2).fixed(3, 20).level(99).eg(25, 20, 20, 25, 99, 99, 99, 0);
      v.op(3).fixed(2, 90).level(88).ams(3).eg(30, 20, 20, 25, 99, 85, 80, 0);
      v.op(4).fixed(3, 40).level(99).eg(99, 99, 99, 25, 99, 99, 99, 0);
      v.op(5).fixed(3, 70).level(99).eg(99, 99, 99, 25, 99, 99, 99, 0);
      v.op(6).fixed(3, 90).level(99).eg(99, 99, 99, 25, 99, 99, 99, 0);
      v.lfo(15, 0, 0, 55, 5, 0);
      put(v); }

    { V v("SURF", 1, 7);               // waves on the shore, rolling in and out
      v.op(1).fixed(3, 0).level(92).ams(3).eg(30, 25, 20, 25, 99, 90, 85, 0);
      v.op(2).fixed(3, 50).level(99).eg(99, 99, 99, 25, 99, 99, 99, 0);
      v.op(3).fixed(2, 70).level(84).ams(3).eg(30, 25, 20, 25, 99, 90, 85, 0);
      v.op(4).fixed(3, 30).level(99).eg(99, 99, 99, 25, 99, 99, 99, 0);
      v.op(5).fixed(3, 60).level(99).eg(99, 99, 99, 25, 99, 99, 99, 0);
      v.op(6).fixed(3, 80).level(99).eg(99, 99, 99, 25, 99, 99, 99, 0);
      v.lfo(8, 0, 0, 70, 0, 0);
      put(v); }

    { V v("THUNDER", 1, 7);            // a low rumble, then a long roll away
      v.op(1).fixed(1, 80).level(99).vel(3).eg(85, 25, 20, 25, 99, 70, 0, 0);
      v.op(2).fixed(2, 50).level(99).eg(99, 30, 20, 25, 99, 80, 0, 0);
      v.op(3).fixed(2, 20).level(90).vel(3).eg(80, 22, 20, 25, 99, 70, 0, 0);
      v.op(4).fixed(2, 90).level(99).eg(99, 99, 99, 25, 99, 99, 99, 0);
      v.op(5).fixed(3, 30).level(99).eg(99, 99, 99, 25, 99, 99, 99, 0);
      v.op(6).fixed(3, 60).level(99).eg(99, 99, 99, 25, 99, 99, 99, 0);
      put(v); }

    { V v("LASER", 1, 5);              // the pitch envelope dropping five octaves
      v.op(1).ratio(1).level(99).eg(99, 40, 30, 70, 99, 0, 0, 0);
      v.op(2).ratio(2).level(80).eg(99, 40, 30, 70, 99, 0, 0, 0);
      v.op(3).ratio(1, 0, 9).level(94).eg(99, 40, 30, 70, 99, 0, 0, 0);
      v.op(4).ratio(3).level(76).eg(99, 40, 30, 70, 99, 0, 0, 0);
      v.op(5).ratio(1).level(66).eg(99, 40, 30, 70, 99, 0, 0, 0);
      v.op(6).ratio(1).level(66).eg(99, 40, 30, 70, 99, 0, 0, 0);
      v.pitchEg(99, 50, 95, 99, 99, 0, 0, 50);
      put(v); }

    { V v("UFO", 2, 6);                // a hovering, warbling saucer
      v.op(1).ratio(1).level(95).ams(1).eg(50, 30, 30, 40, 99, 95, 92, 0);
      v.op(2).ratio(1, 50).level(80).eg(50, 30, 30, 40, 99, 95, 92, 0);
      v.op(3).ratio(2, 0, 10).level(90).ams(1).eg(50, 30, 30, 40, 99, 95, 92, 0);
      v.op(4).ratio(3, 70).level(72).eg(50, 30, 30, 40, 99, 95, 92, 0);
      v.op(5).ratio(1).level(68).eg(50, 30, 30, 40, 99, 95, 92, 0);
      v.op(6).ratio(5).level(66).eg(50, 30, 30, 40, 99, 95, 92, 0);
      v.lfo(70, 0, 60, 20, 4, 6);
      put(v); }

    { V v("RAINDROPS", 5, 0);          // random pitches, a drop on every LFO step
      v.op(1).ratio(4).level(96).ams(3).eg(99, 99, 99, 50, 99, 99, 99, 0);
      v.op(2).ratio(9).level(58).eg(99, 99, 99, 50, 99, 99, 99, 0);
      v.op(3).ratio(6, 0, 9).level(84).ams(3).eg(99, 99, 99, 50, 99, 99, 99, 0);
      v.op(4).ratio(1).level(50).eg(99, 99, 99, 50, 99, 99, 99, 0);
      v.op(5).ratio(1).level(0);
      v.op(6).ratio(1).level(0);
      v.lfo(68, 0, 50, 99, 5, 7);
      put(v); }

    { V v("ALARM", 1, 7);              // a siren, the LFO sweeping a whole octave
      v.op(1).ratio(1).level(97).eg(99, 99, 99, 70, 99, 99, 99, 0);
      v.op(2).ratio(1).level(76).eg(99, 99, 99, 70, 99, 99, 99, 0);
      v.op(3).ratio(1).level(0);
      v.op(4).ratio(1).level(0);
      v.op(5).ratio(1).level(0);
      v.op(6).ratio(1).level(0);
      v.lfo(40, 0, 99, 0, 0, 7);
      put(v); }

    { V v("METAL HIT", 1, 7);          // a struck sheet of metal, clanging and fading
      v.op(1).ratio(1).level(99).vel(3).rate(2).eg(99, 30, 25, 35, 99, 60, 0, 0);
      v.op(2).ratio(1, 41).level(90).vel(4).rate(2).eg(99, 35, 25, 35, 99, 60, 0, 0);
      v.op(3).ratio(2, 76).level(94).vel(3).rate(2).eg(99, 32, 25, 35, 99, 60, 0, 0);
      v.op(4).ratio(4, 13).level(84).vel(4).rate(2).eg(99, 40, 30, 35, 99, 50, 0, 0);
      v.op(5).ratio(6, 33).level(80).rate(2).eg(99, 45, 30, 35, 99, 40, 0, 0);
      v.op(6).ratio(9, 71).level(80).rate(2).eg(99, 50, 35, 35, 99, 30, 0, 0);
      put(v); }

    { V v("GONG", 5, 0);               // a tam-tam: a slow bloom of partials
      v.op(1).ratio(1).level(99).vel(2).rate(1).eg(80, 20, 20, 25, 99, 70, 0, 0);
      v.op(2).ratio(1, 41).level(84).vel(4).rate(1).eg(60, 22, 20, 25, 99, 85, 0, 0);
      v.op(3).ratio(0, 50, 9).level(94).vel(2).rate(1).eg(80, 20, 20, 25, 99, 70, 0, 0);
      v.op(4).ratio(2, 33).level(80).vel(4).rate(1).eg(55, 22, 20, 25, 99, 85, 0, 0);
      v.op(5).ratio(2, 70).level(86).vel(2).rate(1).eg(80, 22, 20, 25, 99, 60, 0, 0);
      v.op(6).ratio(4, 61).level(74).vel(4).rate(1).eg(55, 25, 20, 25, 99, 75, 0, 0);
      v.transpose(-12);
      put(v); }

    { V v("BIG DRUM", 1, 6);           // a deep tom with the head's pitch dropping
      v.op(1).ratio(1).level(99).vel(3).rate(2).eg(99, 45, 35, 50, 99, 0, 0, 0);
      v.op(2).ratio(1, 50).level(80).vel(5).rate(2).eg(99, 60, 40, 50, 99, 0, 0, 0);
      v.op(3).fixed(3, 50).level(80).vel(3).eg(99, 75, 50, 60, 99, 0, 0, 0);
      v.op(4).fixed(3, 80).level(99).eg(99, 99, 99, 60, 99, 99, 99, 0);
      v.op(5).fixed(3, 90).level(99).eg(99, 99, 99, 60, 99, 99, 99, 0);
      v.op(6).fixed(3, 95).level(99).eg(99, 99, 99, 60, 99, 99, 99, 0);
      v.pitchEg(99, 50, 95, 60, 58, 48, 50, 50);
      v.transpose(-12);
      put(v); }

    { V v("SNARE", 1, 7);              // a tone and a burst of noise from the feedback stack
      v.op(1).ratio(1).level(92).vel(3).rate(3).eg(99, 65, 50, 60, 99, 0, 0, 0);
      v.op(2).ratio(1, 50).level(70).vel(4).rate(3).eg(99, 70, 50, 60, 99, 0, 0, 0);
      v.op(3).fixed(3, 40).level(94).vel(3).eg(99, 62, 50, 60, 99, 0, 0, 0);
      v.op(4).fixed(3, 80).level(99).eg(99, 99, 99, 60, 99, 99, 99, 0);
      v.op(5).fixed(3, 90).level(99).eg(99, 99, 99, 60, 99, 99, 99, 0);
      v.op(6).fixed(3, 95).level(99).eg(99, 99, 99, 60, 99, 99, 99, 0);
      v.pitchEg(99, 70, 95, 60, 54, 50, 50, 50);
      put(v); }

    { V v("HI-HAT", 1, 7);             // a closed hat: metallic noise, very short
      v.op(1).fixed(3, 80).level(96).vel(3).eg(99, 80, 60, 75, 99, 0, 0, 0);
      v.op(2).fixed(3, 95).level(99).eg(99, 99, 99, 75, 99, 99, 99, 0);
      v.op(3).fixed(3, 60).level(90).vel(3).eg(99, 82, 60, 75, 99, 0, 0, 0);
      v.op(4).fixed(3, 87).level(99).eg(99, 99, 99, 75, 99, 99, 99, 0);
      v.op(5).fixed(3, 93).level(99).eg(99, 99, 99, 75, 99, 99, 99, 0);
      v.op(6).fixed(3, 98).level(99).eg(99, 99, 99, 75, 99, 99, 99, 0);
      put(v); }

    { V v("BIRDS", 5, 0);              // chirping: fast sweeps up, in random bursts
      v.op(1).ratio(8).level(94).ams(3).eg(99, 99, 99, 60, 99, 99, 99, 0);
      v.op(2).ratio(1).level(60).eg(99, 99, 99, 60, 99, 99, 99, 0);
      v.op(3).ratio(10, 0, 9).level(86).ams(3).eg(99, 99, 99, 60, 99, 99, 99, 0);
      v.op(4).ratio(1).level(56).eg(99, 99, 99, 60, 99, 99, 99, 0);
      v.op(5).ratio(1).level(0);
      v.op(6).ratio(1).level(0);
      v.lfo(80, 0, 70, 99, 5, 7);
      put(v); }

    { V v("HEARTBEAT", 1, 0);          // a low double thump
      v.op(1).ratio(0).level(99).vel(3).eg(99, 55, 40, 60, 99, 0, 0, 0);
      v.op(2).ratio(1).level(60).vel(3).eg(99, 70, 50, 60, 99, 0, 0, 0);
      v.op(3).ratio(0, 50).level(80).eg(99, 60, 45, 60, 99, 0, 0, 0);
      v.op(4).ratio(1).level(0);
      v.op(5).ratio(1).level(0);
      v.op(6).ratio(1).level(0);
      v.pitchEg(99, 60, 95, 60, 56, 50, 50, 50);
      v.transpose(-24);
      put(v); }

    { V v("STEAM", 1, 7);              // a release valve: hiss rising to a whistle
      v.op(1).fixed(3, 70).level(94).eg(60, 30, 25, 50, 99, 90, 80, 0);
      v.op(2).fixed(3, 90).level(99).eg(99, 99, 99, 50, 99, 99, 99, 0);
      v.op(3).ratio(8).level(80).eg(25, 30, 25, 50, 99, 90, 80, 0);
      v.op(4).fixed(3, 20).level(70).eg(20, 30, 25, 50, 99, 80, 70, 0);
      v.op(5).fixed(3, 95).level(99).eg(99, 99, 99, 50, 99, 99, 99, 0);
      v.op(6).fixed(3, 99).level(99).eg(99, 99, 99, 50, 99, 99, 99, 0);
      put(v); }

    { V v("ECHO VOX", 5, 2);           // a far-off voice, a ghost of a choir
      v.op(1).ratio(1, 0, 5).level(95).ams(1).eg(30, 25, 25, 30, 99, 95, 92, 0);
      v.op(2).ratio(1).level(60).eg(32, 25, 25, 30, 99, 92, 90, 0);
      v.op(3).ratio(1, 0, 9).level(94).ams(1).eg(30, 25, 25, 30, 99, 95, 92, 0);
      v.op(4).ratio(1).level(52).eg(32, 25, 25, 30, 99, 92, 90, 0);
      v.op(5).ratio(5).level(64).ams(1).eg(30, 25, 25, 30, 99, 85, 80, 0);
      v.op(6).ratio(1).level(48).eg(32, 25, 25, 30, 99, 92, 90, 0);
      v.lfo(30, 30, 5, 18, 4, 3);
      put(v); }

    { V v("FM DRONE", 1, 7);           // a slowly beating, evolving drone
      v.op(1).ratio(0, 0, 4).level(95).eg(25, 20, 20, 25, 99, 95, 92, 0);
      v.op(2).ratio(1, 1).level(76).ams(2).eg(20, 20, 20, 25, 99, 92, 90, 0);
      v.op(3).ratio(0, 0, 10).level(94).eg(25, 20, 20, 25, 99, 95, 92, 0);
      v.op(4).ratio(1, 2).level(70).ams(3).eg(20, 20, 20, 25, 99, 92, 90, 0);
      v.op(5).ratio(2, 1).level(62).eg(20, 20, 20, 25, 99, 92, 90, 0);
      v.op(6).ratio(3).level(66).eg(20, 20, 20, 25, 99, 92, 90, 0);
      v.lfo(10, 0, 3, 50, 0, 2);
      put(v); }

    { V v("INIT VOICE", 1, 0);         // a blank to start from
      v.op(1).ratio(1).level(99).eg(99, 99, 99, 99, 99, 99, 99, 0);
      put(v); }

    return c;
}

} // namespace

namespace banks
{

int count() { return 4; }

std::string name(int bank)
{
    static const char* names[] = { "CLASSIC", "KEYS & MALLETS", "BASS & LEAD", "PADS & FX" };
    return names[bank < 0 ? 0 : (bank > 3 ? 3 : bank)];
}

const dx7::Cartridge& get(int bank)
{
    static const dx7::Cartridge all[4] = { classic(), keys(), bassLead(), padsFx() };
    return all[bank < 0 ? 0 : (bank > 3 ? 3 : bank)];
}

} // namespace banks
