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

#define ALIGNUP(n, align) (((n) + (align) - 1) & ~((align) - 1))
#define PROB_LEN (1 + 255 + 1 + 2*32 + 2*32)
#define DATA_OFF ALIGNUP(PROB_LEN, 4)

upkr_unpack:
    mov r8, r0
    sub sp, sp, #DATA_OFF

    // init probs
    mov r7, sp
    movs r2, #255
    adds r2, r2, #(PROB_LEN - 255)
    movs r3, #128
.Lclear:
    subs r2, r2, #1
    strb r3, [r7, r2]
    bne .Lclear

.Lloop:
    movs r5, #0
    bl upkr_decode_bit
    beq .Ldata
.Lmatch:
    // r6 = bit_ret = 1
    lsls r5, r6, #8
    cmp r4, #0
    mov r4, r6 // prev_was_match = 1
    bne 1f

    bl upkr_decode_bit
    beq 2f

1:
    adds r5, r5, #1
    bl upkr_decode_length
    subs r5, r5, #1
    subs r3, r4, r6
    beq .Lend
2:

    adds r5, r5, #65
    bl upkr_decode_length
.Lcopy_loop:
    ldrb r5, [r0, r3]
    strb r5, [r0]
    adds r0, r0, #1
    subs r6, r6, #1
    bne .Lcopy_loop
    b .Lloop

.Ldata:
    movs r4, #0
    movs r5, #1

.Ldata_loop:
    bl upkr_decode_bit
    lsls r5, r5, #1
    adds r5, r5, r6
    lsrs r6, r5, #8
    beq .Ldata_loop

    strb r5, [r0]
    adds r0, r0, #1
    b .Lloop

.Lend:
    add sp, sp, #DATA_OFF
    mov r2, r8
    adds r2, r2, #(PICOPACK_PART_ENTRY_OFFSET + 1)
    bx r2

// arg in r5 (preserved)
// ret in r6
upkr_decode_length:
    push { r0, r4, r5, lr }

    movs r4, #0
    movs r0, #0
.Lbit_loop:
    bl upkr_decode_bit
    beq 1f

    adds r5, r5, #1
    bl upkr_decode_bit
    adds r5, r5, #1
    lsls r6, r6, r4
    adds r4, r4, #1
    orrs r0, r0, r6
    b .Lbit_loop
1:
    movs r6, #1
    lsls r6, r6, r4
    orrs r6, r6, r0

    pop { r0, r4, r5, pc }

// arg in r5 (preserved)
// ret in r6
upkr_decode_bit:
    push { r0, r3, r4, lr }

.Lstate_loop:
    lsrs r0, r2, #12
    bne 1f
    lsls r2, r2, #8
    ldrb r6, [r1]
    adds r1, r1, #1
    orrs r2, r2, r6
    b .Lstate_loop
1:

    ldrb r4, [r7, r5]
    mov r0, r4

    uxtb r6, r2
    subs r6, r6, r4
    lsrs r6, r6, #31
    bne 1f

    rsbs r0, r0, #0
    uxtb r0, r0
1:

    lsrs r3, r2, #8
    muls r3, r3, r0
    uxtb r2, r2
    adds r2, r2, r3

    rsbs r3, r0, #0
    uxtb r3, r3
    adds r3, r3, #8
    lsrs r3, r3, #4
    adds r0, r0, r3

    cmp r6, #0
    bne 1f

    subs r2, r2, r4
    rsbs r0, r0, #0
1:

    strb r0, [r7, r5]

    cmp r6, #0
    pop { r0, r3, r4, pc }

.p2align 2
literals:
    .ltorg
    .byte 'y', 'r', 'l', 'f'

static_assert (__boot2_load_part - __boot2_start == PICOPACK_LOAD_PART_OFFSET), "PICOPACK_LOAD_PART_OFFSET incorrect"
