#pragma once

#include <cstdint>
#include <cstring>
#include <array>
#include <hardware/pio.h>
#include <usdk/util/array.h>
#include <usdk/util/iovec.h>

namespace usdk {

constexpr uint32_t pio_major_instr_bits(uint32_t instr) {
    return instr & 0xe000;
}

constexpr uint32_t pio_encode_instr_and_args(enum pio_instr_bits instr_bits, uint32_t arg1, uint32_t arg2) {
    return instr_bits | (arg1 << 5u) | (arg2 & 0x1fu);
}

constexpr uint32_t pio_encode_instr_and_src_dest(enum pio_instr_bits instr_bits, enum pio_src_dest dest, uint32_t value) {
    return pio_encode_instr_and_args(instr_bits, dest & 7u, value);
}

constexpr uint32_t pio_encode_delay(uint32_t cycles) {
    return cycles << 8;
}

constexpr uint32_t pio_encode_sideset(uint32_t sideset_bit_count, uint32_t value) {
    return value << (13 - sideset_bit_count);
}

constexpr uint32_t pio_encode_sideset_opt(uint32_t sideset_bit_count, uint32_t value) {
    return 0x1000u | value << (12u - sideset_bit_count);
}

constexpr uint32_t pio_encode_jmp(uint32_t addr) {
    return pio_encode_instr_and_args(pio_instr_bits_jmp, 0, addr);
}

constexpr uint32_t pio_encode_jmp_not_x(uint32_t addr) {
    return pio_encode_instr_and_args(pio_instr_bits_jmp, 1, addr);
}

constexpr uint32_t pio_encode_jmp_x_dec(uint32_t addr) {
    return pio_encode_instr_and_args(pio_instr_bits_jmp, 2, addr);
}

constexpr uint32_t pio_encode_jmp_not_y(uint32_t addr) {
    return pio_encode_instr_and_args(pio_instr_bits_jmp, 3, addr);
}

constexpr uint32_t pio_encode_jmp_y_dec(uint32_t addr) {
    return pio_encode_instr_and_args(pio_instr_bits_jmp, 4, addr);
}

constexpr uint32_t pio_encode_jmp_x_ne_y(uint32_t addr) {
    return pio_encode_instr_and_args(pio_instr_bits_jmp, 5, addr);
}

constexpr uint32_t pio_encode_jmp_pin(uint32_t addr) {
    return pio_encode_instr_and_args(pio_instr_bits_jmp, 6, addr);
}

constexpr uint32_t pio_encode_jmp_not_osre(uint32_t addr) {
    return pio_encode_instr_and_args(pio_instr_bits_jmp, 7, addr);
}

constexpr uint32_t pio_encode_irq(bool relative, uint32_t irq) {
    return (relative ? 0x10u : 0x0u) | irq;
}

constexpr uint32_t pio_encode_wait_gpio(bool polarity, uint32_t gpio) {
    return pio_encode_instr_and_args(pio_instr_bits_wait, 0u | (polarity ? 4u : 0u), gpio);
}

constexpr uint32_t pio_encode_wait_pin(bool polarity, uint32_t pin) {
    return pio_encode_instr_and_args(pio_instr_bits_wait, 1u | (polarity ? 4u : 0u), pin);
}

constexpr uint32_t pio_encode_wait_irq(bool polarity, bool relative, uint32_t irq) {
    return pio_encode_instr_and_args(pio_instr_bits_wait, 2u | (polarity ? 4u : 0u), pio_encode_irq(relative, irq));
}

constexpr uint32_t pio_encode_in(enum pio_src_dest src, uint32_t count) {
    return pio_encode_instr_and_src_dest(pio_instr_bits_in, src, count);
}

constexpr uint32_t pio_encode_out(enum pio_src_dest dest, uint32_t count) {
    return pio_encode_instr_and_src_dest(pio_instr_bits_out, dest, count);
}

constexpr uint32_t pio_encode_push(bool if_full, bool block) {
    return pio_encode_instr_and_args(pio_instr_bits_push, (if_full ? 2u : 0u) | (block ? 1u : 0u), 0);
}

constexpr uint32_t pio_encode_pull(bool if_empty, bool block) {
    return pio_encode_instr_and_args(pio_instr_bits_pull, (if_empty ? 2u : 0u) | (block ? 1u : 0u), 0);
}

constexpr uint32_t pio_encode_mov(enum pio_src_dest dest, enum pio_src_dest src) {
    return pio_encode_instr_and_src_dest(pio_instr_bits_mov, dest, src & 7u);
}

constexpr uint32_t pio_encode_mov_not(enum pio_src_dest dest, enum pio_src_dest src) {
    return pio_encode_instr_and_src_dest(pio_instr_bits_mov, dest, (1u << 3u) | (src & 7u));
}

constexpr uint32_t pio_encode_mov_reverse(enum pio_src_dest dest, enum pio_src_dest src) {
    return pio_encode_instr_and_src_dest(pio_instr_bits_mov, dest, (2u << 3u) | (src & 7u));
}

constexpr uint32_t pio_encode_irq_set(bool relative, uint32_t irq) {
    return pio_encode_instr_and_args(pio_instr_bits_irq, 0, pio_encode_irq(relative, irq));
}

constexpr uint32_t pio_encode_irq_wait(bool relative, uint32_t irq) {
    return pio_encode_instr_and_args(pio_instr_bits_irq, 1, pio_encode_irq(relative, irq));
}

constexpr uint32_t pio_encode_irq_clear(bool relative, uint32_t irq) {
    return pio_encode_instr_and_args(pio_instr_bits_irq, 2, pio_encode_irq(relative, irq));
}

constexpr uint32_t pio_encode_set(enum pio_src_dest dest, uint32_t value) {
    return pio_encode_instr_and_src_dest(pio_instr_bits_set, dest, value);
}

constexpr uint32_t pio_encode_nop(void) {
    return usdk::pio_encode_mov(pio_y, pio_y);
}

constexpr void pio_calculate_clkdiv_from_float(float div, uint16_t& div_int, uint8_t& div_frac) {
    div_int = div;
    if (div_int == 0) {
        div_frac = 0;
    } else {
        div_frac = (div - div_int) * (1 << 8);
    }
}

struct pio_sm_config {
    uint32_t clkdiv = 0;
    uint32_t execctrl = 0;
    uint32_t shiftctrl = 0;
    uint32_t pinctrl = 0;

