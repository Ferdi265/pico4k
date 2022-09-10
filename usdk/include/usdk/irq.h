#pragma once

#include <hardware/irq.h>

namespace usdk {

#define IRQ_SW_BASE 26

inline void irq_set_priority(uint num, uint8_t hardware_priority) {
    check_irq_param(num);

    // note that only 32 bit writes are supported
    io_rw_32 *p = (io_rw_32 *)((PPB_BASE + M0PLUS_NVIC_IPR0_OFFSET) + (num & ~3u));
    *p = (*p & ~(0xffu << (8 * (num & 3u)))) | (((uint32_t) hardware_priority) << (8 * (num & 3u)));
}

inline void irq_set_mask_enabled(uint32_t mask, bool enabled) {
    if (enabled) {
        // Clear pending before enable
        // (if IRQ is actually asserted, it will immediately re-pend)
        *((io_rw_32 *) (PPB_BASE + M0PLUS_NVIC_ICPR_OFFSET)) = mask;
        *((io_rw_32 *) (PPB_BASE + M0PLUS_NVIC_ISER_OFFSET)) = mask;
    } else {
        *((io_rw_32 *) (PPB_BASE + M0PLUS_NVIC_ICER_OFFSET)) = mask;
    }
}

}
