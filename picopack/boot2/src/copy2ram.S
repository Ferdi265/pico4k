.syntax unified
.cpu cortex-m0plus
.thumb

#include <hardware/regs/addressmap.h>
#include <picopack/assert.S>
#include <picopack/boot2.h>

.section .text
__boot2_start:
    movs r2, #0
    ldrh r0, [r2, #BOOTROM_FUNC_TABLE]
    ldr r1, =('C' | ('X' << 8))
    ldrh r2, [r2, #BOOTROM_TABLE_LOOKUP]
    blx r2
    blx r0

    ldr r0, =(SRAM_BASE)
    ldr r1, =(XIP_BASE + 0x100)
__boot2_load_part:
    push {r0}
    ldm r1!, {r2}
__boot2_copy_loop:
    ldrb r3, [r1]
    strb r3, [r0]
    adds r0, r0, #1
    adds r1, r1, #1
    subs r2, r2, #1
    bne __boot2_copy_loop

    pop {r2}
    adds r2, r2, #(PICOPACK_PART_ENTRY_OFFSET + 1)
    bx r2

.p2align 2
literals:
    .ltorg
    .byte 'y', 'r', 'l', 'f'

static_assert (__boot2_load_part - __boot2_start == PICOPACK_LOAD_PART_OFFSET), "PICOPACK_LOAD_PART_OFFSET incorrect"
