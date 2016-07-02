
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

#ifndef __FASTCALL
#define __FASTCALL  __fastcall
#endif

#if defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
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

size_t __FASTCALL strlen_fast_v1_avx(const char * str);
size_t __FASTCALL strlen_fast_v2_avx(const char * str);
size_t __FASTCALL strlen_fast_v1_avx_x64(const char * str);
size_t __FASTCALL strlen_fast_asm_avx(const char * str);

#ifdef __cplusplus
}
#endif

#endif // !_STRLEN_FAST_H_
