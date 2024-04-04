#pragma once

#include <cstddef>
#include <cstdint>

namespace usdk {

[[gnu::always_inline]] inline constexpr void strcpy(char *dest, const char *src) {
    char* dest8 = dest;
    const char* src8 = src;

    while (*src8) *dest8++ = *src8++;
}

[[gnu::noinline]] inline constexpr void strcpy_noinline(char *dest, const char *src) {
    strcpy(dest, src);
}

[[gnu::always_inline]] inline void memmove(void *dest, const void *src, size_t length) {
    uint8_t* dest8 = (uint8_t*) dest;
    const uint8_t* src8 = (const uint8_t*) src;

    if (src > dest) {
        while (length--) *dest8++ = *src8++;
    } else {
        src8 += length - 1;
        dest8 += length - 1;
        while (length--) *dest8-- = *src8--;
    }
}

[[gnu::noinline]] inline void memmove_noinline(void *dest, const void *src, size_t length) {
    memmove(dest, src, length);
}

}
