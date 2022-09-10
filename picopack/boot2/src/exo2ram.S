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

#define out_ptr r0
#define in_ptr r1
#define b2  r2
#define idx r2
#define tmp r2
#define len r3
#define i   r3
#define c256 r4
#define arg r5
#define ret r6
#define bit_buffer r7

/*
    PARAMETERS
        r0 = output ptr
        r1 = input ptr
    REGISTERS
        r0 = output ptr
        r1 = input ptr
        r2 = b2 / idx / tmp
        r3 = len / i
        r4 = #256
        r5 = subroutine arg / scratch
        r6 = subroutine ret / scratch
        r7 = bit buffer
    RETURNS
        r0 = output ptr + decomp size
        r1 = input ptr + comp size
    STACK
        | { uint16_t, uint16_t } base_bits[52] | sp
        | saved regs                           | sp + 4*52
*/
exo_decrunch:
    push {r0}
    sub sp, sp, #4*52

    movs c256, #1
    lsls c256, c256, #8

    // init
    movs bit_buffer, #0
    movs i, #0
.Ldecrunch_init:
    // reset b2 every 16 iters
    movs arg, #15
    tst i, arg
    bne 1f
    movs b2, #1
1:
    // read bits into b1
    movs arg, #4
    bl .Lexo_read_bits

    // store b2 to base
    // store b1 to bits
    lsls arg, i, #2
    add arg, arg, sp
    strh b2, [arg, #0]
    strh ret, [arg, #2]

    // increment b2
    movs arg, #1
    lsls arg, arg, ret
    add b2, b2, arg

    // increment i
    adds i, i, #1
    cmp i, #52
    bne .Ldecrunch_init

    // decrunch
    movs bit_buffer, #0

.Ldecrunch_literal_byte:
    bl .Lexo_literal_byte
.Ldecrunch:
    // check for literal byte
    movs arg, #1
    bl .Lexo_read_bits
    cmp ret, #1
    beq .Ldecrunch_literal_byte

    movs idx, #0
.Ldecrunch_read_gamma:
    movs arg, #1
    bl .Lexo_read_bits
    cmp ret, #0
    bne 1f
    adds idx, idx, #1
    b .Ldecrunch_read_gamma
1:

    // check for stop code
    cmp idx, #16
    beq .Ldecrunch_end
    // check for literal run
    cmp idx, #17
    bne 1f
.Ldecrunch_literal_run:
    movs arg, #16
    bl .Lexo_read_bits
    mov len, ret
    mov arg, in_ptr
    add in_ptr, in_ptr, len
.Ldecrunch_copy:
    ldrb ret, [arg]
    strb ret, [out_ptr]
    adds arg, arg, #1
    adds out_ptr, out_ptr, #1
    subs len, len, #1
    bne .Ldecrunch_copy
    b .Ldecrunch

1:
    // otherwise: offset literal
    bl .Lexo_read_length
    add idx, idx, ret
    mov len, idx

    cmp idx, #3
    ble 1f
    movs idx, #3
1:
    subs idx, idx, #1
    movs arg, #2
    cmp idx, #0
    beq 1f
    movs arg, #4
1:
    bl .Lexo_read_bits
    lsls arg, idx, #4
    movs idx, #48
    subs idx, idx, arg
    add idx, idx, ret

    bl .Lexo_read_length
    add idx, idx, ret

    // offset in idx
    // length in len
    mov arg, out_ptr
    subs arg, arg, idx
    b .Ldecrunch_copy

.Ldecrunch_end:
    add sp, sp, #4*52
    pop {r2}
    adds r2, r2, #(PICOPACK_PART_ENTRY_OFFSET + 1)
    bx r2

/*
    PARAMETERS
        idx .. index
    REGISTERS
        clobbers arg
        does not clobber tmp (via fallthrough to exo_read_bits)
        uses #256 (via fallthrough to exo_read_bits)
        uses in_ptr (via fallthrough to exo_read_bits)
        uses bit_buffer (via fallthrough to exo_read_bits)
    RETURNS
        idx ... base[index]
        ret .. read_bits(bits[index])
*/
.Lexo_read_length:
    lsls arg, idx, #2
    add arg, arg, sp
    ldrh idx, [arg, #0]
    ldrh arg, [arg, #2]
    // falls through
    // b .Lexo_read_bits

/*
    PARAMETERS
        arg = number of bits
    REGISTERS
        does not clobber tmp
        uses #256
        uses in_ptr
        uses bit_buffer
    RETURNS
        ret = read bits
*/
.Lexo_read_bits:
    push {r2, lr}

    movs ret, #0
.Lread_bits_loop:
    cmp arg, #0
    beq .Lread_bits_end
    cmp bit_buffer, #1
    bhi 1f
    ldrb bit_buffer, [in_ptr]
    adds in_ptr, in_ptr, #1
    orrs bit_buffer, bit_buffer, c256
1:
    lsls ret, ret, #1
    movs tmp, #1
    ands tmp, tmp, bit_buffer
    orrs ret, ret, tmp
    lsrs bit_buffer, bit_buffer, #1

    subs arg, arg, #1
    b .Lread_bits_loop
.Lread_bits_end:
    pop {r2, pc}

/*
    PARAMETERS
        none
    REGISTERS
        clobbers arg
        uses in_ptr
        uses out_ptr
    RETURNS
        none
*/
.Lexo_literal_byte:
    ldrb arg, [in_ptr]
    strb arg, [out_ptr]
    adds in_ptr, in_ptr, #1
    adds out_ptr, out_ptr, #1
    bx lr

.p2align 2
literals:
    .ltorg
    .byte 'y', 'r', 'l', 'f'

static_assert (__boot2_load_part - __boot2_start == PICOPACK_LOAD_PART_OFFSET), "PICOPACK_LOAD_PART_OFFSET incorrect"
