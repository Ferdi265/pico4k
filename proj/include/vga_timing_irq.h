#pragma once

#include <cstdint>
#include <usdk/pio.h>

#ifndef WANT_VBLANK_IRQ
#define WANT_VBLANK_IRQ 0
#endif

#define WANT_VBLANK_IRQ_STR __XSTRING(WANT_VBLANK_IRQ)

#ifndef VBLANK_IRQ
#define VBLANK_IRQ 26
#endif

#define isr_vblank isr_sw0

extern "C" {

struct VGAConfig {
    constexpr static uint32_t active_cols = 640;
    constexpr static uint32_t hfront_cols = 16;
    constexpr static uint32_t hsync_cols = 96;
    constexpr static uint32_t hback_cols = 48;
    constexpr static uint32_t total_cols = active_cols + hfront_cols + hsync_cols + hback_cols;
    constexpr static bool hsync_invert = true;

    constexpr static uint32_t active_lines = 480;
    constexpr static uint32_t vfront_lines = 10;
    constexpr static uint32_t vsync_lines = 2;
    constexpr static uint32_t vback_lines = 33;
    constexpr static uint32_t total_lines = active_lines + vfront_lines + vsync_lines + vback_lines;
    constexpr static bool vsync_invert = true;

    constexpr static uint32_t irq_active = 1;
    constexpr static uint32_t irq_inactive = 0;
    constexpr static uint32_t irq_ack_bits = (1 << irq_active) | (1 << irq_inactive);

    constexpr static bool H = !hsync_invert;
    constexpr static bool NH = hsync_invert;
    constexpr static bool V = !vsync_invert;
    constexpr static bool NV = vsync_invert;

    constexpr static uint32_t commands[] = {
        (NV << 31) | (NH << 30) | ((active_cols + hfront_cols - 6) << 16) | usdk::pio_encode_irq_set(false, irq_active),
        (NV << 31) | ( H << 30) | ((hsync_cols - 6) << 16) | usdk::pio_encode_nop(),
        (NV << 31) | (NH << 30) | ((hback_cols - 6) << 16) | usdk::pio_encode_nop(),
    };

    constexpr static uint32_t irq_xor = usdk::pio_encode_irq_set(false, irq_active) ^ usdk::pio_encode_irq_set(false, irq_inactive);
    constexpr static uint32_t vsync_xor = (V << 31) ^ (NV << 31);
};

void vga_timing_irq();
void vga_timing_refill(uint32_t scanline);

extern uint32_t vga_timing_scanline;

}
