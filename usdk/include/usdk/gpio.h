#pragma once

#include <hardware/regs/addressmap.h>
#include <hardware/structs/iobank0.h>
#include <hardware/gpio.h>
#include <usdk/util/iovec.h>

namespace usdk {

inline void gpio_set_function(uint gpio, enum gpio_function fn) {
    iobank0_hw->io[gpio].ctrl = fn << IO_BANK0_GPIO0_CTRL_FUNCSEL_LSB;
}

namespace iovec {
    constexpr iovec<2> io_gpio_set_function(uint32_t gpio, enum gpio_function fn) {
        return io_write(IO_BANK0_BASE + IO_BANK0_GPIO0_CTRL_OFFSET + 8 * gpio, fn);
    }

    constexpr iovec<3> io_gpio_set_consecutive_functions(uint32_t gpio, uint32_t num, enum gpio_function fn) {
        return io_memset(IO_BANK0_BASE + IO_BANK0_GPIO0_CTRL_OFFSET + 8 * gpio, 8, num, fn);
    }
}

}
