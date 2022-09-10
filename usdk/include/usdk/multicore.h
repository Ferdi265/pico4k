#pragma once

namespace usdk {

#include <hardware/regs/addressmap.h>
#include <hardware/regs/sio.h>
#include <pico/platform.h>

[[gnu::always_inline]] inline void __not_in_flash_func(multicore_launch)(void (*entry)()) {
    uint32_t ptr;
    uint32_t end;
    uint32_t fifo;
    uint32_t scratch;
    asm volatile(R"(
    .syntax unified

    adr %[ptr], .Lsequence
    str %[entry], [%[ptr], #(.Lsequence_entry - .Lsequence)]
    adr %[end], .Lsequence_end
    ldr %[fifo], =%[fifo_st_addr]

.Lnext:
    ldm %[ptr]!, {%[scratch]}

    str %[scratch], [%[fifo], %[fifo_wr_offset]]
    sev
    wfe

    cmp %[ptr], %[end]
    bne .Lnext

    b .Lsequence_end

    .align
.Lsequence:
    .word 0
    .word 0
    .word 1
    .word _vectors
    .word __StackOneBottom
.Lsequence_entry:
    .word 0
.Lsequence_end:
        )"
        :
        [ptr]"=&r"(ptr),
        [end]"=r"(end),
        [fifo]"=r"(fifo),
        [scratch]"=r"(scratch)
        :
        [fifo_st_addr]"i"(SIO_BASE + SIO_FIFO_ST_OFFSET),
        [fifo_wr_offset]"i"(SIO_FIFO_WR_OFFSET - SIO_FIFO_ST_OFFSET),
        [entry]"r"(entry)
    );
}

}
