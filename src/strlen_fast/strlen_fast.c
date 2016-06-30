
#include "strlen_fast.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

/**************************************************************************************

 _BitScanForward (VC) = __builtin_clz (gcc) = bsf (asm)
 _BitScanReverse (VC) = __builtin_ctz (gcc) = bsr (asm)

  On ARM it would be the CLZ (count leading zeroes) instruction.

 See: https://msdn.microsoft.com/en-us/library/wfd9z0bb.aspx
 See: https://msdn.microsoft.com/en-us/library/fbxyd7zd.aspx

 See: http://www.cnblogs.com/gleam/p/5025867.html

 ¡ª int __builtin_clz (unsigned int x);
    int __builtin_clzll (unsigned long long x);

    Returns the number of leading 0-bits in x, starting at the most significant bit position.
    If x is 0, the result is undefined. 

 ¡ª int __builtin_ctz (unsigned int x);
    int __builtin_ctzll (unsigned long long x);

    Returns the number of trailing 0-bits in x, starting at the least significant bit position.
    If x is 0, the result is undefined. 

 See: https://gcc.gnu.org/onlinedocs/gcc/Other-Builtins.html

 ¡ª int __builtin_ffs (int x);
    int __builtin_ffsll (long long x);

    Returns one plus the index of the least significant 1-bit of x, or if x is zero, returns zero. 

 int __builtin_ffs(int x) {
     if (x == 0)
         return 0;
     return __builtin_clz((unsigned int)x) + 1;
 }    

***************************************************************************************/

#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
#include <intrin.h>     // For _BitScanForward, _BitScanForward64
#pragma intrinsic(_BitScanForward)
#if defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__)
#pragma intrinsic(_BitScanForward64)
#endif // _WIN64
#endif // _MSC_VER
#include <xmmintrin.h>  // For MMX, SSE instructions
#include <emmintrin.h>  // For SSE2 instructions, __SSE2__ | -msse2

//
// See: http://www.cnblogs.com/zyl910/archive/2012/08/27/intrin_table_gcc.html
//
//#include <avxintrin.h>    // __AVX__  | -mavx     AVX:  Advanced Vector Extensions
//#include <avx2intrin.h>   // __AVX2__ | -mavx2    AVX2: Advanced Vector Extensions 2

// Get the index of the first bit on set to 1.
#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
// _MSC_VER
#if defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__)
    #define __BitScanForward32(bit_index, bit_mask) \
            _BitScanForward((unsigned long *)&(bit_index), (unsigned long)(bit_mask))

    #define __BitScanForward64(bit_index, bit_mask) \
            _BitScanForward64((unsigned long *)&(bit_index), (unsigned long long)(bit_mask))

    #define __BitScanForward(bit_index, bit_mask) \
            __BitScanForward64(bit_index, bit_mask)
#else
    #define __BitScanForward32(bit_index, bit_mask) \
            _BitScanForward((unsigned long *)&(bit_index), (unsigned long)(bit_mask))

    #define __BitScanForward64(bit_index, bit_mask) ((void)0)

    #define __BitScanForward(bit_index, bit_mask) \
            __BitScanForward32(bit_index, bit_mask)
#endif // _WIN64

#elif defined(__GNUC__) || defined(__clang__)
// __GNUC__
#if defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__)
    #define __BitScanForward32(bit_index, bit_mask) \
            bit_index = __builtin_clz((unsigned int)bit_mask)

    #define __BitScanForward64(bit_index, bit_mask) \
            bit_index = __builtin_clzll((unsigned long long)bit_mask)

    #define __BitScanForward(bit_index, bit_mask) \
            __BitScanForward64(bit_index, bit_mask)
#else
    #define __BitScanForward32(bit_index, bit_mask) \
            bit_index = __builtin_clz((unsigned int)bit_mask)

    #define __BitScanForward64(bit_index, bit_mask) ((void)0)

    #define __BitScanForward(bit_index, bit_mask) \
            __BitScanForward32(bit_index, bit_mask)
#endif // __x86_64__

#else
    // Not support
    #define __BitScanForward32(bit_index, bit_mask)
    #define __BitScanForward64(bit_index, bit_mask)
    #define __BitScanForward(bit_index, bit_mask)
    #error "The compiler does not support BitScanForward()."
#endif // BitScanForward()

size_t __FASTCALL strlen_fast_v1_sse2(const char * str)
{
    size_t len;
    __m128i zero16, src16_low, src16_high;
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

        // If it have any one bit is 1, mean have a null terminator
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
#if !defined(NDEBUG)
    if (len != strlen(str)) {
        printf("len = %" PRIuPTR ", zero_index = %u, missalign = %" PRIuPTR ".\n",
            len, zero_index, misalignment);
    }
#endif
    return len;
}

size_t __FASTCALL strlen_fast_v2_sse2(const char * str)
{
    size_t len;
    __m128i zero16, src16, src16_low, src16_high;
    register size_t zero_mask, zero_mask_low, zero_mask_high;
    unsigned long zero_index;
    register const char * cur = str;
    // Set the zero masks (16 bytes).
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
        zero_mask >>= (unsigned char)misalignment;
        zero_mask <<= (unsigned char)misalignment;

        if (zero_mask != 0) {
            // Get the index of the first bit on set to 1.
            __BitScanForward(zero_index, zero_mask);
            goto strlen_exit;
        }

        // Align address to 32 bytes for main loop.
        cur = (const char *)((size_t)str & ((size_t)~(size_t)0x1F));
    }
    else {
        // Align address to 32 bytes for main loop.
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
        zero_mask >>= (unsigned char)misalignment;
        zero_mask <<= (unsigned char)misalignment;

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

        // If it have any one bit is 1, mean have a null terminator
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
    __m128i zero16, src16_low, src16_high;
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

        // If it have any one bit is 1, mean have a null terminator
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

        // If it have any one bit is 1, mean have a null terminator
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
#if !defined(NDEBUG)
    if (len != strlen(str)) {
        printf("len = %" PRIuPTR ", zero_index = %u, missalign = %" PRIuPTR ".\n",
            len, zero_index, misalignment);
    }
#endif
    return len;
}

#endif // _X64

size_t __FASTCALL strlen_fast_asm(const char * str)
{
    return 0;
}
