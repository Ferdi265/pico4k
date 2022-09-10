#pragma once

#include <hardware/regs/sio.h>
#include <hardware/interp.h>
#include <usdk/util/iovec.h>

namespace usdk {

struct interp_config {
    uint32_t ctrl = 0;

    constexpr interp_config() {
        set_mask(0, 31);
    }

    constexpr interp_config(::interp_config cfg) {
        ctrl = cfg.ctrl;
    }

    constexpr operator ::interp_config() const {
        return { ctrl };
    }

    constexpr void set_shift(uint32_t shift) {
        ctrl = (ctrl & ~SIO_INTERP0_CTRL_LANE0_SHIFT_BITS) | (shift << SIO_INTERP0_CTRL_LANE0_SHIFT_LSB);
    }

    constexpr void set_mask(uint32_t mask_lsb, uint32_t mask_msb) {
        ctrl = (ctrl & ~SIO_INTERP0_CTRL_LANE0_MASK_LSB_BITS) | (mask_lsb << SIO_INTERP0_CTRL_LANE0_MASK_LSB_LSB);
        ctrl = (ctrl & ~SIO_INTERP0_CTRL_LANE0_MASK_MSB_BITS) | (mask_msb << SIO_INTERP0_CTRL_LANE0_MASK_MSB_LSB);
    }

    constexpr void set_cross_input(bool cross_input) {
        ctrl = (ctrl & ~SIO_INTERP0_CTRL_LANE0_CROSS_INPUT_BITS) | (cross_input << SIO_INTERP0_CTRL_LANE0_CROSS_INPUT_LSB);
    }

    constexpr void set_cross_result(bool cross_result) {
        ctrl = (ctrl & ~SIO_INTERP0_CTRL_LANE0_CROSS_RESULT_BITS) | (cross_result << SIO_INTERP0_CTRL_LANE0_CROSS_RESULT_LSB);
    }

    constexpr void set_signed(bool is_signed) {
        ctrl = (ctrl & ~SIO_INTERP0_CTRL_LANE0_SIGNED_BITS) | (is_signed << SIO_INTERP0_CTRL_LANE0_SIGNED_LSB);
    }

    constexpr void set_add_raw(bool add_raw) {
        ctrl = (ctrl & ~SIO_INTERP0_CTRL_LANE0_ADD_RAW_BITS) | (add_raw << SIO_INTERP0_CTRL_LANE0_ADD_RAW_LSB);
    }

    constexpr void set_blend(bool blend) {
        ctrl = (ctrl & ~SIO_INTERP0_CTRL_LANE0_BLEND_BITS) | (blend << SIO_INTERP0_CTRL_LANE0_BLEND_LSB);
    }

    constexpr void set_clamp(bool clamp) {
        ctrl = (ctrl & ~SIO_INTERP1_CTRL_LANE0_CLAMP_BITS) | (clamp << SIO_INTERP1_CTRL_LANE0_CLAMP_LSB);
    }

    constexpr void set_force_bits(uint32_t bits) {
        ctrl = (ctrl & ~SIO_INTERP0_CTRL_LANE0_FORCE_MSB_BITS) | (bits << SIO_INTERP0_CTRL_LANE0_FORCE_MSB_LSB);
    }
};

constexpr uint32_t interp_register_offset(uint32_t interp0_offset) {
    return interp0_offset - SIO_INTERP0_ACCUM0_OFFSET;
}

namespace iovec {
    constexpr iovec<2> io_interp_set_config(uint32_t interp, uint32_t lane, interp_config cfg) {
        uint32_t interp_base = SIO_BASE + SIO_INTERP0_ACCUM0_OFFSET + interp * sizeof (interp_hw_t);
        uint32_t ctrl_reg = interp_base + interp_register_offset(SIO_INTERP0_CTRL_LANE0_OFFSET) + 4 * lane;
        return io_write(ctrl_reg, cfg.ctrl);
    }

    constexpr iovec<2> io_interp_set_base(uint32_t interp, uint32_t lane, iovec_value value) {
        uint32_t interp_base = SIO_BASE + SIO_INTERP0_ACCUM0_OFFSET + interp * sizeof (interp_hw_t);
        uint32_t base_reg = interp_base + interp_register_offset(SIO_INTERP0_BASE0_OFFSET) + 4 * lane;
        return io_write(base_reg, value);
    }
}

}
