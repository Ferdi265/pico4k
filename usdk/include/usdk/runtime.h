#pragma once

#include <hardware/regs/addressmap.h>
#include <hardware/structs/resets.h>
#include <hardware/structs/padsbank0.h>
#include <hardware/structs/iobank0.h>
#include <hardware/structs/sio.h>
#include <hardware/structs/xosc.h>
#include <hardware/structs/pll.h>
#include <hardware/structs/clocks.h>
#include <hardware/regs/m0plus.h>

#include <usdk/util/array.h>
#include <usdk/util/iovec.h>
#include <usdk/irq.h>

namespace usdk {

constexpr auto io_runtime_init() {
    using namespace usdk::iovec;

    constexpr uint32_t early_reset_bits = (
        // qspi is always needed for flash
        (1 << RESETS_RESET_IO_QSPI_LSB) |
        // devices needed for clock config
        (1 << RESETS_RESET_PLL_SYS_LSB) |
        // HACK: reset_done of TBMAN is always set, take it out of reset to make it make sense
        (1 << RESETS_RESET_TBMAN_LSB) |
        0
    );

    constexpr uint32_t late_reset_bits = (
        early_reset_bits |
        // gpio devices
        (1 << RESETS_RESET_IO_BANK0_LSB) |
        (1 << RESETS_RESET_PADS_BANK0_LSB) |
        // processor stuff
        (1 << RESETS_RESET_BUSCTRL_LSB) |
        (1 << RESETS_RESET_SYSCFG_LSB) |
        (1 << RESETS_RESET_SYSINFO_LSB) |
        (1 << RESETS_RESET_TIMER_LSB) |
        // other devices
        (1 << RESETS_RESET_DMA_LSB) |
        (1 << RESETS_RESET_PIO0_LSB) |
        (1 << RESETS_RESET_PIO1_LSB) |
        (1 << RESETS_RESET_PWM_LSB) |
        (1 << RESETS_RESET_ADC_LSB) |
        (1 << RESETS_RESET_I2C0_LSB) |
        (1 << RESETS_RESET_I2C1_LSB) |
        (1 << RESETS_RESET_JTAG_LSB) |
        (1 << RESETS_RESET_PADS_QSPI_LSB) |
        (1 << RESETS_RESET_PLL_USB_LSB) |
        (1 << RESETS_RESET_RTC_LSB) |
        (1 << RESETS_RESET_SPI0_LSB) |
        (1 << RESETS_RESET_SPI1_LSB) |
        (1 << RESETS_RESET_UART0_LSB) |
        (1 << RESETS_RESET_UART1_LSB) |
        (1 << RESETS_RESET_USBCTRL_LSB) |
        0
    );

    return usdk::array_concat(
        // take peripherals for clock config out of reset
        io_write(RESETS_BASE + RESETS_RESET_OFFSET, ~early_reset_bits),
        io_wait(RESETS_BASE + RESETS_RESET_DONE_OFFSET, early_reset_bits),

        // enable crystal oscillator
        io_write(XOSC_BASE + XOSC_CTRL_OFFSET, (XOSC_CTRL_ENABLE_VALUE_ENABLE << XOSC_CTRL_ENABLE_LSB)),
        io_wait(XOSC_BASE + XOSC_STATUS_OFFSET, (
            (1 << XOSC_STATUS_STABLE_LSB) |
            (1 << XOSC_STATUS_ENABLED_LSB) |
            (1 << XOSC_STATUS_BADWRITE_LSB) |
            // HACK: datasheet says this always reads zero, it doesn't
            (1 << XOSC_STATUS_FREQ_RANGE_LSB)
        )),

        // configure system PLL
        io_write(PLL_SYS_BASE + PLL_CS_OFFSET, 1),
        io_write(PLL_SYS_BASE + PLL_FBDIV_INT_OFFSET, 125),
        io_write(PLL_SYS_BASE + PLL_PWR_OFFSET, (
            (0 << PLL_PWR_VCOPD_LSB) |
            (1 << PLL_PWR_POSTDIVPD_LSB) |
            (1 << PLL_PWR_DSMPD_LSB) |
            (0 << PLL_PWR_PD_LSB)
        )),
        io_wait(PLL_SYS_BASE + PLL_CS_OFFSET, ((1 << PLL_CS_LOCK_LSB) | (1 << PLL_CS_REFDIV_LSB))),
        io_write(PLL_SYS_BASE + PLL_PRIM_OFFSET, (
            (6 << PLL_PRIM_POSTDIV1_LSB) |
            (2 << PLL_PRIM_POSTDIV2_LSB)
        )),
        io_write(PLL_SYS_BASE + PLL_PWR_OFFSET, (
            (0 << PLL_PWR_VCOPD_LSB) |
            (0 << PLL_PWR_POSTDIVPD_LSB) |
            (1 << PLL_PWR_DSMPD_LSB) |
            (0 << PLL_PWR_PD_LSB)
        )),

        // configure reference clock to XOSC
        io_write(CLOCKS_BASE + CLOCKS_CLK_REF_CTRL_OFFSET, (
            (CLOCKS_CLK_REF_CTRL_SRC_VALUE_XOSC_CLKSRC << CLOCKS_CLK_REF_CTRL_SRC_LSB)
        )),
        io_wait(CLOCKS_BASE + CLOCKS_CLK_REF_SELECTED_OFFSET, (1 << CLOCKS_CLK_REF_CTRL_SRC_VALUE_XOSC_CLKSRC)),

        // configure system clock to system PLL
        io_write(CLOCKS_BASE + CLOCKS_CLK_SYS_CTRL_OFFSET, (
            (CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS << CLOCKS_CLK_SYS_CTRL_AUXSRC_LSB)
        )),
        io_write(CLOCKS_BASE + CLOCKS_CLK_SYS_CTRL_OFFSET, (
            (CLOCKS_CLK_SYS_CTRL_AUXSRC_VALUE_CLKSRC_PLL_SYS << CLOCKS_CLK_SYS_CTRL_AUXSRC_LSB) |
            (CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX << CLOCKS_CLK_SYS_CTRL_SRC_LSB)
        )),
        io_wait(CLOCKS_BASE + CLOCKS_CLK_SYS_SELECTED_OFFSET, (1 << CLOCKS_CLK_SYS_CTRL_SRC_VALUE_CLKSRC_CLK_SYS_AUX)),

        // take other peripherals out of reset
        io_write(RESETS_BASE + RESETS_RESET_OFFSET, ~late_reset_bits),
        io_wait(RESETS_BASE + RESETS_RESET_DONE_OFFSET, late_reset_bits),

        // init irq priorities
        // TODO: create usdk stuff for nicer priority init
        io_memset(PPB_BASE + M0PLUS_NVIC_IPR0_OFFSET, 4, 8, PICO_DEFAULT_IRQ_PRIORITY * 0x01010101U)
    );
}

}
