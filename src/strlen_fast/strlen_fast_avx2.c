
#include "strlen_fast.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "x86_intrin.h"
#include "bitscan_forward.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4700)
#endif

size_t __FASTCALL strlen_fast_v1_avx2(const char * str)
{
    size_t len;
    register __m256i zero32, src32_low, src32_high;
    register size_t zero_mask_low, zero_mask_high;
    register uint64_t zero_mask;
    unsigned long zero_index;
    register const char * cur = str;
    // Get the misalignment bytes last 6 bits.
    size_t misalignment = (size_t)str & 0x3F;
    if (misalignment != 0) {
        // Scan the null terminator in first missalign bytes.
        for (uint32_t i = 0; i < (64U - (uint32_t)misalignment); ++i) {
            // Find out the null terminator.
            if (*cur++ == '\0') {
                return (size_t)i;
            }
        }

        // Align address to 64 bytes for main loop.
        cur = (const char *)((size_t)str & ((size_t)~(size_t)0x3F)) + 64;
    }
    // Set the zero masks (32 bytes).
    INIT_ZERO_32(zero32);
    zero32 = _mm256_xor_si256(zero32, zero32);

    // Main loop
    do {
        // Load the src 32 bytes to XMM register
        src32_high = _mm256_load_si256((__m256i *)(cur + 32));
        src32_low  = _mm256_load_si256((__m256i *)(cur));
        // Compare with zero32 masks per byte.
        src32_high = _mm256_cmpeq_epi8(src32_high, zero32);
        src32_low  = _mm256_cmpeq_epi8(src32_low,  zero32);
        // Package the compare result (32 bytes) to 16 bits.
        zero_mask_high = (size_t)_mm256_movemask_epi8(src32_high);
        zero_mask_low  = (size_t)_mm256_movemask_epi8(src32_low);
#if defined(_WIN64) || defined(WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__)
        // Combin the mask of the low 32 bits and high 32 bits.
        zero_mask = (zero_mask_high << 32) | zero_mask_low;

        // If it have any one bit is 1, mean it have a null terminator
        // inside this scaned strings (per 64 bytes).
        if (zero_mask != 0) {
            // Get the index of the first bit on set to 1.
            __BitScanForward64(zero_index, zero_mask);
            break;
        }
#else
        (void)zero_mask;
        // If it have any one bit is 1, mean it have a null terminator
        // inside this scaned strings (per 64 bytes).
        if (zero_mask_low != 0) {
            // Get the index of the first bit on set to 1.
            __BitScanForward(zero_index, zero_mask_low);
            break;
        }
        else if (zero_mask_high != 0) {
            // Get the index of the first bit on set to 1.
            __BitScanForward(zero_index, zero_mask_high);
            zero_index += 32;
            break;
        }
#endif // _M_X64 || __x86_64__
        // One loop scan 64 bytes.
        cur += 64;
    } while (1);

    len = cur - str;
    len += zero_index;
    return len;
}

