.syntax unified

#include <hardware/regs/intctrl.h>
#include <hardware/regs/addressmap.h>
#include <hardware/regs/m0plus.h>
#include <hardware/regs/sio.h>
#include <pico/platform.h>
#include <picopack/assert.S>
#include <picopack/boot2.h>

.section .vectors, "ax"
.align 2

.global _vectors
_vectors:
    .word __StackTop
    .word isr_reset
    .word isr_nmi
    .word isr_hardfault
    .word isr_invalid
    .word isr_invalid
    .word isr_invalid
    .word isr_invalid
    .word isr_invalid
    .word isr_invalid
    .word isr_invalid
    .word isr_svcall
    .word isr_invalid
    .word isr_invalid
    .word isr_pendsv
    .word isr_systick
    .word isr_timer_0
    .word isr_timer_1
    .word isr_timer_2
    .word isr_timer_3
    .word isr_pwm_wrap
    .word isr_usbctrl
    .word isr_xip
    .word isr_pio0_0
    .word isr_pio0_1
    .word isr_pio1_0
    .word isr_pio1_1
    .word isr_dma_0
    .word isr_dma_1
    .word isr_io_bank0
    .word isr_io_qspi
    .word isr_sio_proc0
    .word isr_sio_proc1
    .word isr_clocks
    .word isr_spi0
    .word isr_spi1
    .word isr_uart0
    .word isr_uart1
    .word isr_adc_fifo
    .word isr_i2c0
    .word isr_i2c1
    .word isr_rtc
    .word isr_sw0
    .word isr_sw1
    .word isr_sw2
    .word isr_sw3
    .word isr_sw4
    .word isr_sw5

#if !PICOPACK_NO_ASM_ENTRYPOINT
.global _entry_point
.type _entry_point, %function
.thumb_func
_entry_point:
#if !PICOPACK_NO_NEXT_PART
    ldr r2, =next_part_ptr
    str r1, [r2]
#endif

    ldr r0, =_vectors
    ldr r1, =(PPB_BASE + M0PLUS_VTOR_OFFSET)
    str r0, [r1]
    ldm r0!, {r1, r2}
    msr msp, r1
    bx r2
#endif

static_assert (_entry_point - _vectors == PICOPACK_PART_ENTRY_OFFSET), "invalid part entry offset"

.section .text.vectors

.macro decl_weak name
    .weak \name
    .type \name,%function
    .thumb_func
    \name:
.endm

#if !PICOPACK_NO_ASM_ENTRYPOINT
.global isr_reset
.type isr_reset, %function
.thumb_func
.align 2
isr_reset:
    movs r0, #0
    ldr r1, =__bss_start__
    ldr r2, =__bss_end__
    b 1f
    // TODO: call memset
.Lclear_bss:
    str r0, [r1]
    adds r1, r1, #4
1:
    cmp r1, r2
    bne .Lclear_bss

#if PICOPACK_WITH_PICO_MEM_OPS
    bl __aeabi_mem_init
#endif

    bl main
    // fallthrough into _exit
#endif

.align 2
decl_weak _exit
#if PICOPACK_NO_ASM_ENTRYPOINT
decl_weak isr_reset
#endif
decl_weak isr_nmi
decl_weak isr_hardfault
decl_weak isr_invalid
decl_weak isr_invalid
decl_weak isr_invalid
decl_weak isr_invalid
decl_weak isr_invalid
decl_weak isr_invalid
decl_weak isr_invalid
decl_weak isr_svcall
decl_weak isr_invalid
decl_weak isr_invalid
decl_weak isr_pendsv
decl_weak isr_systick
decl_weak isr_timer_0
decl_weak isr_timer_1
decl_weak isr_timer_2
decl_weak isr_timer_3
decl_weak isr_pwm_wrap
decl_weak isr_usbctrl
decl_weak isr_xip
decl_weak isr_pio0_0
decl_weak isr_pio0_1
decl_weak isr_pio1_0
decl_weak isr_pio1_1
decl_weak isr_dma_0
decl_weak isr_dma_1
decl_weak isr_io_bank0
decl_weak isr_io_qspi
decl_weak isr_sio_proc0
decl_weak isr_sio_proc1
decl_weak isr_clocks
decl_weak isr_spi0
decl_weak isr_spi1
decl_weak isr_uart0
decl_weak isr_uart1
decl_weak isr_adc_fifo
decl_weak isr_i2c0
decl_weak isr_i2c1
decl_weak isr_rtc
decl_weak isr_sw0
decl_weak isr_sw1
decl_weak isr_sw2
decl_weak isr_sw3
decl_weak isr_sw4
decl_weak isr_sw5
1:
    bkpt #0
    b 1b

.section .data
#if !PICOPACK_NO_NEXT_PART
.global next_part_ptr
.align 2
next_part_ptr:
    .space 4
#endif

.section .stack
    // align to allow for memory protection (although this alignment is pretty much ignored by linker script)
    .align 5
    .equ StackSize, PICO_STACK_SIZE
    .space StackSize
