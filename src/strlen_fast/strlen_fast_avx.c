
#include "strlen_fast.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <assert.h>

#include "x86_intrin.h"
#include "bitscan_forward.h"

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4700)
#endif

size_t __FASTCALL strlen_fast_v1a_avx(const char * str)
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
        src32_low  = _mm256_load_si256((__m256i *)(cur));
        src32_high = _mm256_load_si256((__m256i *)(cur + 32));
        // Compare with zero32 masks per byte.
        src32_low  = _mm256_cmpeq_epi8(src32_low,  zero32);
        src32_high = _mm256_cmpeq_epi8(src32_high, zero32);
        // Package the compare result (32 bytes) to 16 bits.
        zero_mask_low  = (size_t)_mm256_movemask_epi8(src32_low);
        zero_mask_high = (size_t)_mm256_movemask_epi8(src32_high);
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

size_t __FASTCALL strlen_fast_v1b_avx(const char * str)
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
        misalignment = (size_t)str & 0x1F;
        // Scan the null terminator in first missalign bytes.
        register const char * end = cur + ((size_t)16UL - misalignment);
        while (cur < end) {
            // Find out the null terminator.
            if (*cur == '\0') {
                return (size_t)(cur - str);
            }
            cur++;
        }

        // Align address to 64 bytes for main loop.
        end = (const char *)((size_t)str & ((size_t)~(size_t)0x3F)) + 64;

        register __m128i zero16, src16;
        register uint32_t zero_mask16;

        // Set the zero masks (16 bytes).
        INIT_ZERO_16(zero16);
        zero16 = _mm_xor_si128(zero16, zero16);

        // Minor 16 bytes loop
        while (cur < end) {
            // Load the src 16 bytes to XMM register
            src16  = _mm_load_si128((__m128i *)(cur));
            // Compare with zero16 masks per byte.
            src16  = _mm_cmpeq_epi8(src16,  zero16);
            // Package the compare result (16 bytes) to 16 bits.
            zero_mask16 = (uint32_t)_mm_movemask_epi8(src16);

            // If it have any one bit is 1, mean it have a null terminator
            // inside this scaned strings (per 16 bytes).
            if (zero_mask16 != 0) {
                // Get the index of the first bit on set to 1.
                __BitScanForward(zero_index, zero_mask16);
                goto strlen_exit;
            }
            // One minor loop scan 16 bytes.
            cur += 16;
        }
    }
    // Set the zero masks (32 bytes).
    INIT_ZERO_32(zero32);
    zero32 = _mm256_xor_si256(zero32, zero32);

    // Main loop
    do {
        // Load the src 32 bytes to XMM register
        src32_low  = _mm256_load_si256((__m256i *)(cur));
        src32_high = _mm256_load_si256((__m256i *)(cur + 32));
        // Compare with zero32 masks per byte.
        src32_low  = _mm256_cmpeq_epi8(src32_low,  zero32);
        src32_high = _mm256_cmpeq_epi8(src32_high, zero32);
        // Package the compare result (32 bytes) to 16 bits.
        zero_mask_low  = (size_t)_mm256_movemask_epi8(src32_low);
        zero_mask_high = (size_t)_mm256_movemask_epi8(src32_high);
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

strlen_exit:
    len = cur - str;
    len += zero_index;
    return len;
}

