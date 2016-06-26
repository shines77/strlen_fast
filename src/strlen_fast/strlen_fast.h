
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

#define strlen_fast     strlen_fast_v1

#ifdef __cplusplus
extern "C" {
#endif

size_t __fastcall strlen_fast_v1(const char * str);
size_t __fastcall strlen_fast_v2(const char * str);
size_t __fastcall strlen_fast_asm(const char * str);

#ifdef __cplusplus
}
#endif

#endif // !_STRLEN_FAST_H_
