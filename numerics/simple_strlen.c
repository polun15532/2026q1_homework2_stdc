#include <stdint.h>
#include "simple_strlen.h"

static inline uint32_t has_zero_u32(uint32_t x)
{
    return ((x - 0x01010101U) & ~x & 0x80808080U);
}

__attribute__((noinline, noipa))
size_t single_byte_strlen(const unsigned char *s)
{
    size_t i;

    for (i = 0;; ++i) {
        if (s[i] == 0U) {
            return i;
        }
    }
}

__attribute__((noinline, noipa))
size_t four_byte_strlen(const unsigned char *s)
{
    const unsigned char *p = s;
    const uint32_t *w;

    while (((uintptr_t)p & (sizeof(uint32_t) - 1U)) != 0U) {
        if (*p == 0U) {
            return (size_t)(p - s);
        }
        ++p;
    }

    w = (const uint32_t *)(const void *)p;
    for (;;) {
        uint32_t x = *w;
        uint32_t bits = has_zero_u32(x);

        if (bits == 0U) {
            ++w;
            continue;
        }

        p = (const unsigned char *)(const void *)w;
        return (size_t)(p - s) + ((size_t)__builtin_ctz(bits) >> 3);
    }
}