size_t __FASTCALL strlen_fast_v2_avx2(const char * str)
{
    size_t len;
    register __m256i zero32, src32, src32_low, src32_high;
    register size_t zero_mask_low, zero_mask_high;
    register uint64_t zero_mask;
    unsigned long zero_index;
    register const char * cur = str;
    // Set the zero masks (32 bytes).
    INIT_ZERO_32(zero32);
    zero32 = _mm256_xor_si256(zero32, zero32);
    // Get the misalignment bytes last 6 bits.
    size_t misalignment = (size_t)cur & 0x3F;
    // If the misalignment bytes is < 16 bytes?
    if (misalignment < 0x20) {
        if (misalignment == 0) {
            // Align address to 32 bytes for main loop.
            cur = (const char *)((size_t)cur & ((size_t)~(size_t)0x3F)) - 64;
            goto main_loop;
        }
        // The misalignment bytes is less than 32 bytes.
        misalignment = (size_t)cur & 0x1F;
        // Align address to 16 bytes for XMM register.
        cur = (const char * )((size_t)cur & ((size_t)~(size_t)0x1F));
        // Load the src 32 bytes to XMM register
        src32 = _mm256_load_si256((__m256i *)(cur));
        // Compare with zero32 masks per byte.
        src32 = _mm256_cmpeq_epi8(src32, zero32);
        // Package the compare result (32 bytes) to 32 bits.
        zero_mask = (size_t)_mm256_movemask_epi8(src32);
        // Remove last missalign bits.
        zero_mask >>= (unsigned char)misalignment;
        zero_mask <<= (unsigned char)misalignment;

        if (zero_mask != 0) {
            // Get the index of the first bit on set to 1.
            __BitScanForward64(zero_index, zero_mask);
            goto strlen_exit;
        }

        // Align address to 64 bytes for main loop.
        cur = (const char *)((size_t)str & ((size_t)~(size_t)0x3F));
    }
    else {
        // Align address to 64 bytes for main loop.
        cur = (const char * )((size_t)cur & ((size_t)~(size_t)0x3F));
        // Load the src 32 bytes to XMM register
        src32_high = _mm256_load_si256((__m256i *)(cur + 32));
        src32_low  = _mm256_load_si256((__m256i *)(cur));
        // Compare with zero32 masks per byte.
        src32_high = _mm256_cmpeq_epi8(src32_high, zero32);
        src32_low  = _mm256_cmpeq_epi8(src32_low,  zero32);
        // Package the compare result (32 bytes) to 32 bits.
        zero_mask_high = (size_t)_mm256_movemask_epi8(src32_high);
        zero_mask_low  = (size_t)_mm256_movemask_epi8(src32_low);
#if defined(_WIN64) || defined(WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__)
        // Combin the mask of the low 32 bits and high 32 bits.
        zero_mask = (zero_mask_high << 32) | zero_mask_low;
        // Remove last misalignment bits.
        zero_mask >>= (unsigned char)misalignment;
        zero_mask <<= (unsigned char)misalignment;

        if (zero_mask != 0) {
            // Get the index of the first bit on set to 1.
            __BitScanForward64(zero_index, zero_mask);
            goto strlen_exit;
        }
#else
        // Combin the mask of the low 32 bits and high 32 bits.
        zero_mask = ((uint64_t)zero_mask_high << 32) | zero_mask_low;
        // Remove last misalignment bits.
        zero_mask >>= (unsigned char)misalignment;
        zero_mask <<= (unsigned char)misalignment;

        // If it have any one bit is 1, mean it have a null terminator
        // inside this scaned strings (per 64 bytes).
        zero_mask_low = zero_mask & 0xFFFFFFFFUL;
        if (zero_mask_low != 0) {
            // Get the index of the first bit on set to 1.
            __BitScanForward(zero_index, zero_mask_low);
            goto strlen_exit;
        }
        zero_mask_high = (zero_mask >> 32) & 0xFFFFFFFFUL;
        if (zero_mask_high != 0) {
            // Get the index of the first bit on set to 1.
            __BitScanForward(zero_index, zero_mask_high);
            zero_index += 32;
            goto strlen_exit;
        }
#endif // _M_X64 || __x86_64__
    }

main_loop:
    // Main loop
    do {
        // One loop scan 64 bytes.
        cur += 64;
        // Load the src 32 bytes to XMM register
        src32_high = _mm256_load_si256((__m256i *)(cur + 32));
        src32_low  = _mm256_load_si256((__m256i *)(cur));
        // Compare with zero32 masks per byte.
        src32_high = _mm256_cmpeq_epi8(src32_high, zero32);
        src32_low  = _mm256_cmpeq_epi8(src32_low,  zero32);
        // Package the compare result to 32 bits.
        zero_mask_high = (size_t)_mm256_movemask_epi8(src32_high);
        zero_mask_low  = (size_t)_mm256_movemask_epi8(src32_low);
#if defined(_WIN64) || defined(WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__)
        // Combin the mask of the low 32 bits and high 32 bits.
        zero_mask = (zero_mask_high << 32) | zero_mask_low;

        // If it have any one bit is 1, mean it have a null terminator
        // inside this scaned strings (per 32 bytes).
        if (zero_mask != 0) {
            // Get the index of the first bit on set to 1.
            __BitScanForward64(zero_index, zero_mask);
            break;
        }
#else
        (void)zero_mask;
        // If it have any one bit is 1, mean it have a null terminator
        // inside this scaned strings (per 64 bytes).
        if (zero_mask_low != 0) {
            // Get the index of the first bit on set to 1.
            __BitScanForward(zero_index, zero_mask_low);
            break;
        }
        else if (zero_mask_high != 0) {
            // Get the index of the first bit on set to 1.
            __BitScanForward(zero_index, zero_mask_high);
            zero_index += 32;
            break;
        }
#endif // _M_X64 || __x86_64__
    } while (1);

strlen_exit:
    len = cur - str;
    len += zero_index;
    return len;
}

#if defined(_WIN64) || defined(WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__)

