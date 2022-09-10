#pragma once

#include <cstddef>
#include <cstdint>
#include <usdk/util/mask.h>
#include <usdk/interp.h>

// --- interp lerp -------------------------------------------------------------

[[gnu::pure]] inline uint32_t interp_lerp(uint32_t a, uint32_t b, uint8_t frac) {
      interp0->accum[0] = frac;
      interp0->base[0] = a;
      interp0->base[1] = b;
      return interp0->peek[1];
}

[[gnu::pure]] inline uint32_t interp_table_lerp(uint32_t index) {
      interp0->accum[0] = index;
      uint32_t * table_entry = (uint32_t *)interp0->peek[2];
      interp0->base[0] = table_entry[0];
      interp0->base[1] = table_entry[1];
      return interp0->peek[1];
}

// table of exp2 values between 0 and 1 in U16.16 fixed point
// for arg in U18.14 fixed point with 8 bit lerp
constexpr uint32_t exp2_01_table[] = {
    0x00010000, 0x000102c9, 0x0001059b, 0x00010874,
    0x00010b55, 0x00010e3e, 0x00011130, 0x00011429,
    0x0001172b, 0x00011a35, 0x00011d48, 0x00012063,
    0x00012387, 0x000126b4, 0x000129e9, 0x00012d28,
    0x0001306f, 0x000133c0, 0x0001371a, 0x00013a7d,
    0x00013dea, 0x00014160, 0x000144e0, 0x0001486a,
    0x00014bfd, 0x00014f9b, 0x00015342, 0x000156f4,
    0x00015ab0, 0x00015e76, 0x00016247, 0x00016623,
    0x00016a09, 0x00016dfb, 0x000171f7, 0x000175fe,
    0x00017a11, 0x00017e2f, 0x00018258, 0x0001868d,
    0x00018ace, 0x00018f1a, 0x00019373, 0x000197d8,
    0x00019c49, 0x0001a0c6, 0x0001a550, 0x0001a9e6,
    0x0001ae89, 0x0001b33a, 0x0001b7f7, 0x0001bcc1,
    0x0001c199, 0x0001c67f, 0x0001cb72, 0x0001d072,
    0x0001d581, 0x0001da9e, 0x0001dfc9, 0x0001e502,
    0x0001ea4a, 0x0001efa1, 0x0001f507, 0x0001fa7c,
    0x00020000
};

constexpr size_t EXP_FRAC_BITS = 14;
constexpr size_t RES_FRAC_BITS = 16;
constexpr size_t TABLE_VAL_BITS = 2;
constexpr size_t TABLE_INDEX_BITS = 6;

// set up interpolator for table lookup
// for arg in U18.14 fixed point with 8 bit lerp
// for value in U16.16 fixed point
constexpr auto exp2_interp_config() {
    using namespace usdk::iovec;

    usdk::interp_config cfg0;
    cfg0.set_shift(EXP_FRAC_BITS - TABLE_INDEX_BITS - TABLE_VAL_BITS);
    cfg0.set_mask(TABLE_VAL_BITS, TABLE_VAL_BITS + TABLE_INDEX_BITS - 1);
    cfg0.set_blend(true);

    usdk::interp_config cfg1;
    cfg1.set_mask(0, 7);
    cfg1.set_signed(true);
    cfg1.set_cross_input(true);

    return usdk::array_concat(
        io_interp_set_config(0, 0, cfg0),
        io_interp_set_config(0, 1, cfg1),
        io_interp_set_base(0, 2, exp2_01_table)
    );
}

// arg is in U18.14 fixed point
// result is in U16.16 fixed point
[[gnu::pure]] inline uint32_t fixed_point_exp2(uint32_t arg) {
    uint32_t int_part = arg >> EXP_FRAC_BITS;
    uint32_t frac_exp = interp_table_lerp(arg);
    return frac_exp << int_part;
}

// --- sin impl ----------------------------------------------------------------

constexpr int16_t sin_table[] = {
     0x0000,  0x0645,  0x0c7c,  0x1294,  0x187d,  0x1e2b,  0x238e,  0x2899,
     0x2d41,  0x3179,  0x3536,  0x3871,  0x3b20,  0x3d3e,  0x3ec5,  0x3fb1,
     0x4000,  0x3fb1,  0x3ec5,  0x3d3e,  0x3b20,  0x3871,  0x3536,  0x3179,
     0x2d41,  0x2899,  0x238e,  0x1e2b,  0x187d,  0x1294,  0x0c7c,  0x0645,
     0x0000,
};

namespace sin_config {
    constexpr size_t index_bits = usdk::log2(usdk::array_length(sin_table) - 1) + 1;
    constexpr size_t degree_bits = usdk::log2(sin_table[1 << (index_bits - 2)]);
    constexpr size_t addr_offset_bits = usdk::log2(sizeof (sin_table[0]));
    constexpr size_t frac_bits = degree_bits - index_bits;
    constexpr size_t shl_bits = frac_bits < 8 ? 8 - frac_bits : 0;
    constexpr size_t shr_bits = frac_bits > 8 ? frac_bits - 8 : 0;
}

constexpr auto sin_interp_config() {
    using namespace usdk::iovec;
    using namespace sin_config;

    usdk::interp_config cfg0;
    cfg0.set_shift(frac_bits + shl_bits - addr_offset_bits);
    cfg0.set_mask(addr_offset_bits, index_bits + addr_offset_bits - 2);
    cfg0.set_blend(true);

    usdk::interp_config cfg1;
    cfg1.set_shift(shr_bits);
    cfg1.set_mask(shl_bits, 7);
    cfg1.set_signed(true);
    cfg1.set_cross_input(true);

    return usdk::array_concat(
        io_interp_set_config(0, 0, cfg0),
        io_interp_set_config(0, 1, cfg1),
        io_interp_set_base(0, 2, sin_table)
    );
}

[[gnu::pure]] inline int sin(int angle) {
    using namespace sin_config;

    interp0->accum[0] = angle << shl_bits;
    const int16_t * entry = (const int16_t *)interp0->peek[2];
    interp0->base[0] = (int32_t)entry[0];
    interp0->base[1] = (int32_t)entry[1];
    int value = interp0->peek[1];
    return angle & (1 << (degree_bits - 1)) ? -value : value;
}

[[gnu::always_inline]] inline int cos(int angle) {
    using namespace sin_config;
    return sin(angle + (1 << (degree_bits - 2)));
}

// --- clamp -------------------------------------------------------------------

constexpr auto clamp_interp_config() {
    using namespace usdk::iovec;
    using namespace sin_config;

    usdk::interp_config cfg0;
    cfg0.set_shift(0);
    cfg0.set_clamp(true);
    cfg0.set_signed(true);

    return usdk::array_concat(
        io_interp_set_config(1, 0, cfg0),
        io_interp_set_base(1, 0, 0),
        io_interp_set_base(1, 1, 479)
    );
}

[[gnu::pure]] inline int clamp(int value) {
    interp1->accum[0] = value;
    return interp1->peek[0];
}