    constexpr pio_sm_config() {
        set_clkdiv_int_frac(1, 0);
        set_wrap(0, 31);
        set_in_shift(true, false, 32);
        set_out_shift(true, false, 32);
    }

    constexpr pio_sm_config(::pio_sm_config cfg) {
        clkdiv = cfg.clkdiv;
        execctrl = cfg.execctrl;
        shiftctrl = cfg.shiftctrl;
        pinctrl = cfg.pinctrl;
    }

    constexpr operator ::pio_sm_config() const {
        return { clkdiv, execctrl, shiftctrl, pinctrl };
    }

    constexpr void apply_config(pio_hw_t * pio, uint32_t sm) const {
        pio->sm[sm].clkdiv = clkdiv;
        pio->sm[sm].execctrl = execctrl;
        pio->sm[sm].shiftctrl = shiftctrl;
        pio->sm[sm].pinctrl = pinctrl;
    }

    constexpr void set_out_pins(uint32_t out_base, uint32_t out_count) {
        pinctrl = (
            (pinctrl & ~(PIO_SM0_PINCTRL_OUT_BASE_BITS | PIO_SM0_PINCTRL_OUT_COUNT_BITS))
            | (out_base << PIO_SM0_PINCTRL_OUT_BASE_LSB)
            | (out_count << PIO_SM0_PINCTRL_OUT_COUNT_LSB)
        );
    }

    constexpr void set_set_pins(uint32_t set_base, uint32_t set_count) {
        pinctrl = (
            (pinctrl & ~(PIO_SM0_PINCTRL_SET_BASE_BITS | PIO_SM0_PINCTRL_SET_COUNT_BITS))
            | (set_base << PIO_SM0_PINCTRL_SET_BASE_LSB)
            | (set_count << PIO_SM0_PINCTRL_SET_COUNT_LSB)
        );
    }

    constexpr void set_in_pins(uint32_t in_base) {
        pinctrl = (
            (pinctrl & ~PIO_SM0_PINCTRL_IN_BASE_BITS)
            | (in_base << PIO_SM0_PINCTRL_IN_BASE_LSB)
        );
    }

