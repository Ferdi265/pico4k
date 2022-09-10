#pragma once

#include <cstddef>
#include <cstdint>
#include <usdk/util/array.h>
#include <hardware/regs/addressmap.h>

namespace usdk {

namespace iovec {

struct iovec_value {
    union {
        uint32_t number;
        const volatile void * ptr;
    };

    enum iovec_value_type {
        NUMBER,
        PTR
    };

    iovec_value_type type;
    uint32_t flags;

    constexpr iovec_value() : iovec_value(uint32_t(0)) {}
    constexpr iovec_value(uint32_t n) : number(n), type(NUMBER), flags(0) {}
    template <typename T>
    constexpr iovec_value(T * p) : ptr(p), type(PTR), flags(0) {}

    constexpr iovec_value operator|(uint32_t flag) const {
        iovec_value copy = *this;
        copy.flags |= flag;
        return copy;
    }

    constexpr uint32_t realize() const {
        // this is not actually constexpr
        // but the compiler will optimize it for us as if it were
        return (type == NUMBER ? number : (uint32_t)ptr) + flags;
    }
};

template <size_t N>
using iovec = array<iovec_value, N>;

template <auto meta_iovec>
struct realized_iovec : array<uint32_t, meta_iovec.size()> {
    using array<uint32_t, meta_iovec.size()>::operator[];

    constexpr static decltype(meta_iovec) meta = meta_iovec;
};

namespace detail {
    template <auto... values>
    struct array_pack {};

    template <size_t N, auto array, typename Pack = array_pack<>>
    struct array_as_pack;

    template <size_t N, auto array, typename Pack = array_pack<>>
    using array_as_pack_t = typename array_as_pack<N, array, Pack>::type;

    template <auto array, auto... pack_vals>
    struct array_as_pack<0, array, array_pack<pack_vals...>> {
        using type = array_pack<pack_vals...>;
    };

    template <size_t N, auto array, auto... pack_vals>
    struct array_as_pack<N, array, array_pack<pack_vals...>> {
        using type = array_as_pack_t<N - 1, array_span<1, N>(array), array_pack<pack_vals..., array[0]>>;
    };

    template <typename Pack>
    struct iovec_realize_pack;

    template <iovec_value... iovec_vals>
    struct iovec_realize_pack<array_pack<iovec_vals...>> {
        static inline const realized_iovec<iovec<sizeof...(iovec_vals)>{ iovec_vals... }> value = { iovec_vals.realize()... };
    };
}

template <auto iovec>
struct iovec_realize {
    static inline const realized_iovec<iovec> value = detail::iovec_realize_pack<detail::array_as_pack_t<iovec.size(), iovec>>::value;
};

template <auto iovec>
static inline const realized_iovec<iovec> iovec_realize_v = iovec_realize<iovec>::value;

enum iovec_cmd : uint32_t {
    IO_WRITE = 0,
    IO_WAIT = 1,
    IO_MEMSET = 2,
    IO_MEMCPY = 3
};

constexpr iovec_value BSET(iovec_value addr) { return addr | REG_ALIAS_SET_BITS; }
constexpr iovec_value BCLR(iovec_value addr) { return addr | REG_ALIAS_CLR_BITS; }
constexpr iovec_value BXOR(iovec_value addr) { return addr | REG_ALIAS_XOR_BITS; }

constexpr iovec<2> io_write(iovec_value addr, iovec_value value) {
    return to_array({ addr | IO_WRITE, value });
}

constexpr iovec<2> io_wait(iovec_value addr, iovec_value value) {
    return to_array({ addr | IO_WAIT, value });
}

constexpr iovec<3> io_memset(iovec_value addr, uint16_t stride, uint16_t len, iovec_value value) {
    return to_array({ addr | IO_MEMSET, iovec_value((stride << 16) | (len << 0)), value });
}

template <typename... Ts>
constexpr iovec<2 + sizeof...(Ts)> io_memcpy(iovec_value addr, uint16_t stride, Ts... values) {
    return to_array({ addr | IO_MEMCPY, iovec_value((stride << 16) | ((sizeof...(values)) << 0)), iovec_value(values)... });
}

template <typename T, size_t N>
constexpr iovec<2 + N> io_memcpy(iovec_value addr, uint16_t stride, array<T, N> values) {
    return array_concat(
        to_array({ addr | IO_MEMCPY, iovec_value((stride << 16) | (N << 0)) }),
        array_convert<iovec_value>(values)
    );
}

template <size_t N>
constexpr bool iovec_has_cmd(const iovec<N>& vec, iovec_cmd target_cmd) {
    const iovec_value * ptr = vec.begin();

    while (ptr != vec.end()) {
        if (ptr - vec.begin() >= N) __builtin_unreachable(); // out of bounds

        iovec_cmd cmd = (iovec_cmd)(ptr[0].flags & 3);
        if (cmd == target_cmd) return true;

        if (cmd == IO_WRITE) ptr += 2;
        else if (cmd == IO_WAIT) ptr += 2;
        else if (cmd == IO_MEMSET) ptr += 3;
        else if (cmd == IO_MEMCPY) ptr += 2 + (uint16_t)ptr[1].number;
        else __builtin_unreachable();
    }

    return false;
}

template <bool has_write, bool has_wait, bool has_memset, bool has_memcpy>
[[gnu::always_inline]] inline void iovec_apply_inline_generic(const uint32_t * ptr, const uint32_t * end) {
    while (ptr != end) {
        iovec_cmd cmd = (iovec_cmd)(ptr[0] & 3);
        uint32_t addr = ptr[0] & ~3;
        uint32_t val1 = ptr[1];

        if (has_write && cmd == IO_WRITE) {
            *(uint32_t *)addr = val1;
            ptr += 2;
        } else if (has_wait && cmd == IO_WAIT) {
            while (*(uint32_t *)addr != val1);
            ptr += 2;
        } else {
            uint16_t len = val1 >> 0;
            uint16_t stride = val1 >> 16;
            if (has_memset && cmd == IO_MEMSET) {
                uint32_t value = ptr[2];
                ptr += 3;
                do { *(uint32_t *)addr = value; addr += stride; } while (--len);
            } else if (has_memcpy && cmd == IO_MEMCPY) {
                ptr += 2;
                do { *(uint32_t *)addr = *ptr++; addr += stride; } while (--len);
            }
        }
    }
}

template <auto meta_iovec>
[[gnu::always_inline]] inline void iovec_apply_inline(const realized_iovec<meta_iovec>& iovec) {
    constexpr bool has_write = iovec_has_cmd(meta_iovec, IO_WRITE);
    constexpr bool has_wait = iovec_has_cmd(meta_iovec, IO_WAIT);
    constexpr bool has_memset = iovec_has_cmd(meta_iovec, IO_MEMSET);
    constexpr bool has_memcpy = iovec_has_cmd(meta_iovec, IO_MEMCPY);
    return iovec_apply_inline_generic<has_write, has_wait, has_memset, has_memcpy>(iovec.begin(), iovec.end());
}

[[gnu::noinline]] inline void iovec_apply(const uint32_t * ptr, const uint32_t * end) {
    iovec_apply_inline_generic<true, true, true, true>(ptr, end);
}

}

}
