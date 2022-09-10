#pragma once

#include <cstddef>
#include <cstdint>
#include <usdk/pio.h>
#include <usdk/gpio.h>
#include <usdk/dma.h>
#include <usdk/irq.h>

#include "vga_timing_irq.h"
#include "vga_timing.pio.h"
#include "vga_pixels.pio.h"
#include "vga_pins.h"

#include "dma_channels.h"

// --- pio state machines ------------------------------------------------------

constexpr uint32_t timing_sm = 0;
constexpr uint32_t pixels_sm = 1;
constexpr uint32_t text_sm = 2;

constexpr uint32_t pixels_offset = 0;
constexpr uint32_t timing_offset = 0 + usdk::array_length(vga_pixels_program_instructions);
constexpr uint32_t pio_program_end = timing_offset + usdk::array_length(vga_timing_program_instructions);
static_assert(pio_program_end < 32);

constexpr auto pio_vga_config() {
    using namespace usdk::iovec;

    constexpr usdk::array pixels_relocated = usdk::pio_program_relocate(usdk::to_array(vga_pixels_program_instructions), pixels_offset);
    constexpr usdk::array timing_relocated = usdk::pio_program_relocate(usdk::to_array(vga_timing_program_instructions), timing_offset);
    constexpr usdk::array pio_program = usdk::array_concat(pixels_relocated, timing_relocated);

    // timing
    usdk::pio_sm_config timing_cfg;
    timing_cfg.set_wrap(timing_offset + vga_timing_wrap_target, timing_offset + vga_timing_wrap);
    timing_cfg.set_clkdiv_int_frac(5, 0); // 125MHz -> 25 MHz
    timing_cfg.set_fifo_join(PIO_FIFO_JOIN_TX);
    timing_cfg.set_out_shift(true, true, 32);
    timing_cfg.set_out_pins(VGA_PINS_SYNC_BASE, 2);
    // pixels
    usdk::pio_sm_config pixels_cfg;
    pixels_cfg.set_wrap(pixels_offset + vga_pixels_wrap_target, pixels_offset + vga_pixels_wrap);
    pixels_cfg.set_clkdiv_int_frac(1, 64); // 125MHz -> 100 MHz
    pixels_cfg.set_fifo_join(PIO_FIFO_JOIN_TX);
    pixels_cfg.set_out_shift(true, true, 32);
    pixels_cfg.set_out_pins(VGA_PINS_RED_BASE, 16);
    pixels_cfg.set_out_special(true, false, VGA_PINS_ALPHA_BASE);
    // 1bpp pixels
    usdk::pio_sm_config text_cfg;
    text_cfg.set_wrap(pixels_offset + vga_pixels_wrap_target, pixels_offset + vga_pixels_wrap);
    text_cfg.set_clkdiv_int_frac(1, 64); // 125MHz -> 100 MHz
    text_cfg.set_fifo_join(PIO_FIFO_JOIN_TX);
    text_cfg.set_out_shift(true, true, 32);
    text_cfg.set_out_pins(VGA_PINS_RED_BASE, 16);
    text_cfg.set_out_special(true, true, VGA_PINS_ALPHA_BASE);

    return usdk::array_concat(
        // gpio
        io_gpio_set_consecutive_functions(VGA_PINS_RED_BASE, 18, GPIO_FUNC_PIO0),

        // load programs
        io_pio_add_relocated_program(0, pixels_offset, pio_program),

        // set pindirs
        io_pio_sm_set_consecutive_pindirs<VGA_PINS_SYNC_BASE, 2>(0, timing_sm, true),
        io_pio_sm_set_consecutive_pindirs<VGA_PINS_RED_BASE, 16>(0, pixels_sm, true),
        io_pio_sm_set_consecutive_pindirs<VGA_PINS_RED_BASE, 16>(0, text_sm, true),

        // configure pio state machines
        io_pio_sm_enable_mask(0, (1 << timing_sm) | (1 << pixels_sm) | (1 << text_sm), false),
        io_pio_sm_configure(0, timing_sm, timing_cfg),
        io_pio_sm_configure(0, pixels_sm, pixels_cfg),
        io_pio_sm_configure(0, text_sm, text_cfg),
        io_pio_sm_restart_mask(0, (1 << timing_sm) | (1 << pixels_sm) | (1 << text_sm)),
        io_pio_sm_clkdiv_restart_mask(0, (1 << timing_sm) | (1 << pixels_sm) | (1 << text_sm)),
        io_pio_sm_exec(0, timing_sm, timing_offset),
        io_pio_sm_exec(0, pixels_sm, pixels_offset),
        io_pio_sm_exec(0, text_sm, pixels_offset),

        // enable PIO irq sources
        io_pio_set_irq0_source_mask_enabled(0, (1 << pis_interrupt0) | (1 << pis_interrupt1) | (1 << pis_interrupt2), true)
    );
}