    constexpr void set_sideset(uint32_t bit_count, bool optional, bool pindirs) {
        pinctrl = (
            (pinctrl & ~PIO_SM0_PINCTRL_SIDESET_COUNT_BITS)
            | (bit_count << PIO_SM0_PINCTRL_SIDESET_COUNT_LSB)
        );
        execctrl = (
            (execctrl & ~(PIO_SM0_EXECCTRL_SIDE_EN_BITS | PIO_SM0_EXECCTRL_SIDE_PINDIR_BITS))
            | (optional << PIO_SM0_EXECCTRL_SIDE_EN_LSB)
            | (pindirs << PIO_SM0_EXECCTRL_SIDE_PINDIR_LSB)
        );
    }

    constexpr void set_clkdiv_int_frac(uint16_t div_int, uint8_t div_frac) {
        clkdiv = (
            0
            | (div_frac << PIO_SM0_CLKDIV_FRAC_LSB)
            | (div_int << PIO_SM0_CLKDIV_INT_LSB)
        );
    }

    constexpr void set_clkdiv(float div) {
        uint16_t div_int = 0;
        uint8_t div_frac = 0;
        pio_calculate_clkdiv_from_float(div, div_int, div_frac);
        set_clkdiv_int_frac(div_int, div_frac);
    }

    constexpr void set_wrap(uint32_t wrap_target, uint32_t wrap) {
        execctrl = (
            (execctrl & ~(PIO_SM0_EXECCTRL_WRAP_TOP_BITS | PIO_SM0_EXECCTRL_WRAP_BOTTOM_BITS))
            | (wrap_target << PIO_SM0_EXECCTRL_WRAP_BOTTOM_LSB)
            | (wrap << PIO_SM0_EXECCTRL_WRAP_TOP_LSB)
        );
    }

    constexpr void set_jmp_pin(uint32_t pin) {
        execctrl = (
            (execctrl & ~PIO_SM0_EXECCTRL_JMP_PIN_BITS)
            | (pin << PIO_SM0_EXECCTRL_JMP_PIN_LSB)
        );
    }

    constexpr void set_in_shift(bool shift_right, bool autopush, uint32_t push_threshold) {
        shiftctrl = (
            (shiftctrl & ~(
                PIO_SM0_SHIFTCTRL_IN_SHIFTDIR_BITS | PIO_SM0_SHIFTCTRL_AUTOPUSH_BITS | PIO_SM0_SHIFTCTRL_PUSH_THRESH_BITS
            ))
            | (shift_right << PIO_SM0_SHIFTCTRL_IN_SHIFTDIR_LSB)
            | (autopush << PIO_SM0_SHIFTCTRL_AUTOPUSH_LSB)
            | ((push_threshold & 0x1fu) << PIO_SM0_SHIFTCTRL_PUSH_THRESH_LSB)
        );
    }

    constexpr void set_out_shift(bool shift_right, bool autopull, uint32_t pull_threshold) {
        shiftctrl = (
            (shiftctrl & ~(
                PIO_SM0_SHIFTCTRL_OUT_SHIFTDIR_BITS | PIO_SM0_SHIFTCTRL_AUTOPULL_BITS | PIO_SM0_SHIFTCTRL_PULL_THRESH_BITS
            ))
            | (shift_right << PIO_SM0_SHIFTCTRL_OUT_SHIFTDIR_LSB)
            | (autopull << PIO_SM0_SHIFTCTRL_AUTOPULL_LSB)
            | ((pull_threshold & 0x1fu) << PIO_SM0_SHIFTCTRL_PULL_THRESH_LSB)
        );
    }

    constexpr void set_fifo_join(enum pio_fifo_join join) {
        shiftctrl = (
            (shiftctrl & ~(PIO_SM0_SHIFTCTRL_FJOIN_TX_BITS | PIO_SM0_SHIFTCTRL_FJOIN_RX_BITS))
            | (join << PIO_SM0_SHIFTCTRL_FJOIN_TX_LSB)
        );
    }

    constexpr void set_out_special(bool sticky, bool has_enable_pin, uint32_t enable_pin_index) {
        execctrl = (
            (execctrl & ~(
                PIO_SM0_EXECCTRL_OUT_STICKY_BITS | PIO_SM0_EXECCTRL_INLINE_OUT_EN_BITS | PIO_SM0_EXECCTRL_OUT_EN_SEL_BITS
            ))
            | (sticky << PIO_SM0_EXECCTRL_OUT_STICKY_LSB)
            | (has_enable_pin << PIO_SM0_EXECCTRL_INLINE_OUT_EN_LSB)
            | ((enable_pin_index << PIO_SM0_EXECCTRL_OUT_EN_SEL_LSB) & PIO_SM0_EXECCTRL_OUT_EN_SEL_BITS)
        );
    }

