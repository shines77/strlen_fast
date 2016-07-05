
#include "strlen_fast.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "bitscan_forward.h"

size_t __FASTCALL strlen_fast_v1_sse2(const char * str)
{
    size_t len;
    register __m128i zero16, src16_low, src16_high;
    register size_t zero_mask, zero_mask_low, zero_mask_high;
    unsigned long zero_index;
    register const char * cur = str;
    // Get the misalignment bytes last 5 bits.
    size_t misalignment = (size_t)str & 0x1F;
    if (misalignment != 0) {
        // Scan the null terminator in first missalign bytes.
        for (uint32_t i = 0; i < (32U - (uint32_t)misalignment); ++i) {
            // Find out the null terminator.
            if (*cur++ == '\0') {
                return (size_t)i;
            }
        }

        // Align address to 32 bytes for main loop.
        cur = (const char *)((size_t)str & ((size_t)~(size_t)0x1F)) + 32;
    }
    // Set the zero masks (16 bytes).
    INIT_ZERO_16(zero16);
    zero16 = _mm_xor_si128(zero16, zero16);

    // Main loop
    do {
        // Load the src 16 bytes to XMM register
        src16_high = _mm_load_si128((__m128i *)(cur + 16));
        src16_low  = _mm_load_si128((__m128i *)(cur));
        // Compare with zero16 masks per byte.
        src16_high = _mm_cmpeq_epi8(src16_high, zero16);
        src16_low  = _mm_cmpeq_epi8(src16_low,  zero16);
        // Package the compare result (16 bytes) to 16 bits.
        zero_mask_high = (size_t)_mm_movemask_epi8(src16_high);
        zero_mask_low  = (size_t)_mm_movemask_epi8(src16_low);
        // Combin the mask of the low 16 bits and high 16 bits.
        zero_mask = (zero_mask_high << 16) | zero_mask_low;

        // If it have any one bit is 1, mean it have a null terminator
        // inside this scaned strings (per 32 bytes).
        if (zero_mask != 0) {
            // Get the index of the first bit on set to 1.
            __BitScanForward(zero_index, zero_mask);
            break;
        }
        // One loop scan 32 bytes.
        cur += 32;
    } while (1);

    len = cur - str;
    len += zero_index;
    return len;
}

size_t __FASTCALL strlen_fast_v2_sse2(const char * str)
{
    size_t len;
    register __m128i zero16, src16, src16_low, src16_high;
    register size_t zero_mask, zero_mask_low, zero_mask_high;
    unsigned long zero_index;
    register const char * cur = str;
    // Set the zero masks (16 bytes).
    INIT_ZERO_16(zero16);
    zero16 = _mm_xor_si128(zero16, zero16);
    // Get the misalignment bytes last 5 bits.
    size_t misalignment = (size_t)cur & 0x1F;
    // If the misalignment bytes is < 16 bytes?
    if (misalignment < 0x10) {
        if (misalignment == 0) {
            // Align address to 32 bytes for main loop.
            cur = (const char *)((size_t)cur & ((size_t)~(size_t)0x1F)) - 32;
            goto main_loop;
        }
        // The misalignment bytes is less than 16 bytes.
        misalignment = (size_t)cur & 0x0F;
        // Align address to 16 bytes for XMM register.
        cur = (const char * )((size_t)cur & ((size_t)~(size_t)0x0F));
        // Load the src 16 bytes to XMM register
        src16 = _mm_load_si128((__m128i *)(cur));
        // Compare with zero16 masks per byte.
        src16 = _mm_cmpeq_epi8(src16, zero16);
        // Package the compare result (16 bytes) to 16 bits.
        zero_mask = (size_t)_mm_movemask_epi8(src16);
        // Remove last missalign bits.
        zero_mask >>= misalignment;
        zero_mask <<= misalignment;;

        if (zero_mask != 0) {
            // Get the index of the first bit on set to 1.
            __BitScanForward(zero_index, zero_mask);
            goto strlen_exit;
        }

        // Align address to 32 bytes for misalignment.
        cur = (const char *)((size_t)str & ((size_t)~(size_t)0x1F));
    }
    else {
        // Align address to 32 bytes for this loop.
        cur = (const char * )((size_t)cur & ((size_t)~(size_t)0x1F));

        // Load the src 16 bytes to XMM register
        src16_high = _mm_load_si128((__m128i *)(cur + 16));
        src16_low  = _mm_load_si128((__m128i *)(cur));
        // Compare with zero16 masks per byte.
        src16_high = _mm_cmpeq_epi8(src16_high, zero16);
        src16_low  = _mm_cmpeq_epi8(src16_low,  zero16);
        // Package the compare result (16 bytes) to 16 bits.
        zero_mask_high = (size_t)_mm_movemask_epi8(src16_high);
        zero_mask_low  = (size_t)_mm_movemask_epi8(src16_low);
        // Combin the mask of the low 16 bits and high 16 bits.
        zero_mask = (zero_mask_high << 16) | zero_mask_low;
        // Remove last misalignment bits.
        zero_mask >>= misalignment;
        zero_mask <<= misalignment;;

        if (zero_mask != 0) {
            // Get the index of the first bit on set to 1.
            __BitScanForward(zero_index, zero_mask);
            goto strlen_exit;
        }
    }

main_loop:
    // Main loop
    do {
        // One loop scan 32 bytes.
        cur += 32;
        // Load the src 16 bytes to XMM register
        src16_high = _mm_load_si128((__m128i *)(cur + 16));
        src16_low  = _mm_load_si128((__m128i *)(cur));
        // Compare with zero16 masks per byte.
        src16_high = _mm_cmpeq_epi8(src16_high, zero16);
        src16_low  = _mm_cmpeq_epi8(src16_low,  zero16);
        // Package the compare result to 16 bits.
        zero_mask_high = (size_t)_mm_movemask_epi8(src16_high);
        zero_mask_low  = (size_t)_mm_movemask_epi8(src16_low);
        // Combin the mask of the low 16 bits and high 16 bits.
        zero_mask = (zero_mask_high << 16) | zero_mask_low;

        // If it have any one bit is 1, mean it have a null terminator
        // inside this scaned strings (per 32 bytes).
        if (zero_mask != 0) {
            // Get the index of the first bit on set to 1.
            __BitScanForward(zero_index, zero_mask);
            break;
        }
    } while (1);

strlen_exit:
    len = cur - str;
    len += zero_index;
    return len;
}

