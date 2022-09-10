#pragma once

#include <cstdint>
#include <hardware/regs/addressmap.h>
#include <hardware/regs/pwm.h>
#include <hardware/pwm.h>
#include <usdk/util/iovec.h>

namespace usdk {

constexpr uint8_t pwm_gpio_to_slice(uint32_t gpio) {
    return (gpio >> 1) & 7;
}

constexpr uint8_t pwm_gpio_to_channel(uint32_t gpio) {
    return gpio & 1;
}

constexpr uint32_t pwm_get_dreq(uint32_t slice) {
    return DREQ_PWM_WRAP0 + slice;
}

struct pwm_config {
    uint32_t csr = 0;
    uint32_t div = 0;
    uint32_t top = 0;

    constexpr pwm_config() {
        set_phase_correct(false);
        set_clkdiv_int(1);
        set_clkdiv_mode(PWM_DIV_FREE_RUNNING);
        set_output_polarity(false, false);
        set_wrap(0xffff);
    }

    constexpr pwm_config(::pwm_config cfg) {
        csr = cfg.csr;
        div = cfg.div;
        top = cfg.top;
    }

    constexpr operator ::pwm_config() const {
        return { csr, div, top };
    }

    constexpr void set_phase_correct(bool phase_correct) {
        csr = (csr & ~PWM_CH0_CSR_PH_CORRECT_BITS) | (phase_correct << PWM_CH0_CSR_PH_CORRECT_LSB);
    }

    constexpr void set_clkdiv(float div) {
        div = div * (1 << PWM_CH0_DIV_INT_LSB);
    }

    constexpr void set_clkdiv_int_frac(uint8_t integer, uint8_t frac) {
        div = (integer << PWM_CH0_DIV_INT_LSB) | (frac << PWM_CH0_DIV_FRAC_LSB);
    }

    constexpr void set_clkdiv_int(uint8_t integer) {
        set_clkdiv_int_frac(integer, 0);
    }

    constexpr void set_clkdiv_mode(enum pwm_clkdiv_mode mode) {
        csr = (csr & ~PWM_CH0_CSR_DIVMODE_BITS) | (mode << PWM_CH0_CSR_DIVMODE_LSB);
    }

    constexpr void set_output_polarity(bool a, bool b) {
        csr = (csr & ~(PWM_CH0_CSR_A_INV_BITS | PWM_CH0_CSR_B_INV_BITS))
            | (a << PWM_CH0_CSR_A_INV_LSB)
            | (b << PWM_CH0_CSR_B_INV_LSB);
    }

    constexpr void set_wrap(uint16_t wrap) {
        top = wrap;
    }
};

namespace iovec {
    constexpr iovec<9> io_pwm_init(uint32_t slice, pwm_config cfg, bool start) {
        uint32_t slice_base = PWM_BASE + sizeof (pwm_slice_hw_t) * slice;
        return array_concat(
            io_memcpy(slice_base, 4,
                0,
                cfg.div,
                PWM_CH0_CTR_RESET,
                PWM_CH0_CC_RESET,
                cfg.top
            ),
            io_write(slice_base, cfg.csr | (start << PWM_CH0_CSR_EN_LSB))
        );
    }
}

}
