// Mz7 - Created by Simon Moscrop
// Free software under the GNU AGPL v3; see LICENSE.

#pragma once

#include <cstdint>

namespace dx7
{

//
// The 32 algorithms, as the DX7's front panel draws them.
//
// For each operator, the operators that modulate it, as a mask (bit n is OPn); which
// operators are carriers; and the feedback path - from the operator whose output is fed
// back, to the one it modulates. Mostly that is one operator feeding itself, but three
// algorithms close a loop through a stack instead: 4 (OP4 back to OP6), 6 (OP5 back to OP6)
// and none other. Those are real loops on the chip, a feedback register written by the
// bottom of the stack and read by the top, and are modelled as such - a common emulator
// shortcut makes them OP6 feeding itself, which is a different sound.
//
// Every modulator has a higher number than what it modulates, so computing OP6 down to OP1,
// as the chip does, always finds a modulator's output ready.
//
struct Algorithm
{
    uint8_t mods[7];     // [op] = mask of the operators modulating op (index 0 unused)
    uint8_t carriers;    // mask
    uint8_t fbFrom, fbTo;
};

namespace detail
{
    constexpr uint8_t m(int a, int b = 0, int c = 0)
    {
        return (uint8_t) ((a ? 1 << a : 0) | (b ? 1 << b : 0) | (c ? 1 << c : 0));
    }
}

//                       OP1                OP2         OP3                OP4                OP5         OP6   carriers                     feedback
inline constexpr Algorithm algorithms[32] = {
    /*  1 */ { { 0, detail::m(2),        0,          detail::m(4),      detail::m(5),      detail::m(6), 0 }, detail::m(1, 3),                6, 6 },
    /*  2 */ { { 0, detail::m(2),        0,          detail::m(4),      detail::m(5),      detail::m(6), 0 }, detail::m(1, 3),                2, 2 },
    /*  3 */ { { 0, detail::m(2),        detail::m(3), 0,               detail::m(5),      detail::m(6), 0 }, detail::m(1, 4),                6, 6 },
    /*  4 */ { { 0, detail::m(2),        detail::m(3), 0,               detail::m(5),      detail::m(6), 0 }, detail::m(1, 4),                4, 6 },
    /*  5 */ { { 0, detail::m(2),        0,          detail::m(4),      0,                 detail::m(6), 0 }, detail::m(1, 3, 5),             6, 6 },
    /*  6 */ { { 0, detail::m(2),        0,          detail::m(4),      0,                 detail::m(6), 0 }, detail::m(1, 3, 5),             5, 6 },
    /*  7 */ { { 0, detail::m(2),        0,          detail::m(4, 5),   0,                 detail::m(6), 0 }, detail::m(1, 3),                6, 6 },
    /*  8 */ { { 0, detail::m(2),        0,          detail::m(4, 5),   0,                 detail::m(6), 0 }, detail::m(1, 3),                4, 4 },
    /*  9 */ { { 0, detail::m(2),        0,          detail::m(4, 5),   0,                 detail::m(6), 0 }, detail::m(1, 3),                2, 2 },
    /* 10 */ { { 0, detail::m(2),        detail::m(3), 0,               detail::m(5, 6),   0,            0 }, detail::m(1, 4),                3, 3 },
    /* 11 */ { { 0, detail::m(2),        detail::m(3), 0,               detail::m(5, 6),   0,            0 }, detail::m(1, 4),                6, 6 },
    /* 12 */ { { 0, detail::m(2),        0,          detail::m(4, 5, 6), 0,                0,            0 }, detail::m(1, 3),                2, 2 },
    /* 13 */ { { 0, detail::m(2),        0,          detail::m(4, 5, 6), 0,                0,            0 }, detail::m(1, 3),                6, 6 },
    /* 14 */ { { 0, detail::m(2),        0,          detail::m(4),      detail::m(5, 6),   0,            0 }, detail::m(1, 3),                6, 6 },
    /* 15 */ { { 0, detail::m(2),        0,          detail::m(4),      detail::m(5, 6),   0,            0 }, detail::m(1, 3),                2, 2 },
    /* 16 */ { { 0, detail::m(2, 3, 5),  0,          detail::m(4),      0,                 detail::m(6), 0 }, detail::m(1),                   6, 6 },
    /* 17 */ { { 0, detail::m(2, 3, 5),  0,          detail::m(4),      0,                 detail::m(6), 0 }, detail::m(1),                   2, 2 },
    /* 18 */ { { 0, detail::m(2, 3, 4),  0,          0,                 detail::m(5),      detail::m(6), 0 }, detail::m(1),                   3, 3 },
    /* 19 */ { { 0, detail::m(2),        detail::m(3), 0,               detail::m(6),      detail::m(6), 0 }, detail::m(1, 4, 5),             6, 6 },
    /* 20 */ { { 0, detail::m(3),        detail::m(3), 0,               detail::m(5, 6),   0,            0 }, detail::m(1, 2, 4),             3, 3 },
    /* 21 */ { { 0, detail::m(3),        detail::m(3), 0,               detail::m(6),      detail::m(6), 0 }, (uint8_t) (detail::m(1, 2, 4) | detail::m(5)), 3, 3 },
    /* 22 */ { { 0, detail::m(2),        0,          detail::m(6),      detail::m(6),      detail::m(6), 0 }, (uint8_t) (detail::m(1, 3, 4) | detail::m(5)), 6, 6 },
    /* 23 */ { { 0, 0,                   detail::m(3), 0,               detail::m(6),      detail::m(6), 0 }, (uint8_t) (detail::m(1, 2, 4) | detail::m(5)), 6, 6 },
    /* 24 */ { { 0, 0,                   0,          detail::m(6),      detail::m(6),      detail::m(6), 0 }, (uint8_t) (detail::m(1, 2, 3) | detail::m(4, 5)), 6, 6 },
    /* 25 */ { { 0, 0,                   0,          0,                 detail::m(6),      detail::m(6), 0 }, (uint8_t) (detail::m(1, 2, 3) | detail::m(4, 5)), 6, 6 },
    /* 26 */ { { 0, 0,                   detail::m(3), 0,               detail::m(5, 6),   0,            0 }, detail::m(1, 2, 4),             6, 6 },
    /* 27 */ { { 0, 0,                   detail::m(3), 0,               detail::m(5, 6),   0,            0 }, detail::m(1, 2, 4),             3, 3 },
    /* 28 */ { { 0, detail::m(2),        0,          detail::m(4),      detail::m(5),      0,            0 }, detail::m(1, 3, 6),             5, 5 },
    /* 29 */ { { 0, 0,                   0,          detail::m(4),      0,                 detail::m(6), 0 }, (uint8_t) (detail::m(1, 2, 3) | detail::m(5)), 6, 6 },
    /* 30 */ { { 0, 0,                   0,          detail::m(4),      detail::m(5),      0,            0 }, (uint8_t) (detail::m(1, 2, 3) | detail::m(6)), 5, 5 },
    /* 31 */ { { 0, 0,                   0,          0,                 0,                 detail::m(6), 0 }, (uint8_t) (detail::m(1, 2, 3) | detail::m(4, 5)), 6, 6 },
    /* 32 */ { { 0, 0,                   0,          0,                 0,                 0,            0 }, (uint8_t) (detail::m(1, 2, 3) | detail::m(4, 5, 6)), 6, 6 },
};

inline int carrierCount(const Algorithm& a)
{
    int n = 0;
    for (int op = 1; op <= 6; ++op) n += (a.carriers >> op) & 1;
    return n;
}

} // namespace dx7
