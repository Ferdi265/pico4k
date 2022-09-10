#pragma once

#include <cstddef>
#include <cstdint>

namespace usdk {

[[gnu::noinline]] void * memmove(void *dest, const void *src, size_t length) {
    uint8_t* dest8 = (uint8_t*) dest;
    const uint8_t* src8 = (const uint8_t*) src;

    if (src > dest) {
        while (length--) *dest8++ = *src8++;
    } else {
        src8 += length - 1;
        dest8 += length - 1;
        while (length--) *dest8-- = *src8--;
    }

    return dest;
}

}
