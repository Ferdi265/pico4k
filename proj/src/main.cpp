#include <cstddef>
#include <cstdint>
#include <algorithm>

#include <hardware/regs/addressmap.h>
#include <hardware/structs/resets.h>
#include <hardware/structs/padsbank0.h>
#include <hardware/structs/iobank0.h>
#include <hardware/structs/sio.h>
#include <hardware/structs/xosc.h>
#include <hardware/structs/pll.h>
#include <hardware/structs/clocks.h>
#include <hardware/divider.h>
#include <hardware/sync.h>

#include <usdk/util/mask.h>
#include <usdk/util/array.h>
#include <usdk/util/iovec.h>
#include <usdk/irq.h>
#include <usdk/interp.h>
#include <usdk/runtime.h>
#include <usdk/multicore.h>

#include "memmove.h"
#include "irq_config.h"
#include "font_config.h"
#include "pwm_config.h"
#include "vga_config.h"
#include "interp_config.h"

#include "effect_text.h"
#include "effect_twister.h"
#include "effect_audio.h"

// --- isr ---------------------------------------------------------------------

extern "C" void __isr isr_dma_0() {
    // TWISTER
    dma_irqn_acknowledge_channel(0, pixels_data_chan);

    if (twister_batch_ready) {
        cur_twister_batch = next_twister_batch;
        twister_batch_ready = false;
    }

    dma_hw->ch[pixels_data_chan].al3_transfer_count = cur_twister_batch.length;
    dma_hw->ch[pixels_data_chan].al3_read_addr_trig = (uint32_t)cur_twister_batch.data;
}

extern "C" void __isr isr_dma_1() {
    // AUDIO
    dma_irqn_acknowledge_channel(1, sample_right_chan);
    refill_sample_buffer();
    current_sample_buffer ^= 1;
}

// --- draw --------------------------------------------------------------------

extern "C" void __isr isr_vblank() {
    // TEXT
    update_text();
}

// --- iovec construction ------------------------------------------------------

// working factors: 1.0,    2.0,    2.4     3.0     3.2     3.36    3.488   3.504
// aka:             125MHz, 250MHz, 300MHz, 375MHz, 400MHz, 420MHz, 436MHz, 438MHz
//                                                                  (with glitches)
constexpr float CLOCK_SPEED_FACTOR = 3.36;

constexpr usdk::array meta_init_iovec = usdk::array_concat(
    usdk::io_runtime_init(CLOCK_SPEED_FACTOR),
    pwm_audio_config(CLOCK_SPEED_FACTOR),
    pio_vga_config(CLOCK_SPEED_FACTOR),
    dma_video_config(),
    dma_audio_config(),
    sin_interp_config(),
    irq_core0_config()
);
const usdk::iovec::realized_iovec init_iovec = usdk::iovec::iovec_realize_v<meta_init_iovec>;

constexpr usdk::array core1_meta_init_iovec = usdk::array_concat(
    clamp_interp_config(), // needed for text
    irq_core1_config()
);
const usdk::iovec::realized_iovec core1_init_iovec = usdk::iovec::iovec_realize_v<core1_meta_init_iovec>;

// --- entry point -------------------------------------------------------------

void core1_main() {
    usdk::iovec::iovec_apply(core1_init_iovec.begin(), core1_init_iovec.end());
    text_draw_loop();
}

int main() {
    usdk::iovec::iovec_apply(init_iovec.begin(), init_iovec.end());

    init_vga();
    init_text();

    usdk::multicore_launch(core1_main);

    // start everything
    pwm_set_mask_enabled((1 << right_slice) | (1 << left_slice) |(1 << samplerate_slice));
    dma_start_channel_mask((1 << reconf_right_chan) | (1 << reconf_left_chan) | (1 << pixels_data_chan) | (1 << text_data_chan));
    pio_enable_sm_mask_in_sync(pio0, (1 << timing_sm) | (1 << pixels_sm) | (1 << text_sm));

    twister_draw_loop();
}