size_t __FASTCALL strlen_fast_v2_avx(const char * str)
{
    size_t len;
    register __m256i zero32, src32_low, src32_high;
    register size_t zero_mask_low, zero_mask_high;
    register uint64_t zero_mask;
    unsigned long zero_index;
    register const char * cur = str;
    // Set the zero masks (32 bytes).
    INIT_ZERO_32(zero32);
    zero32 = _mm256_xor_si256(zero32, zero32);
    // Get the misalignment bytes last 6 bits.
    size_t misalignment = (size_t)cur & 0x3F;
    // If the misalignment bytes is < 32 bytes?
    if (misalignment < 0x20) {
        if (misalignment == 0) {
            // If misalignment is 0, skip this step.
            goto main_loop;
        }
        // Align address to 64 bytes for main loop.
        cur = (const char * )((size_t)cur & ((size_t)~(size_t)0x3F));
        // Load 32 bytes from target string to YMM register.
        src32_low  = _mm256_load_si256((__m256i *)(cur));
        src32_high = _mm256_load_si256((__m256i *)(cur + 32));
        // Compare with zero32 masks per byte.
        src32_low  = _mm256_cmpeq_epi8(src32_low,  zero32);
        src32_high = _mm256_cmpeq_epi8(src32_high, zero32);
        // Package the compare result (32 bytes) to 32 bits.
        zero_mask_low  = (size_t)_mm256_movemask_epi8(src32_low);
        zero_mask_high = (size_t)_mm256_movemask_epi8(src32_high);
        // Remove last missalign bits.
        zero_mask_low >>= misalignment;
        zero_mask_low <<= misalignment;
        if (zero_mask_low != 0) {
            // Get the index of the first bit on set to 1.
            __BitScanForward32(zero_index, zero_mask_low);
            goto strlen_exit;
        }
        if (zero_mask_high != 0) {
            // Get the index of the first bit on set to 1.
            __BitScanForward32(zero_index, zero_mask_high);
            zero_index += 32;
            goto strlen_exit;
        }
        // Align address to the next 64 bytes for main loop.
        cur += 64;
    }
    else {
        // Align address to 64 bytes, and offset 32 bytes for misalignment.
        cur = (const char * )((size_t)cur & ((size_t)~(size_t)0x3F));

        // Load the src 32 bytes to XMM register
        src32_high = _mm256_load_si256((__m256i *)(cur + 32));
        // Compare with zero32 masks per byte.
        src32_high = _mm256_cmpeq_epi8(src32_high, zero32);
        // Package the compare result (32 bytes) to 32 bits.
        zero_mask_high = (size_t)_mm256_movemask_epi8(src32_high);
        // Skip 32 bytes.
        misalignment -= 32;
        // Remove last misalignment bits.
        zero_mask_high >>= misalignment;
        zero_mask_high <<= misalignment;

        // If it have any one bit is 1, mean it have a null terminator
        // inside this scaned strings (per 64 bytes).
        if (zero_mask_high != 0) {
            // Get the index of the first bit on set to 1.
            __BitScanForward(zero_index, zero_mask_high);
            zero_index += 32;
            goto strlen_exit;
        }
        // Align address to the next 64 bytes for main loop.
        cur += 64;
    }

main_loop:
    // Main loop
    do {
        // Load the src 32 bytes to XMM register
        src32_low  = _mm256_load_si256((__m256i *)(cur));
        src32_high = _mm256_load_si256((__m256i *)(cur + 32));
        // Compare with zero32 masks per byte.
        src32_low  = _mm256_cmpeq_epi8(src32_low,  zero32);
        src32_high = _mm256_cmpeq_epi8(src32_high, zero32);
        // Package the compare result to 32 bits.
        zero_mask_low  = (size_t)_mm256_movemask_epi8(src32_low);
        zero_mask_high = (size_t)_mm256_movemask_epi8(src32_high);
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
        if (zero_mask_high != 0) {
            // Get the index of the first bit on set to 1.
            __BitScanForward(zero_index, zero_mask_high);
            zero_index += 32;
            break;
        }
#endif // _M_X64 || __x86_64__
        // One loop scan 64 bytes.
        cur += 64;
    } while (1);

strlen_exit:
    len = cur - str;
    len += zero_index;
    return len;
}

#if defined(_WIN64) || defined(WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__)

size_t __FASTCALL strlen_fast_v1_avx_x64(const char * str)
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

size_t __FASTCALL strlen_fast_asm_avx(const char * str)
{
    (void)str;
    return 0;
}

#ifdef _MSC_VER
#pragma warning(pop)
#endif
