#pragma once

#include <cstddef>
#include <cstdint>
#include <hardware/divider.h>

#include "pwm_config.h"
#include "interp_config.h"

// --- beat --------------------------------------------------------------------

constexpr static uint32_t SAMPLE_LENGTH = 256;
constexpr static uint32_t freq_sample_step(float freq, float rate) {
    return (SAMPLE_LENGTH / ((1 / freq) * rate)) * (1 << 16);
}

[[gnu::noinline]] inline int16_t sample_triangle(uint8_t duty, uint8_t offset, int16_t volume) {
    if (offset < duty) return hw_divider_u32_quotient(offset * (2*volume), duty) - volume;
    else return hw_divider_u32_quotient((SAMPLE_LENGTH - offset) * (2*volume), (SAMPLE_LENGTH - duty)) - volume;
}

constexpr static uint32_t sample_steps[] = {
    freq_sample_step(92.36641221374045, SAMPLE_RATE),
    freq_sample_step(184.7328244274809, SAMPLE_RATE),
    freq_sample_step(110, SAMPLE_RATE),
    freq_sample_step(220, SAMPLE_RATE)
};
struct beat_state {
    constexpr static uint32_t baseduty = 32;
    constexpr static uint32_t lfofreq = 1;
    constexpr static uint32_t lfosize = 32;
    constexpr static uint32_t max_volume = 40;
    constexpr static uint32_t max_pansize = 24;

    uint32_t t = 0, t2 = 0;
    uint32_t fbase = 0;
    uint32_t volume = 0;

    uint32_t pan = 0;
    constexpr static uint32_t panstep = freq_sample_step(0.1, SAMPLE_RATE);
    uint32_t pansize = 0;

    uint32_t ctr = 0, ctrstep = sample_steps[0];
    uint32_t lfo = 0;
    constexpr static uint32_t lfostep = freq_sample_step(lfofreq, SAMPLE_RATE);

    int8_t sample(uint32_t pan_offset) {
        uint8_t duty = baseduty + sample_triangle(128, (uint8_t)(lfo >> 16), lfosize);
        uint8_t vol = volume + sample_triangle(128, (uint8_t)((pan >> 16) + pan_offset), pansize);
        int8_t sample = sample_triangle(duty, (uint8_t)(ctr >> 16), vol);
        return sample;
    }

    void update() {
        ctr += ctrstep;
        if (ctr > (SAMPLE_LENGTH << 16)) ctr -= SAMPLE_LENGTH << 16;
        lfo += lfostep;
        if (lfo > (SAMPLE_LENGTH << 16)) lfo -= SAMPLE_LENGTH << 16;
        pan += panstep;
        if (pan > (SAMPLE_LENGTH << 16)) pan -= SAMPLE_LENGTH << 16;

        if ((t & ((1 << 12) - 1)) == 0) {
            if (volume < max_volume) volume++;
            else if (pansize < max_pansize) pansize++;
        }

        if ((t2 & ((1 << 18) - 1)) == 0) { t = 0; if (t2 & (1 << 18)) {
            fbase = 2;
        } else {
            fbase = 0;
        } }
        if ((t & ((1 << 13) - 1)) == 0) { lfo = 0; ctr = 0;
            if ((t >> 13) % 3 != 2) {
                ctrstep = sample_steps[fbase + 0];
            } else {
                ctrstep = sample_steps[fbase + 1];
            }
        }
        t++;
        t2++;
    }
};

// --- refill ------------------------------------------------------------------

inline beat_state b_state;
[[gnu::always_inline]] inline void refill_sample_buffer() {
    uint32_t * sample_buffer = sample_buffers[current_sample_buffer];

    beat_state state = b_state;
    for (size_t i = 0; i < NUM_SAMPLES; i++) {
        uint16_t sample_l = 128 + state.sample(0);
        uint16_t sample_r = 128 + state.sample(128);
        state.update();
        sample_buffer[i] = (sample_l << 0) | (sample_r << 16);
    }
    b_state = state;
}
