#pragma once

#include <cstddef>
#include <type_traits>

namespace usdk {

template <typename T>
constexpr std::make_unsigned_t<T> mask_nbit(T n) {
    return (std::make_unsigned_t<T>)(-1) >> ((8 * sizeof (T)) - n);
}

template <typename T>
constexpr T log2(T t) {
    if (t == 0) return 0;

    size_t log = -1;
    while (t) {
        log++;
        t >>= 1;
    }
    return log;
}

}
