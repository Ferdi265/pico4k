#pragma once

#include <cstddef>
#include <cstdint>

namespace usdk {

constexpr uint32_t XOSC_CLOCK_SPEED = 12 * 1000 * 1000;
constexpr uint32_t BASE_CLOCK_SPEED = 125 * 1000 * 1000;
constexpr uint32_t MIN_VCO_CLOCK_SPEED = 750 * 1000 * 1000;
constexpr uint32_t MAX_VCO_CLOCK_SPEED = 1600 * 1000 * 1000;

struct pll_config {
    uint32_t fbdiv;
    uint32_t postdiv1;
    uint32_t postdiv2;
};

constexpr bool try_find_pll_config_for(uint32_t target, uint32_t& fbdiv, uint32_t& postdiv1, uint32_t& postdiv2) {
    for (fbdiv = 320; fbdiv >= 16; fbdiv--) {
        uint32_t vco = fbdiv * XOSC_CLOCK_SPEED;
        if (vco < MIN_VCO_CLOCK_SPEED || vco > MAX_VCO_CLOCK_SPEED) continue;
        for (postdiv1 = 7; postdiv1 >= 1; postdiv1--) {
            for (postdiv2 = postdiv1; postdiv2 >= 1; postdiv2--) {
                uint32_t out = vco / (postdiv1 * postdiv2);
                if (out == target && vco % (postdiv1 * postdiv2) == 0) {
                    return true;
                }
            }
        }
    }

    return false;
}

constexpr pll_config pll_config_for(float clock_factor) {
    uint32_t sys_clock_speed = BASE_CLOCK_SPEED * clock_factor;

    uint32_t fbdiv = 0;
    uint32_t postdiv1 = 0;
    uint32_t postdiv2 = 0;
    bool found = try_find_pll_config_for(sys_clock_speed, fbdiv, postdiv1, postdiv2);
    while (!found) /* failed to determine pll config for requested clock factor */;

    pll_config cfg = {
        .fbdiv = fbdiv,
        .postdiv1 = postdiv1,
        .postdiv2 = postdiv2
    };

    return cfg;
}

struct pio_clock_divider {
    uint32_t int_div;
    uint32_t frac_div;
};

constexpr pio_clock_divider pio_clock_divider_for(float clock_factor, uint32_t target_clock_speed) {
    uint32_t sys_clock_speed = BASE_CLOCK_SPEED * clock_factor;

    uint32_t clock_divider_fixed_point = sys_clock_speed * 256LL / target_clock_speed;
    pio_clock_divider div = {
        .int_div = clock_divider_fixed_point / 256,
        .frac_div = clock_divider_fixed_point % 256
    };

    return div;
}

struct pwm_clock_divider {
    uint32_t int_div;
    uint32_t frac_div;
    uint32_t wrap;
};

constexpr pwm_clock_divider pwm_clock_divider_for(float clock_factor, uint32_t target_clock_speed, uint32_t wrap) {
    uint32_t clock_divider_fixed_point = BASE_CLOCK_SPEED * 16LL / (target_clock_speed * wrap);
    pwm_clock_divider div = {
        .int_div = clock_divider_fixed_point / 16,
        .frac_div = clock_divider_fixed_point % 16,
        .wrap = (uint32_t)(wrap * clock_factor)
    };

    return div;
}

}