    constexpr void set_mov_status(enum pio_mov_status_type status_sel, uint32_t status_n) {
        execctrl = (
            (execctrl & ~(PIO_SM0_EXECCTRL_STATUS_SEL_BITS | PIO_SM0_EXECCTRL_STATUS_N_BITS))
            | ((status_sel << PIO_SM0_EXECCTRL_STATUS_SEL_LSB) & PIO_SM0_EXECCTRL_STATUS_SEL_BITS)
            | ((status_n << PIO_SM0_EXECCTRL_STATUS_N_LSB) & PIO_SM0_EXECCTRL_STATUS_N_BITS)
        );
    }
};

constexpr uint32_t pio_get_dreq(uint32_t pio, uint32_t sm, bool is_tx) {
    return sm + (is_tx ? 0 : NUM_PIO_STATE_MACHINES) + (pio == 0 ? DREQ_PIO0_TX0 : DREQ_PIO1_TX0);
}

constexpr uint32_t pio_sm_register_offset(uint32_t pio_sm0_offset) {
    return pio_sm0_offset - PIO_SM0_CLKDIV_OFFSET;
}

template <size_t N>
constexpr array<uint16_t, N> pio_program_relocate(const array<uint16_t, N>& in, uint8_t offset) {
    array<uint16_t, N> out = {};
    for (size_t i = 0; i < N; i++) {
        uint16_t instr = in[i];
        out[i] = pio_major_instr_bits(instr) != pio_instr_bits_jmp ? instr : instr + offset;
    }
    return out;
}

template <size_t N>
inline void pio_add_relocated_program(pio_hw_t * pio, size_t offset, const array<uint16_t, N>& program) {
    io_wo_32 * instr_mem = pio->instr_mem + offset;
    for (size_t i = 0; i < N; i++) {
        instr_mem[i] = program[i];
    }
}

template <size_t Pin, size_t Count>
inline void pio_sm_set_consecutive_pindirs(pio_hw_t * pio, uint32_t sm, bool is_out) {
    uint32_t pindir_val = is_out ? 0x1f : 0;
    if constexpr (Count > 5) {
        pio->sm[sm].pinctrl = (5u << PIO_SM0_PINCTRL_SET_COUNT_LSB) | (Pin << PIO_SM0_PINCTRL_SET_BASE_LSB);
        pio->sm[sm].instr = usdk::pio_encode_set(pio_pindirs, pindir_val);
        pio_sm_set_consecutive_pindirs<(Pin + 5) & 0x1f, Count - 5>(pio, sm, is_out);
    } else {
        pio->sm[sm].pinctrl = (Count << PIO_SM0_PINCTRL_SET_COUNT_LSB) | (Pin << PIO_SM0_PINCTRL_SET_BASE_LSB);
        pio->sm[sm].instr = usdk::pio_encode_set(pio_pindirs, pindir_val);
    }
}

namespace iovec {
    template <size_t N>
    constexpr iovec<2 + N> io_pio_add_relocated_program(uint32_t pio, size_t offset, array<uint16_t, N> program) {
        uint32_t pio_base = pio == 0 ? PIO0_BASE : PIO1_BASE;
        array<uint32_t, N> program_words = array_convert<uint32_t>(program);
        return io_memcpy(pio_base + PIO_INSTR_MEM0_OFFSET, 4, program_words);
    }

