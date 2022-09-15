#pragma once

#include <cstddef>
#include <cstdint>

namespace usdk {

constexpr uint32_t XOSC_CLOCK_SPEED = 12 * 1000 * 1000;
constexpr uint32_t BASE_CLOCK_SPEED = 125 * 1000 * 1000;
constexpr uint32_t VCO_CLOCK_SPEED = 1500 * 1000 * 1000;

struct pll_config {
    uint32_t fbdiv;
    uint32_t postdiv1;
    uint32_t postdiv2;
};

constexpr pll_config pll_config_for(float clock_factor) {
    uint32_t sys_clock_speed = BASE_CLOCK_SPEED * clock_factor;

    uint32_t vco_factor = VCO_CLOCK_SPEED / XOSC_CLOCK_SPEED;
    while (VCO_CLOCK_SPEED % XOSC_CLOCK_SPEED != 0) /* cannot determine valid vco factor, not an integer multiple */;

    uint32_t postdiv_factor = VCO_CLOCK_SPEED / sys_clock_speed;
    while (VCO_CLOCK_SPEED % sys_clock_speed != 0) /* cannot determine valid postdiv factor, not an integer multiple */;

    pll_config cfg = {
        .fbdiv = vco_factor,
        .postdiv1 = 1,
        .postdiv2 = 1,
    };

    uint32_t factors[] = { 2, 3, 5, 7 };
    for (uint32_t factor : factors) {
        while (postdiv_factor % factor == 0) {
            if (cfg.postdiv1 * factor < 8) { cfg.postdiv1 *= factor; postdiv_factor /= factor; }
            else if (cfg.postdiv2 * factor < 8) { cfg.postdiv2 *= factor; postdiv_factor /= factor; }
            else while (1) { /* cannot determine valid postdivider, too many factors */ }
        }
    }

    while (postdiv_factor != 1) /* cannot determine valid postdivider, factor too large */;

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
