#include <hardware/regs/addressmap.h>
#include <hardware/regs/m0plus.h>
#include <hardware/structs/pio.h>

#include <usdk/util/array.h>

#include "vga_timing_irq.h"
#include "vga_timing.pio.h"

namespace demo {

extern "C" __attribute__((naked)) void __isr isr_pio0_0() {
    asm(R"(
.syntax unified
// -- macros -------------------------------------------------------------------

.macro begin type, name
.global \name
.type \name, %%\type
\name:
.endm

.macro end name
.size \name, . - \name
.endm

// -- timing_irq ---------------------------------------------------------------

begin function, vga_timing_irq
    ldr r2, =%[vga_timing_irq_reg]
    movs r3, %[vga_timing_irq_ack_bits]
    str r3, [r2]

    adr r2, vga_timing_scanline
    ldr r0, [r2]
    adds r0, r0, #1
    ldr r1, =%[vga_timing_total_lines]
    cmp r0, r1
    blt 1f
    movs r0, #0
1:
    str r0, [r2]
end vga_timing_irq

begin function, vga_timing_refill
    push {r4, r5, lr}

    ldr r1, =%[vga_timing_active_lines]+1
.if )" WANT_VBLANK_IRQ_STR R"(
    cmp r0, r1
    bne 1f

    ldr r2, =%[nvic_ispr]
    ldr r3, =%[vblank_irq]
    str r3, [r2]
1:
.endif

    ldr r2, =vga_timing_cmds
    ldm r2!, {r3, r4, r5}

    cmp r0, r1
    blt 1f

    movs r2, %[vga_timing_irq_xor]
    eors r3, r3, r2

    adds r1, r1, %[vga_timing_vfront_lines]
    subs r1, r0, r1
    bmi 1f
    subs r1, r1, %[vga_timing_vsync_lines]
    bpl 1f

    ldr r2, =%[vga_timing_vsync_xor]
    eors r3, r3, r2
    eors r4, r4, r2
    eors r5, r5, r2
1:

    ldr r2, =%[vga_timing_fifo]
    str r3, [r2]
    str r4, [r2]
    str r5, [r2]
    pop {r4, r5, pc}
end vga_timing_irq

// -- variables ----------------------------------------------------------------

.align 2
begin object, vga_timing_scanline
    .word 0
end vga_timing_scanline

// -----------------------------------------------------------------------------
        )" ::
        [vga_timing_irq_ack_bits]"i"(VGAConfig::irq_ack_bits),
        [vga_timing_irq_xor]"i"(VGAConfig::irq_xor),
        [vga_timing_vsync_xor]"i"(VGAConfig::vsync_xor),
        [vga_timing_active_lines]"i"(VGAConfig::active_lines),
        [vga_timing_vfront_lines]"i"(VGAConfig::vfront_lines),
        [vga_timing_vsync_lines]"i"(VGAConfig::vsync_lines),
        [vga_timing_vback_lines]"i"(VGAConfig::vback_lines),
        [vga_timing_total_lines]"i"(VGAConfig::total_lines),

        [vga_timing_cmds_0]"i"(VGAConfig::commands[0]),
        [vga_timing_cmds_1]"i"(VGAConfig::commands[1]),
        [vga_timing_cmds_2]"i"(VGAConfig::commands[2]),

        [vga_timing_irq_reg]"i"(PIO0_BASE + PIO_IRQ_OFFSET),
        [vga_timing_fifo]"i"(PIO0_BASE + PIO_TXF0_OFFSET),
        [nvic_ispr]"i"(WANT_VBLANK_IRQ ? PPB_BASE + M0PLUS_NVIC_ISPR_OFFSET : 0),
        [vblank_irq]"i"(WANT_VBLANK_IRQ ? 1 << VBLANK_IRQ : 0)
    );
}

extern "C" {
    alignas(4) extern const usdk::array<uint32_t, 3> vga_timing_cmds = usdk::to_array(VGAConfig::commands);
}

}
