
/**************************************************************************************

 _BitScanForward (VC) = __builtin_ctz (gcc) = bsf (asm)
 _BitScanReverse (VC) = __builtin_clz (gcc) = bsr (asm)

  On ARM it would be the CLZ (count leading zeroes) instruction.

 See: https://msdn.microsoft.com/en-us/library/wfd9z0bb.aspx
 See: https://msdn.microsoft.com/en-us/library/fbxyd7zd.aspx

 See: http://www.cnblogs.com/gleam/p/5025867.html

 ¡ª int __builtin_ctz (unsigned int x);
    int __builtin_ctzll (unsigned long long x);

    Returns the number of trailing 0-bits in x, starting at the least significant bit position.
    If x is 0, the result is undefined.   (MSB)

 ¡ª int __builtin_clz (unsigned int x);
    int __builtin_clzll (unsigned long long x);

    Returns the number of leading 0-bits in x, starting at the most significant bit position.
    If x is 0, the result is undefined.   (LSB)

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

#ifndef _BITSCAN_FORWARD_H_
#define _BITSCAN_FORWARD_H_

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
#include <intrin.h>     // For _BitScanForward, _BitScanForward64
#pragma intrinsic(_BitScanForward)
#if defined(_WIN64) || defined(WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__)
#pragma intrinsic(_BitScanForward64)
#endif // _WIN64
#endif // _MSC_VER

//#include <xmmintrin.h>  // For MMX, SSE instructions
#include <emmintrin.h>  // For SSE2 instructions, __SSE2__ | -msse2

//
// See: http://www.cnblogs.com/zyl910/archive/2012/08/27/intrin_table_gcc.html
//
//#include <avxintrin.h>    // __AVX__  | -mavx     AVX:  Advanced Vector Extensions
//#include <avx2intrin.h>   // __AVX2__ | -mavx2    AVX2: Advanced Vector Extensions 2
//

// Get the index of the first bit on set to 1.
#if defined(_MSC_VER) || defined(__INTEL_COMPILER)
// _MSC_VER
#if defined(_WIN64) || defined(WIN64) || defined(_M_X64) || defined(_M_AMD64) \
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

#elif (defined(__GNUC__) && ((__GNUC__ >= 4) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 4)))) \
   || defined(__clang__)
// __GNUC__
#if defined(_WIN64) || defined(WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__)
    #define __BitScanForward32(bit_index, bit_mask) \
            bit_index = __builtin_ctz((unsigned int)bit_mask)

    #define __BitScanForward64(bit_index, bit_mask) \
            bit_index = __builtin_ctzll((unsigned long long)bit_mask)

    #define __BitScanForward(bit_index, bit_mask) \
            __BitScanForward64(bit_index, bit_mask)
#else
    #define __BitScanForward32(bit_index, bit_mask) \
            bit_index = __builtin_ctz((unsigned int)bit_mask)

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

#endif // !_BITSCAN_FORWARD_H_
