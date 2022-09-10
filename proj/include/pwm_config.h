#pragma once

#include <cstddef>
#include <cstdint>
#include <usdk/gpio.h>
#include <usdk/pwm.h>
#include <usdk/dma.h>

#include "dma_channels.h"

#define PWM_RIGHT_PIN 27
#define PWM_LEFT_PIN 28

// --- pwm config --------------------------------------------------------------

constexpr uint32_t right_slice = usdk::pwm_gpio_to_slice(PWM_RIGHT_PIN);
constexpr uint32_t left_slice = usdk::pwm_gpio_to_slice(PWM_LEFT_PIN);
constexpr uint32_t right_chan = usdk::pwm_gpio_to_channel(PWM_RIGHT_PIN);
constexpr uint32_t left_chan = usdk::pwm_gpio_to_channel(PWM_LEFT_PIN);
constexpr uint32_t samplerate_slice = 0;
static_assert(samplerate_slice != right_slice);
static_assert(samplerate_slice != left_slice);

constexpr static uint32_t SAMPLE_RATE = 48000;

constexpr auto pwm_audio_config() {
    using namespace usdk::iovec;

    // samplerate freq = 125MHz / (162 + 194/256) / 16 == 48KHz
    // tuned clock divider to actual speaker output: (162 + 11/256)
    usdk::pwm_config samplerate_cfg;
    static_assert(SAMPLE_RATE == 48000);
    samplerate_cfg.set_clkdiv_int_frac(162, 11);
    samplerate_cfg.set_wrap(15);

    usdk::pwm_config pwm_cfg;
    pwm_cfg.set_clkdiv_int_frac(1, 0);
    pwm_cfg.set_wrap(0xff);

    return usdk::array_concat(
        io_gpio_set_consecutive_functions(PWM_RIGHT_PIN, 2, GPIO_FUNC_PWM),
        io_pwm_init(samplerate_slice, samplerate_cfg, false),
        io_pwm_init(right_slice, pwm_cfg, false),
        io_pwm_init(left_slice, pwm_cfg, false)
    );
}

// --- sample buffers config ---------------------------------------------------

constexpr static uint32_t NUM_SAMPLES = 512;

// Sample Buffer
uint32_t sample_buffers[2][NUM_SAMPLES];
uint32_t current_sample_buffer = 0;

alignas(8) uint32_t * const sample_buffer_descriptors[] = {
    sample_buffers[0],
    sample_buffers[1]
};
// TODO: enable GNU extensions to silence this warning (and maybe switch to C++20)
constexpr static uint32_t sample_buffer_descriptors_ring_size = 3;
static_assert(alignof(sample_buffer_descriptors) == sizeof (sample_buffer_descriptors));
static_assert(alignof(sample_buffer_descriptors) == 1 << sample_buffer_descriptors_ring_size);

constexpr auto dma_audio_config() {
    using namespace usdk::iovec;

    usdk::dma_channel_config sample_right_cfg(sample_right_chan);
    sample_right_cfg.set_transfer_data_size(DMA_SIZE_32);
    sample_right_cfg.set_read_increment(true);
    sample_right_cfg.set_write_increment(false);
    sample_right_cfg.set_dreq(usdk::pwm_get_dreq(samplerate_slice));
    sample_right_cfg.set_chain_to(reconf_right_chan);

    usdk::dma_channel_config sample_left_cfg(sample_left_chan);
    sample_left_cfg.set_transfer_data_size(DMA_SIZE_32);
    sample_left_cfg.set_read_increment(true);
    sample_left_cfg.set_write_increment(false);
    sample_left_cfg.set_dreq(usdk::pwm_get_dreq(samplerate_slice));
    sample_left_cfg.set_chain_to(reconf_left_chan);

    usdk::dma_channel_config reconf_right_cfg(reconf_right_chan);
    reconf_right_cfg.set_transfer_data_size(DMA_SIZE_32);
    reconf_right_cfg.set_read_increment(true);
    reconf_right_cfg.set_write_increment(false);
    reconf_right_cfg.set_ring(false, sample_buffer_descriptors_ring_size);
    reconf_right_cfg.set_chain_to(sample_right_chan);

    usdk::dma_channel_config reconf_left_cfg(reconf_left_chan);
    reconf_left_cfg.set_transfer_data_size(DMA_SIZE_32);
    reconf_left_cfg.set_read_increment(true);
    reconf_left_cfg.set_write_increment(false);
    reconf_left_cfg.set_ring(false, sample_buffer_descriptors_ring_size);
    reconf_left_cfg.set_chain_to(sample_left_chan);

    return usdk::array_concat(
        io_dma_channel_configure(sample_right_chan, sample_right_cfg,
            // TODO: fix this so GCC likes it in constexpr
            // &pwm_hw->slice[right_slice].cc,
            PWM_BASE + right_slice * sizeof (pwm_slice_hw_t) + PWM_CH0_CC_OFFSET,
            sample_buffers[0],
            NUM_SAMPLES,
            false
        ),
        io_dma_channel_configure(sample_left_chan, sample_left_cfg,
            PWM_BASE + left_slice * sizeof (pwm_slice_hw_t) + PWM_CH0_CC_OFFSET,
            sample_buffers[0],
            NUM_SAMPLES,
            false
        ),
        io_dma_channel_configure(reconf_right_chan, reconf_right_cfg,
            // TODO: fix this so GCC likes it in constexpr
            // &dma_hw->ch[sample_right_chan].read_addr,
            DMA_BASE + sample_right_chan * sizeof (dma_channel_hw_t) + DMA_CH0_READ_ADDR_OFFSET,
            sample_buffer_descriptors,
            1,
            false
        ),
        io_dma_channel_configure(reconf_left_chan, reconf_left_cfg,
            DMA_BASE + sample_left_chan * sizeof (dma_channel_hw_t) + DMA_CH0_READ_ADDR_OFFSET,
            sample_buffer_descriptors,
            1,
            false
        ),
        io_dma_channel_set_irq1_mask_enabled((1 << sample_right_chan), true)
    );
}
