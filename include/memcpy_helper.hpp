#ifndef MEMCPY_HELPER_HPP
#define MEMCPY_HELPER_HPP



// if x86_64
#include <cstddef>
#include <cstring>

#if defined(__x86_64__) || defined(_M_X64)
#include <immintrin.h>
void fast_memcpy(void* dest, const void* src, size_t size) {
    size_t i = 0;
    for (; i + 32 <= size; i += 32) {
        __m256i chunk = _mm256_loadu_si256((__m256i*)((char*)src + i));
        _mm256_storeu_si256((__m256i*)((char*)dest + i), chunk);
    }
    // 处理剩余不足32字节的部分
    if (i < size) {
        memcpy((char*)dest + i, (char*)src + i, size - i);
    }
}
#endif

#if defined(__aarch64__) || defined(__arm__)
#include <arm_neon.h>
void fast_memcpy(void* dest, const void* src, size_t size) {
    size_t i = 0;
    for (; i + 16 <= size; i += 16) {
        uint8x16_t chunk = vld1q_u8((const uint8_t*)((char*)src + i));
        vst1q_u8((uint8_t*)((char*)dest + i), chunk);
    }
    // 处理剩余不足16字节的部分
    if (i < size) {
        memcpy((char*)dest + i, (char*)src + i, size - i);
    }
}
#endif

#endif