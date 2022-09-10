#pragma once

#include <usdk/util/iovec.h>
#include "vga_config.h"

constexpr auto irq_core0_config() {
    using namespace usdk::iovec;

    return usdk::array_concat(
        // enable irqs
        io_write(PPB_BASE + M0PLUS_NVIC_ICPR_OFFSET,
            (1 << PIO0_IRQ_0) | (1 << DMA_IRQ_0) | (WANT_VBLANK_IRQ ? (1 << VBLANK_IRQ) : 0)
        ),
        io_write(PPB_BASE + M0PLUS_NVIC_ISER_OFFSET,
            (1 << PIO0_IRQ_0) | (1 << DMA_IRQ_0) | (WANT_VBLANK_IRQ ? (1 << VBLANK_IRQ) : 0)
        )
    );
}

constexpr auto irq_core1_config() {
    using namespace usdk::iovec;

    return usdk::array_concat(
        io_write(PPB_BASE + M0PLUS_NVIC_ICPR_OFFSET,
            (1 << DMA_IRQ_1)
        ),
        io_write(PPB_BASE + M0PLUS_NVIC_ISER_OFFSET,
            (1 << DMA_IRQ_1)
        )
    );
}