size_t __FASTCALL strlen_fast_v1_avx2_x64(const char * str)
{
    size_t len;
    register __m256i zero32, src32_low, src32_high;
    register size_t zero_mask, zero_mask_low, zero_mask_high;
    unsigned long zero_index;
    register const char * cur = str;
    // Get the misalignment bytes last 6 bits.
    size_t misalignment = (size_t)str & 0x3F;
    if (misalignment != 0) {
        // Scan the null terminator in first missalign bytes.
        for (uint32_t i = 0; i < (64U - (uint32_t)misalignment); ++i) {
            // Find out the null terminator.
            if (*cur++ == '\0') {
                return (size_t)i;
            }
        }

        // Align address to 32 bytes for main loop.
        cur = (const char *)((size_t)str & ((size_t)~(size_t)0x3F)) + 64;
    }
    // Set the zero masks (32 bytes).
    INIT_ZERO_32(zero32);
    zero32 = _mm256_xor_si256(zero32, zero32);

    // Main loop
    do {
        // Load the src 16 bytes to XMM register
        src32_high = _mm256_load_si256((__m256i *)(cur + 16));
        src32_low  = _mm256_load_si256((__m256i *)(cur));
        // Compare with zero32 masks per byte.
        src32_high = _mm256_cmpeq_epi8(src32_high, zero32);
        src32_low  = _mm256_cmpeq_epi8(src32_low,  zero32);
        // Package the compare result (32 bytes) to 32 bits.
        zero_mask_high = (size_t)_mm256_movemask_epi8(src32_high);
        zero_mask_low  = (size_t)_mm256_movemask_epi8(src32_low);
#if defined(_WIN64) || defined(WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__)
        // Combin the mask of the low 32 bits and high 32 bits.
        zero_mask = (zero_mask_high << 32) | zero_mask_low;

        // If it have any one bit is 1, mean it have a null terminator
        // inside this scaned strings (per 64 bytes).
        if (zero_mask != 0) {
            // Get the index of the first bit on set to 1.
            __BitScanForward64(zero_index, zero_mask);
            break;
        }
#else
        (void)zero_mask;
        // If it have any one bit is 1, mean it have a null terminator
        // inside this scaned strings (per 64 bytes).
        if (zero_mask_low != 0) {
            // Get the index of the first bit on set to 1.
            __BitScanForward(zero_index, zero_mask_low);
            break;
        }
        else if (zero_mask_high != 0) {
            // Get the index of the first bit on set to 1.
            __BitScanForward(zero_index, zero_mask_high);
            zero_index += 32;
            break;
        }
#endif // _M_X64 || __x86_64__
        // One loop scan 64 bytes.
        cur += 64;

        // Load the src 16 bytes to XMM register
        src32_high = _mm256_load_si256((__m256i *)(cur + 16));
        src32_low  = _mm256_load_si256((__m256i *)(cur + 0));
        // Compare with zero32 masks per byte.
        src32_high = _mm256_cmpeq_epi8(src32_high, zero32);
        src32_low  = _mm256_cmpeq_epi8(src32_low,  zero32);
        // Package the compare result (32 bytes) to 32 bits.
        zero_mask_high = (size_t)_mm256_movemask_epi8(src32_high);
        zero_mask_low  = (size_t)_mm256_movemask_epi8(src32_low);

#if defined(_WIN64) || defined(WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__)
        // Combin the mask of the low 32 bits and high 32 bits.
        zero_mask = (zero_mask_high << 32) | zero_mask_low;

        // If it have any one bit is 1, mean it have a null terminator
        // inside this scaned strings (per 64 bytes).
        if (zero_mask != 0) {
            // Get the index of the first bit on set to 1.
            __BitScanForward64(zero_index, zero_mask);
            break;
        }
#else
        (void)zero_mask;
        // If it have any one bit is 1, mean it have a null terminator
        // inside this scaned strings (per 64 bytes).
        if (zero_mask_low != 0) {
            // Get the index of the first bit on set to 1.
            __BitScanForward(zero_index, zero_mask_low);
            break;
        }
        else if (zero_mask_high != 0) {
            // Get the index of the first bit on set to 1.
            __BitScanForward(zero_index, zero_mask_high);
            zero_index += 32;
            break;
        }
#endif // _M_X64 || __x86_64__
        // One loop scan 64 bytes.
        cur += 64;
    } while (1);

    len = cur - str;
    len += zero_index;
    return len;
}

#endif // _M_X64 || __x86_64__

size_t __FASTCALL strlen_fast_asm_avx2(const char * str)
{
    (void)str;
    return 0;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
