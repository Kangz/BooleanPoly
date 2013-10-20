#ifndef BIT_UTILS_H
#define BIT_UTILS_H

#include <cstdint>

int log2_u32(uint32_t v);
int log2_u64(uint64_t v);

uint64_t interleave_32_64(uint32_t a, uint32_t b);
uint32_t interleave_16_32(uint32_t a, uint32_t b);

uint64_t convolution_32_64(uint32_t a, uint32_t b);
uint32_t convolution_16_32(uint16_t a, uint16_t b);

#endif //BIT_UTILS_H