// --- 1bpp framebuffer --------------------------------------------------------

constexpr uint32_t FB_WIDTH = 640;
constexpr uint32_t FB_HEIGHT = 480;
using pio_1bpp_scanline = usdk::array<uint16_t, 4 + FB_WIDTH/16 + 2>;
using pio_1bpp_framebuffer = usdk::array<pio_1bpp_scanline, FB_HEIGHT>;

alignas(4) pio_1bpp_framebuffer framebuffer;
void * framebuffer_ptr = &framebuffer;


// --- dma channels ------------------------------------------------------------

struct dma_descriptor {
    uint32_t length;
    uint16_t * data;
};

alignas(4) uint16_t empty_twister_cmds[] = { VP_VSYNC, VP_VSYNC };

dma_descriptor cur_twister_batch = { usdk::array_length(empty_twister_cmds) / 2, empty_twister_cmds };
dma_descriptor next_twister_batch;
uint32_t twister_batch_index = 0;
volatile bool twister_batch_ready = false;

constexpr auto dma_video_config() {
    using namespace usdk::iovec;

    usdk::dma_channel_config text_data_cfg(text_data_chan);
    text_data_cfg.set_transfer_data_size(DMA_SIZE_32);
    text_data_cfg.set_read_increment(true);
    text_data_cfg.set_write_increment(false);
    text_data_cfg.set_dreq(usdk::pio_get_dreq(0, text_sm, true));
    text_data_cfg.set_chain_to(text_reconf_chan);

    usdk::dma_channel_config text_reconf_cfg(text_reconf_chan);
    text_reconf_cfg.set_transfer_data_size(DMA_SIZE_32);
    text_reconf_cfg.set_read_increment(false);
    text_reconf_cfg.set_write_increment(false);
    text_reconf_cfg.set_chain_to(text_data_chan);

    usdk::dma_channel_config pixels_data_cfg(pixels_data_chan);
    pixels_data_cfg.set_transfer_data_size(DMA_SIZE_32);
    pixels_data_cfg.set_read_increment(true);
    pixels_data_cfg.set_write_increment(false);
    pixels_data_cfg.set_dreq(usdk::pio_get_dreq(0, pixels_sm, true));

    return usdk::array_concat(
        // dma configuration
        io_dma_channel_configure(text_data_chan, text_data_cfg,
            PIO0_BASE + PIO_TXF0_OFFSET + (text_sm * 4),
            &framebuffer,
            sizeof (pio_1bpp_framebuffer) / 4,
            false
        ),
        io_dma_channel_configure(text_reconf_chan, text_reconf_cfg,
            DMA_BASE + text_data_chan * sizeof (dma_channel_hw_t) + DMA_CH0_AL3_READ_ADDR_TRIG_OFFSET,
            &framebuffer_ptr,
            1,
            false
        ),
        io_dma_channel_configure(pixels_data_chan, pixels_data_cfg,
            PIO0_BASE + PIO_TXF0_OFFSET + (pixels_sm * 4),
            empty_twister_cmds,
            usdk::array_length(empty_twister_cmds) / 2,
            false
        ),

        // enable dma irq sources
        io_dma_channel_set_irq0_mask_enabled((1 << pixels_data_chan), true)
    );
}

[[gnu::always_inline]] inline void init_vga() {
    vga_timing_refill(0);
    usdk::irq_set_priority(PIO0_IRQ_0, 0);
}
