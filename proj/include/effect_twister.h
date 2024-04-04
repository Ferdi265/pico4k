#pragma once

#include <cstddef>
#include <cstdint>
#include <pico/platform.h>
#include <hardware/sync.h>
#include <hardware/divider.h>
#include <usdk/util/array.h>

#include "memmove.h"
#include "font_config.h"
#include "vga_config.h"
#include "interp_config.h"

inline uint32_t cur_twister_buffer = 0;
inline uint32_t cur_twister_frame = 0;

const uint16_t twister_colors[] = { 0x0001, 0x0040, 0x0800, 0x0041 };

alignas(4) inline uint16_t batch_twister_cmds[2][32];

[[gnu::always_inline]] inline void render_twister() {
    uint32_t y = twister_batch_index;
    uint32_t t = cur_twister_frame;

    uint16_t * begin = batch_twister_cmds[cur_twister_buffer];
    uint16_t * data = begin;

    auto div = [](uint32_t a, uint32_t b){ return hw_divider_quotient_s32(a, b); };

    uint32_t a = cos(div(sin(t * 16 + y * 8), 5)) + cos(-t * 32 + y * 2) / 2;
    uint32_t i = (0x4000 - ((a + 0x3800) % 0x4000)) / 0x1000;
    int32_t x1 = 320 + cos(a + i * 0x1000) * 160 / 0x4000;
    int32_t x2 = 320 + cos(a + i * 0x1000 + 0x1000) * 160 / 0x4000;
    int32_t x3 = 320 - cos(a + i * 0x1000) * 160 / 0x4000;

    auto emit_inline = [](uint16_t * data, uint32_t type, uint32_t len, uint32_t color) __attribute((always_inline)) {
        switch (type) {
            case 0:
                return data;
            case 1:
                *data++ = VP_SINGLE;
                *data++ = color;
                return data;
            default:
                *data++ = VP_RLE;
                *data++ = len - 1;
                *data++ = color;
                return data;
        }
    };

    auto emit_noinline = [=](uint16_t * data, uint32_t len, uint32_t color) __attribute((noinline)) {
        return emit_inline(data, len, len, color);
    };

    auto emit_single = [&](uint32_t color) __attribute__((always_inline)) {
        data = emit_inline(data, 1, 1, color);
    };

    auto emit_rle = [&](uint32_t len, uint32_t color) __attribute__((always_inline)) {
        data = emit_inline(data, 2, len, color);
    };

    auto emit = [&](uint32_t len, uint32_t color) __attribute((always_inline)) {
        data = emit_noinline(data, len, color);
    };

    auto calc_color = [](uint32_t diff, uint32_t color) __attribute__((always_inline)) {
        uint32_t val = (diff >> 3) + 1;
        return color * val;
    };

    emit_rle(x3, 0x0000);
    if (x2 - x3 != 0) emit(x2 - x3 - 1, calc_color(x2 - x3, twister_colors[i]));
    emit_single(0x0020);
    if (x1 - x2 != 0) emit(x1 - x2 - 1, calc_color(x1 - x2, twister_colors[(i + 3) % 4]));
    emit_single(0x0000);

    if ((data - begin) % 2 == 0) {
        *data++ = VP_ALIGN;
    }

    if (y < 479) {
        *data++ = VP_HSYNC;
        twister_batch_index++;
    } else {
        *data++ = VP_VSYNC;
        twister_batch_index = 0;
        cur_twister_frame++;
    }

    next_twister_batch.length = (data - begin) / 2;
    next_twister_batch.data = begin;

    cur_twister_buffer ^= 1;
    twister_batch_ready = true;
}

[[gnu::always_inline, noreturn]] inline void twister_draw_loop() {
    while (true) {
        while (twister_batch_ready || twister_batch_index >= 480) { asm("" ::: "memory"); __wfi(); /* spin */ };
        render_twister();
    }
}

