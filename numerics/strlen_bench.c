#define _POSIX_C_SOURCE 200809L
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "simple_strlen.h"

static volatile size_t sink;
#define STRLEN_BENCH_COMPARE 0
#define STRLEN_BENCH_SINGLE 1
#define STRLEN_BENCH_FOUR 2
#ifndef STRLEN_BENCH_MODE
#define STRLEN_BENCH_MODE STRLEN_BENCH_COMPARE
#endif
#ifndef STRLEN_BENCH_LEN
#define STRLEN_BENCH_LEN (1024U * 1024U)
#endif
#ifndef STRLEN_BENCH_BYTES
#define STRLEN_BENCH_BYTES (64U * 1024U * 1024U)
#endif
#ifndef STRLEN_BENCH_SCRUB
#define STRLEN_BENCH_SCRUB (64U * 1024U * 1024U)
#endif

static uint64_t now_ns(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

static unsigned char *make_buf(size_t len, unsigned char fill)
{
    unsigned char *p;
    size_t i;

    if (posix_memalign((void **)&p, 64U, len + 4U) != 0) {
        exit(EXIT_FAILURE);
    }
    for (i = 0; i < len; ++i) {
        p[i] = fill;
    }
    memset(p + len, 0, 4U);
    return p;
}

static void scrub_cache(const unsigned char *p)
{
    size_t i;
    for (i = 0; i < STRLEN_BENCH_SCRUB; i += 64U) {
        sink += p[i];
    }
}

static double bench(size_t (*fn)(const unsigned char *), const unsigned char *s, const unsigned char *scrub)
{
    size_t repeat = STRLEN_BENCH_BYTES / STRLEN_BENCH_LEN;
    uint64_t start;
    size_t i;

    if (repeat == 0U) {
        repeat = 1U;
    }
    scrub_cache(scrub);
    start = now_ns();
    for (i = 0; i < repeat; ++i) {
        sink += fn(s);
    }
    return (double)(now_ns() - start) / (double)repeat;
}

int main(void)
{
    unsigned char *s = make_buf(STRLEN_BENCH_LEN, 0x41U);
    unsigned char *scrub = make_buf(STRLEN_BENCH_SCRUB, 0x5aU);
#if STRLEN_BENCH_MODE == STRLEN_BENCH_SINGLE
    double t = bench(single_byte_strlen, s, scrub);
    printf("single_byte len=%zu ns/call=%.2f ns/byte=%.4f\n", (size_t)STRLEN_BENCH_LEN, t, t / (double)STRLEN_BENCH_LEN);
#elif STRLEN_BENCH_MODE == STRLEN_BENCH_FOUR
    double t = bench(four_byte_strlen, s, scrub);
    printf("four_byte len=%zu ns/call=%.2f ns/byte=%.4f\n", (size_t)STRLEN_BENCH_LEN, t, t / (double)STRLEN_BENCH_LEN);
#else
    double a = bench(single_byte_strlen, s, scrub);
    double b = bench(four_byte_strlen, s, scrub);
    printf("single_byte len=%zu ns/call=%.2f ns/byte=%.4f\n", (size_t)STRLEN_BENCH_LEN, a, a / (double)STRLEN_BENCH_LEN);
    printf("four_byte   len=%zu ns/call=%.2f ns/byte=%.4f\n", (size_t)STRLEN_BENCH_LEN, b, b / (double)STRLEN_BENCH_LEN);
#endif
    free(scrub);
    free(s);
    return sink == 0U;
}
