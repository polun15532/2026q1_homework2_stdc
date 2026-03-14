#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

static uint32_t float_bits(float value)
{
    uint32_t bits = 0;
    memcpy(&bits, &value, sizeof(bits));
    return bits;
}

static uint64_t double_bits(double value)
{
    uint64_t bits = 0;
    memcpy(&bits, &value, sizeof(bits));
    return bits;
}

static void print_u32_binary(uint32_t value)
{
    int i;

    for (i = 31; i >= 0; --i) {
        putchar((value >> i) & 1U ? '1' : '0');
        if (i == 31 || i == 23) {
            putchar(' ');
        }
    }
}

static void print_u64_binary(uint64_t value)
{
    int i;

    for (i = 63; i >= 0; --i) {
        putchar((value >> i) & 1ULL ? '1' : '0');
        if (i == 63 || i == 52) {
            putchar(' ');
        }
    }
}

int main(void)
{
    float f = 0.1f;
    double d = 0.1;
    uint32_t fbits = float_bits(f);
    uint64_t dbits = double_bits(d);
    uint32_t f_sign = (fbits >> 31) & 0x1U;
    uint32_t f_exponent = (fbits >> 23) & 0xffU;
    uint32_t f_fraction = fbits & 0x7fffffU;
    uint64_t d_sign = (dbits >> 63) & 0x1ULL;
    uint64_t d_exponent = (dbits >> 52) & 0x7ffULL;
    uint64_t d_fraction = dbits & 0x000fffffffffffffULL;

    printf("float  0.1f\n");
    printf("  hex      : 0x%08" PRIx32 "\n", fbits);
    printf("  bits     : ");
    print_u32_binary(fbits);
    printf("\n");
    printf("  sign     : %" PRIu32 "\n", f_sign);
    printf("  exponent : %" PRIu32 " (actual %d)\n",
           f_exponent, (int)f_exponent - 127);
    printf("  fraction : 0x%06" PRIx32 "\n", f_fraction);
    printf("\n");

    printf("double 0.1\n");
    printf("  hex      : 0x%016" PRIx64 "\n", dbits);
    printf("  bits     : ");
    print_u64_binary(dbits);
    printf("\n");
    printf("  sign     : %" PRIu64 "\n", d_sign);
    printf("  exponent : %" PRIu64 " (actual %d)\n",
           d_exponent, (int)d_exponent - 1023);
    printf("  fraction : 0x%013" PRIx64 "\n", d_fraction);

    return 0;
}
