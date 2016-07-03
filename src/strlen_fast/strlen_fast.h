
#ifndef _STRLEN_FAST_H_
#define _STRLEN_FAST_H_

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#if defined(__linux__)
#  include <endian.h>
#elif defined(__FreeBSD__) || defined(__NetBSD__)
#  include <sys/endian.h>
#elif defined(__OpenBSD__)
#  include <sys/types.h>
#endif

#include "x86_intrin.h"

#ifndef ATTR_ALIGN
#  if defined(__GNUC__) || defined(__clang__)   // GCC or clang
#    define ATTR_ALIGN(N)    __attribute__((aligned(N)))
#  elif defined(_MSC_VER)                       // MSVC
#    define ATTR_ALIGN(N)    __declspec(align(N))
#  else
#    define ATTR_ALIGN(N)
#  endif
#endif // ATTR_ALIGN

#ifndef __FASTCALL
#define __FASTCALL  __fastcall
#endif

#if defined(_WIN64) || defined(WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__)

#if defined(__GNUC__) || defined(__clang__)
    #undef  __FASTCALL
    #define __FASTCALL
#endif // __GNUC__

#endif  // __x86_64__

#define strlen_fast     strlen_fast_v2_sse2

#ifdef __cplusplus
extern "C" {
#endif

size_t __FASTCALL strlen_fast_v1_sse2(const char * str);
size_t __FASTCALL strlen_fast_v2_sse2(const char * str);
size_t __FASTCALL strlen_fast_v1_sse2_x64(const char * str);
size_t __FASTCALL strlen_fast_asm(const char * str);

#if defined(_WIN64) || defined(WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__) || 1
size_t __FASTCALL strlen_fast_v1a_avx(const char * str);
size_t __FASTCALL strlen_fast_v1b_avx(const char * str);
size_t __FASTCALL strlen_fast_v2_avx(const char * str);
size_t __FASTCALL strlen_fast_v1_avx_x64(const char * str);
size_t __FASTCALL strlen_fast_asm_avx(const char * str);

size_t __FASTCALL strlen_fast_v1_avx2(const char * str);
size_t __FASTCALL strlen_fast_v2_avx2(const char * str);
size_t __FASTCALL strlen_fast_v1_avx2_x64(const char * str);
size_t __FASTCALL strlen_fast_asm_avx2(const char * str);
#endif // _M_X64 || __x86_64__

#if defined(__GNUC__)
static const __m128i __simd_zero16 = { 0, 0 };
static const __m256i __simd_zero32 = { 0, 0, 0, 0 };
#else
static const __m128i __simd_zero16 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static const __m256i __simd_zero32 = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
                                       0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
#endif

#if defined(NDEBUG)
#define INIT_ZERO_16(xmm)   (void)(xmm)
#define INIT_ZERO_32(avx)   (void)(avx)
#else
#define INIT_ZERO_16(xmm)   (xmm) = __simd_zero16
#define INIT_ZERO_32(avx)   (avx) = __simd_zero32
#endif

#ifdef __cplusplus
}
#endif

#endif // !_STRLEN_FAST_H_
