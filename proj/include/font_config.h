#pragma once

#include <cstddef>
#include <cstdint>
#include <usdk/util/array.h>

#include "font1bpp.h"

template <size_t N>
constexpr usdk::array<usdk::array<uint8_t, 16>, 256> filter_font(const uint8_t (&font)[256][16], const uint8_t (&keep)[N]) {
    usdk::array<usdk::array<uint8_t, 16>, 256> font_copy;

    for (size_t i = 0; i < 256; i++) {
        bool allowed = false;
        for (size_t j = 0; j < N; j++) {
            if (i == keep[j]) { allowed = true; break; }
        }

        if (allowed) {
            usdk::copy_n(font[i], 16, font_copy[i].begin());
        } else {
            usdk::copy_n(font[0], 16, font_copy[i].begin());
        }
    }

    return font_copy;
}

constexpr uint8_t BL = 0xC8;
constexpr uint8_t TL = 0xC9;
constexpr uint8_t BR = 0xBC;
constexpr uint8_t TR = 0xBB;
constexpr uint8_t V = 0xBA;
constexpr uint8_t H = 0xCD;

constexpr uint8_t font_keep[] =
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz"
    "0123456789"
    " -#:!"
//    "\xC8\xC9\xBC\xBB\xBA\xCD"
    "\xCD"
;

constexpr auto filtered_font = filter_font(font_1bpp, font_keep);
