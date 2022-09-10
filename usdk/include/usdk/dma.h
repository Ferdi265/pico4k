#pragma once

#include <hardware/regs/dma.h>
#include <hardware/dma.h>
#include <usdk/util/iovec.h>

namespace usdk {

struct dma_channel_config {
    uint32_t ctrl = 0;

    constexpr dma_channel_config(uint32_t chan) {
        set_read_increment(true);
        set_write_increment(false);
        set_dreq(DREQ_FORCE);
        set_chain_to(chan);
        set_transfer_data_size(DMA_SIZE_32);
        set_ring(false, 0);
        set_bswap(false);
        set_irq_quiet(false);
        set_enable(true);
        set_sniff_enable(false);
    }

    constexpr dma_channel_config(::dma_channel_config cfg) {
        ctrl = cfg.ctrl;
    }

    constexpr operator ::dma_channel_config() const {
        return { ctrl };
    }

    constexpr void set_read_increment(bool incr) {
        ctrl = (ctrl & ~DMA_CH0_CTRL_TRIG_INCR_READ_BITS) | (incr << DMA_CH0_CTRL_TRIG_INCR_READ_LSB);
    }

    constexpr void set_write_increment(bool incr) {
        ctrl = (ctrl & ~DMA_CH0_CTRL_TRIG_INCR_WRITE_BITS) | (incr << DMA_CH0_CTRL_TRIG_INCR_WRITE_LSB);
    }

    constexpr void set_dreq(uint32_t dreq) {
        ctrl = (ctrl & ~DMA_CH0_CTRL_TRIG_TREQ_SEL_BITS) | (dreq << DMA_CH0_CTRL_TRIG_TREQ_SEL_LSB);
    }

    constexpr void set_chain_to(uint32_t chan) {
        ctrl = (ctrl & ~DMA_CH0_CTRL_TRIG_CHAIN_TO_BITS) | (chan << DMA_CH0_CTRL_TRIG_CHAIN_TO_LSB);
    }

    constexpr void set_transfer_data_size(uint32_t data_size) {
        ctrl = (ctrl & ~DMA_CH0_CTRL_TRIG_DATA_SIZE_BITS) | (data_size << DMA_CH0_CTRL_TRIG_DATA_SIZE_LSB);
    }

    constexpr void set_ring(bool write, uint32_t ring_size) {
        ctrl = (ctrl & ~DMA_CH0_CTRL_TRIG_RING_SEL_BITS) | (write << DMA_CH0_CTRL_TRIG_RING_SEL_LSB);
        ctrl = (ctrl & ~DMA_CH0_CTRL_TRIG_RING_SIZE_BITS) | (ring_size << DMA_CH0_CTRL_TRIG_RING_SIZE_LSB);
    }

    constexpr void set_bswap(bool bswap) {
        ctrl = (ctrl & ~DMA_CH0_CTRL_TRIG_BSWAP_BITS) | (bswap << DMA_CH0_CTRL_TRIG_BSWAP_LSB);
    }

    constexpr void set_irq_quiet(bool quiet) {
        ctrl = (ctrl & ~DMA_CH0_CTRL_TRIG_IRQ_QUIET_BITS) | (quiet << DMA_CH0_CTRL_TRIG_IRQ_QUIET_LSB);
    }

    constexpr void set_enable(bool enable) {
        ctrl = (ctrl & ~DMA_CH0_CTRL_TRIG_EN_BITS) | (enable << DMA_CH0_CTRL_TRIG_EN_LSB);
    }

    constexpr void set_sniff_enable(bool enable) {
        ctrl = (ctrl & ~DMA_CH0_CTRL_TRIG_SNIFF_EN_BITS) | (enable << DMA_CH0_CTRL_TRIG_SNIFF_EN_LSB);
    }
};

namespace iovec {
    constexpr iovec<6> io_dma_channel_configure(uint32_t chan, dma_channel_config cfg,
        iovec_value write_addr, iovec_value read_addr, uint32_t transfer_count, bool trigger) {
        uint32_t chan_base = DMA_BASE + sizeof (dma_channel_hw_t) * chan;
        return trigger ?
            io_memcpy(chan_base, 4,
                read_addr,
                write_addr,
                transfer_count,
                cfg.ctrl
            ) :
            io_memcpy(chan_base, 8,
                read_addr,
                transfer_count,
                cfg.ctrl,
                write_addr
            )
        ;
    }

    constexpr iovec<2> io_dma_start_channel_mask(uint32_t chan_mask) {
        return io_write(DMA_BASE + DMA_MULTI_CHAN_TRIGGER_OFFSET, chan_mask);
    }

    constexpr iovec<2> io_dma_channel_set_irq0_mask_enabled(uint32_t channel_mask, bool enabled) {
        if (enabled) {
            return io_write(BSET(DMA_BASE + DMA_INTE0_OFFSET), channel_mask);
        } else {
            return io_write(BCLR(DMA_BASE + DMA_INTE0_OFFSET), channel_mask);
        }
    }

    constexpr iovec<2> io_dma_channel_set_irq1_mask_enabled(uint32_t channel_mask, bool enabled) {
        if (enabled) {
            return io_write(BSET(DMA_BASE + DMA_INTE1_OFFSET), channel_mask);
        } else {
            return io_write(BCLR(DMA_BASE + DMA_INTE1_OFFSET), channel_mask);
        }
    }
}

}