    template <size_t Pin, size_t Count>
    constexpr auto io_pio_sm_set_consecutive_pindirs(uint32_t pio, uint32_t sm, bool is_out) {
        uint32_t pio_base = pio == 0 ? PIO0_BASE : PIO1_BASE;
        uint32_t sm_base = pio_base + PIO_SM0_CLKDIV_OFFSET + sizeof (pio_sm_hw_t) * sm;
        uint32_t pindir_val = is_out ? 0x1f : 0;
        if constexpr (Count > 5) {
            return array_concat(
                io_write(sm_base + pio_sm_register_offset(PIO_SM0_PINCTRL_OFFSET),
                    (5u << PIO_SM0_PINCTRL_SET_COUNT_LSB) | (Pin << PIO_SM0_PINCTRL_SET_BASE_LSB)
                ),
                io_write(sm_base + pio_sm_register_offset(PIO_SM0_INSTR_OFFSET),
                    usdk::pio_encode_set(pio_pindirs, pindir_val)
                ),
                io_pio_sm_set_consecutive_pindirs<(Pin + 5) & 0x1f, Count - 5>(pio, sm, is_out)
            );
        } else {
            return array_concat(
                io_write(sm_base + pio_sm_register_offset(PIO_SM0_PINCTRL_OFFSET),
                    (Count << PIO_SM0_PINCTRL_SET_COUNT_LSB) | (Pin << PIO_SM0_PINCTRL_SET_BASE_LSB)
                ),
                io_write(sm_base + pio_sm_register_offset(PIO_SM0_INSTR_OFFSET),
                    usdk::pio_encode_set(pio_pindirs, pindir_val)
                )
            );
        }
    }

    constexpr iovec<7> io_pio_sm_configure(uint32_t pio, uint32_t sm, pio_sm_config cfg) {
        uint32_t pio_base = pio == 0 ? PIO0_BASE : PIO1_BASE;
        uint32_t sm_base = pio_base + PIO_SM0_CLKDIV_OFFSET + sizeof (pio_sm_hw_t) * sm;
        return array_concat(
            io_memcpy(sm_base, 4,
                cfg.clkdiv,
                cfg.execctrl,
                cfg.shiftctrl
            ),
            io_write(sm_base + (PIO_SM0_PINCTRL_OFFSET - PIO_SM0_CLKDIV_OFFSET), cfg.pinctrl)
        );
    }

    constexpr iovec<2> io_pio_sm_enable_mask(uint32_t pio, uint32_t sm_mask, bool enable) {
        uint32_t pio_base = pio == 0 ? PIO0_BASE : PIO1_BASE;
        return io_write(
            enable ? BSET(pio_base + PIO_CTRL_OFFSET) : BCLR(pio_base + PIO_CTRL_OFFSET),
            sm_mask << PIO_CTRL_SM_ENABLE_LSB
        );
    }

    constexpr iovec<2> io_pio_sm_clkdiv_restart_mask(uint32_t pio, uint32_t sm_mask) {
        uint32_t pio_base = pio == 0 ? PIO0_BASE : PIO1_BASE;
        return io_write(
            BSET(pio_base + PIO_CTRL_OFFSET),
            sm_mask << PIO_CTRL_CLKDIV_RESTART_LSB
        );
    }

    constexpr iovec<2> io_pio_sm_restart_mask(uint32_t pio, uint32_t sm_mask) {
        uint32_t pio_base = pio == 0 ? PIO0_BASE : PIO1_BASE;
        return io_write(
            BSET(pio_base + PIO_CTRL_OFFSET),
            sm_mask << PIO_CTRL_SM_RESTART_LSB
        );
    }

    constexpr iovec<2> io_pio_sm_exec(uint32_t pio, uint32_t sm, uint32_t instr) {
        uint32_t pio_base = pio == 0 ? PIO0_BASE : PIO1_BASE;
        uint32_t sm_base = pio_base + PIO_SM0_CLKDIV_OFFSET + sizeof (pio_sm_hw_t) * sm;
        return io_write(sm_base + pio_sm_register_offset(PIO_SM0_INSTR_OFFSET), instr);
    }

    constexpr iovec<2> io_pio_set_irq0_source_mask_enabled(uint32_t pio, uint32_t source_mask, bool enabled) {
        uint32_t pio_base = pio == 0 ? PIO0_BASE : PIO1_BASE;
        if (enabled) {
            return io_write(BSET(pio_base + PIO_IRQ0_INTE_OFFSET), source_mask);
        } else {
            return io_write(BCLR(pio_base + PIO_IRQ0_INTE_OFFSET), source_mask);
        }
    }

    constexpr iovec<2> io_pio_set_irq1_source_mask_enabled(uint32_t pio, uint32_t source_mask, bool enabled) {
        uint32_t pio_base = pio == 0 ? PIO0_BASE : PIO1_BASE;
        if (enabled) {
            return io_write(BSET(pio_base + PIO_IRQ1_INTE_OFFSET), source_mask);
        } else {
            return io_write(BCLR(pio_base + PIO_IRQ1_INTE_OFFSET), source_mask);
        }
    }
}

}
