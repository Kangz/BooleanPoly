#include "bit_utils.h"

#define USE_BUILTINS 1

int log2_u32(uint32_t v) {
    #if USE_BUILTINS
        if (v == 0) {
            return 0;
        } else {
            return 31 - __builtin_clz(v);
        }
    #else
        static const int MultiplyDeBruijnBitPosition[32] = {
            0, 9, 1, 10, 13, 21, 2, 29, 11, 14, 16, 18, 22, 25, 3, 30, 8, 12, 20, 28, 15, 17, 24, 7, 19, 27, 23, 6, 26, 5, 4, 31
        };

        v |= v >> 1; // first round down to one less than a power of 2 
        v |= v >> 2;
        v |= v >> 4;
        v |= v >> 8;
        v |= v >> 16;

        return MultiplyDeBruijnBitPosition[(uint32_t)(v * 0x07C4ACDDU) >> 27];
    #endif
}

int log2_u64(uint64_t v) {
    #if USE_BUILTINS
        if (v == 0) {
            return 0;
        } else {
            return 63 - __builtin_clzl(v);
        }
    #else
        uint32_t top = v >> 32;
        if (top == 0) {
            return log2_u32(v);
        } else {
            return 32 + log2_u32(top);
        }
    #endif
}

// Interleave the bits of a and b, bits of a are in even position and the bits of b in the odds
uint64_t interleave_32_64(uint32_t a, uint32_t b) {
    static const uint64_t B[] = {0x5555555555555555, 0x3333333333333333, 0x0F0F0F0F0F0F0F0F, 0x00FF00FF00FF00FF, 0x0000FFFF0000FFFF};
    static const uint64_t S[] = {1, 2, 4, 8, 16};

    uint64_t x = a;
    uint64_t y = b;

    x = (x | (x << S[4])) & B[4];
    x = (x | (x << S[3])) & B[3];
    x = (x | (x << S[2])) & B[2];
    x = (x | (x << S[1])) & B[1];
    x = (x | (x << S[0])) & B[0];

    y = (y | (y << S[4])) & B[4];
    y = (y | (y << S[3])) & B[3];
    y = (y | (y << S[2])) & B[2];
    y = (y | (y << S[1])) & B[1];
    y = (y | (y << S[0])) & B[0];

    return x | (y << 1);
}

uint32_t interleave_16_32(uint32_t a, uint32_t b) {
    static const unsigned int B[] = {0x55555555, 0x33333333, 0x0F0F0F0F, 0x00FF00FF};
    static const unsigned int S[] = {1, 2, 4, 8};

    unsigned int x = a;
    unsigned int y = b;

    x = (x | (x << S[3])) & B[3];
    x = (x | (x << S[2])) & B[2];
    x = (x | (x << S[1])) & B[1];
    x = (x | (x << S[0])) & B[0];

    y = (y | (y << S[3])) & B[3];
    y = (y | (y << S[2])) & B[2];
    y = (y | (y << S[1])) & B[1];
    y = (y | (y << S[0])) & B[0];

    return x | (y << 1);
}

uint64_t convolution_32_64(uint32_t a, uint32_t b) {
    uint32_t resOdd = 0;
    uint32_t resEven = a & b;

    for (int i = 2; i < 32; i+= 2) {
        uint32_t temp = 0;
        temp ^= (a >> i) & b;
        temp ^= a & (b >> i);
        resEven ^= temp << (i/2);
    }

    for (int i = 1; i < 32; i+= 2) {
        uint32_t temp = 0;
        temp ^= (a >> i) & b;
        temp ^= a & (b >> i);
        resOdd ^= temp << (i/2);
    }

    return interleave_32_64(resEven, resOdd);
}

uint32_t convolution_16_32(uint16_t a, uint16_t b) {
    uint32_t resOdd = 0;
    uint32_t resEven = a & b;

    for (int i = 2; i < 16; i+= 2) {
        uint32_t temp = 0;
        temp ^= (a >> i) & b;
        temp ^= a & (b >> i);
        resEven ^= temp << (i/2);
    }

    for (int i = 1; i < 16; i+= 2) {
        uint32_t temp = 0;
        temp ^= (a >> i) & b;
        temp ^= a & (b >> i);
        resOdd ^= temp << (i/2);
    }

    return interleave_16_32(resEven, resOdd);
}
