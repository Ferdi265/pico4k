#pragma once

#include <cstddef>
#include <cstdint>
#include <algorithm>
#include <pico/platform.h>
#include <hardware/sync.h>
#include <usdk/util/array.h>
#include <usdk/util/mem.h>

#include "font_config.h"
#include "vga_config.h"
#include "interp_config.h"

template <size_t N>
constexpr auto scrollerfix(const char (&msg)[N]) {
    usdk::array<char, N> fixed_msg;
    usdk::copy_n(msg, N, fixed_msg.begin());

    auto it = fixed_msg.begin();
    auto end = fixed_msg.end();
    while (true) {
        auto newline = std::find(it, end, '\n');
        if (newline == end) break;

        size_t len = newline - it;
        for (size_t i = 0; i < len / 2; i++) {
            std::swap(it[len - i - 1], it[i]);
        }

        it = newline + 1;
    }

    return fixed_msg;
}

constexpr auto msg = scrollerfix(
    "A new platform for 4Ks is here!\n"
    "\n"
    "264KB RAM\n"
    "125MHz Dual Core ARM CPU\n"
    "Crazy PIO Coprocessor\n"
    "DMA Tricks\n"
    "Bit-Banged everything\n"
    "\n"
    "Introducing\n"
    "A Packer\n"
    "Optimized Init Routines\n"
    "Shitty Scrollers\n"
    "\n"
    "Code by: yrlf\n"
    "\n"
    "Greetings to:\n"
    "all other firsties :D\n"
    "fgenesis\n"
    "molive\n"
    "\n\n\n\n"
);

inline const char * msgptr;
inline size_t pausectr = 0;

inline uint32_t cur_text_frame = 0;
inline char textmode_framebuffer[30][81];
inline int32_t text_sine[64];

[[gnu::always_inline]] inline void init_text() {
    memset(&framebuffer, 0xff, sizeof (pio_1bpp_framebuffer));
    for (auto& line : framebuffer) {
        line[0] = VP_1BPP_SCROLL;
        line[1] = VP_COUNT(640);
        line[2] = VP_SCROLL_OFFSET(8);
        line[4 + FB_WIDTH/16 + 0] = VP_ALIGN;
        line[4 + FB_WIDTH/16 + 1] = VP_HSYNC;
    }
    framebuffer[FB_HEIGHT - 1][4 + FB_WIDTH/16 + 1] = VP_VSYNC;

    msgptr = msg.begin();
    usdk::strcpy(&textmode_framebuffer[10][34], "P I C O 4 K");
    memset(&textmode_framebuffer[4][0], H, 81);
    memset(&textmode_framebuffer[29-4][0], H, 81);
}

[[gnu::always_inline]] inline void render_text() {
    for (size_t y = 0; y < 480; y++) {
        uint32_t t = cur_text_frame;

        uint16_t * cmds = framebuffer[y].begin();
        uint8_t * buffer = (uint8_t *)cmds;

        int32_t offset = text_sine[y % 64] >> 3;
        cmds[2] = VP_SCROLL_OFFSET(8) + ((y > 220 && y < 360) ? 7 - (t % 8) : offset);

        uint8_t * pixels = buffer + 6;
        int32_t dir_factor = ((y > 220 && y < 360) ? -1 : (y > 120 && y < 220 ? 0 : 1));
        for (size_t i = 0; i < 81; i++) {
            int32_t wobble = text_sine[2 * (i % 32)] * dir_factor;
            int32_t actual_y = clamp(y + wobble);

            uint32_t fb_line = actual_y / 16;
            uint32_t line = actual_y % 16;
            pixels[i] = ~filtered_font[textmode_framebuffer[fb_line][i]][line];
        }
    }
}

[[gnu::always_inline, noreturn]] inline void text_draw_loop() {
    uint32_t text_frame = cur_text_frame;
    while (true) {
        while (text_frame == cur_text_frame) { asm("" ::: "memory"); __wfi(); /* spin */ };
        text_frame = cur_text_frame;

        render_text();
    }
}

[[gnu::always_inline]] inline void update_text() {
    cur_text_frame++;

    uint32_t t = cur_text_frame;
    for (size_t i = 0; i < 64; i++) {
        text_sine[i] = sin(128 * t + 0x0100 * i) / 512;
    }

    if (cur_text_frame % 8 == 0) {
        usdk::memmove_noinline(&textmode_framebuffer[16][1], &textmode_framebuffer[16][0], 80);

        char c = *msgptr;
        textmode_framebuffer[16][0] = c;
        if (c != '\n' || pausectr++ > 16) {
            msgptr++;
            pausectr = 0;
        }

        if (c == '\0') msgptr = msg.begin();
    }
}