#if defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__)

size_t __FASTCALL strlen_fast_v1_sse2_x64(const char * str)
{
    size_t len;
    register __m128i zero16, src16_low, src16_high;
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
    // Set the zero masks (16 bytes).
    INIT_ZERO_16(zero16);
    zero16 = _mm_xor_si128(zero16, zero16);

    // Main loop
    do {
        // Load the src 16 bytes to XMM register
        src16_high = _mm_load_si128((__m128i *)(cur + 16));
        src16_low  = _mm_load_si128((__m128i *)(cur));
        // Compare with zero16 masks per byte.
        src16_high = _mm_cmpeq_epi8(src16_high, zero16);
        src16_low  = _mm_cmpeq_epi8(src16_low,  zero16);
        // Package the compare result (16 bytes) to 16 bits.
        zero_mask_high = (size_t)_mm_movemask_epi8(src16_high);
        zero_mask_low  = (size_t)_mm_movemask_epi8(src16_low);
        // Combin the mask of the low 16 bits and high 16 bits.
        zero_mask = (zero_mask_high << 16) | zero_mask_low;

        // If it have any one bit is 1, mean it have a null terminator
        // inside this scaned strings (per 64 bytes).
        if (zero_mask != 0) {
            // Get the index of the first bit on set to 1.
            __BitScanForward64(zero_index, zero_mask);
            break;
        }
        // One loop scan 32 bytes.
        cur += 32;

        // Load the src 16 bytes to XMM register
        src16_high = _mm_load_si128((__m128i *)(cur + 16));
        src16_low  = _mm_load_si128((__m128i *)(cur + 0));
        // Compare with zero16 masks per byte.
        src16_high = _mm_cmpeq_epi8(src16_high, zero16);
        src16_low  = _mm_cmpeq_epi8(src16_low,  zero16);
        // Package the compare result (16 bytes) to 16 bits.
        zero_mask_high = (size_t)_mm_movemask_epi8(src16_high);
        zero_mask_low  = (size_t)_mm_movemask_epi8(src16_low);
        // Combin the mask of the low 16 bits and high 16 bits.
        zero_mask = (zero_mask_high << 16) | zero_mask_low;

        // If it have any one bit is 1, mean it have a null terminator
        // inside this scaned strings (per 64 bytes).
        if (zero_mask != 0) {
            // Get the index of the first bit on set to 1.
            __BitScanForward64(zero_index, zero_mask);
            break;
        }
        // One loop scan 32 bytes.
        cur += 32;
    } while (1);

    len = cur - str;
    len += zero_index;
    return len;
}

#endif // _X64

size_t __FASTCALL strlen_fast_asm(const char * str)
{
    (void)str;
    return 0;
}
