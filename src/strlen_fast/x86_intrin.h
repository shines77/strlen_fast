
#ifndef _AUTO_INTRIN_H_
#define _AUTO_INTRIN_H_

#if defined(_MSC_VER) && (_MSC_VER >= 1020)
#pragma once
#endif

#include "msvc/stdint.h"

#if !defined(PTRDIFF_MAX) || !defined(INT32_MAX)
#error Need C99 marcos: __STDC_LIMIT_MACROS.
#endif

// INTRIN_WORDSIZE: target arch's word size.
#if PTRDIFF_MAX >= INT64_MAX
    #define INTRIN_WORDSIZE    64
#elif PTRDIFF_MAX >= INT32_MAX
    #define INTRIN_WORDSIZE    32
#else
    #define INTRIN_WORDSIZE    16
#endif

#if defined(__GNUC__) || defined(__clang__) // GCC or clang
    #if (defined(__i386__) || defined(__x86_64__) || defined(__amd64__))
        // header files
        //#include <cpuid.h>        // Sometimes can not find On Mac, so give up.
                                    // Use the ccpuid module will be more convenient.
        //#include <x86intrin.h>    // Sometimes can not find On Mac,
                                    // so loaded on the header file by the macro.
        // macros
        #ifdef __MMX__
            #define __HAS_MMX__     1
            #include <mmintrin.h>
        #endif
        #ifdef __3dNOW__
            #define __HAS_3dNOW__   1
            #include <mm3dnow.h>
        #endif
        #ifdef __SSE__
            #define __HAS_SSE__     1
            #include <xmmintrin.h>
        #endif
        #ifdef __SSE2__
            #define __HAS_SSE2__    1
            #include <emmintrin.h>
        #endif
        #ifdef __SSE3__
            #define __HAS_SSE3__    1
            #include <pmmintrin.h>
        #endif
        #ifdef __SSSE3__
            #define __HAS_SSSE3__   1
            #include <tmmintrin.h>
        #endif
        #ifdef __SSE4_1__
            #define __HAS_SSE4_1__  1
            #include <smmintrin.h>
        #endif
        #ifdef __SSE4_2__
            #define __HAS_SSE4_2__  1
            #include <nmmintrin.h>
        #endif
        #ifdef __SSE4A__
            #define __HAS_SSE4A__   1
            #include <ammintrin.h>
        #endif
        #ifdef __AES__
            #define __HAS_AES__     1
            #include <x86intrin.h>
        #endif
        #ifdef __PCLMUL__
            #define __HAS_PCLMUL__  1
            #include <x86intrin.h>
        #endif
        #ifdef __AVX__
            #define __HAS_AVX__     1
            #include <x86intrin.h>
        #endif
        #ifdef __AVX2__
            #define __HAS_AVX2__    1
            #include <x86intrin.h>
        #endif
        #ifdef __F16C__
            #define __HAS_F16C__    1
            #include <x86intrin.h>
        #endif
        #ifdef __FMA__
            #define __HAS_FMA__     1
            #include <x86intrin.h>
        #endif
        #ifdef __FMA4__
            #define __HAS_FMA4__    1
            #include <x86intrin.h>
        #endif
        #ifdef __XOP__
            #define __HAS_XOP__     1
            #include <xopintrin.h>
        #endif
        #ifdef __LWP__
            #define __HAS_LWP__     1
            #include <x86intrin.h>
        #endif
        #ifdef __RDRND__
            #define __HAS_RDRND__   1
            #include <x86intrin.h>
        #endif
        #ifdef __FSGSBASE__
            #define __HAS_FSGSBASE__    1
            #include <x86intrin.h>
        #endif
        #ifdef __POPCNT__
            #define __HAS_POPCNT__  1
            #include <popcntintrin.h>
        #endif
        #ifdef __LZCNT__
            #define __HAS_LZCNT__   1
            #include <x86intrin.h>
        #endif
        #ifdef __TBM__
            #define __HAS_TBM__     1
            #include <x86intrin.h>
        #endif
        #ifdef __BMI__
            #define __HAS_BMI__     1
            #include <x86intrin.h>
        #endif
        #ifdef __BMI2__
            #define __HAS_BMI2__    1
            #include <x86intrin.h>
        #endif

    #endif // __i386__ || __x86_64__ || __amd64__

#elif defined(_MSC_VER)     // MSVC
    // header files
    #if (_MSC_VER >= 1400)          // >= VC 2005
        #include <intrin.h>
    #elif (_MSC_VER >= 1200)        // >= VC 6.0
        #if (defined(_M_IX86) || defined(_M_X64))
            #include <emmintrin.h>  // For MMX, SSE, SSE2
            #include <mm3dnow.h>    // For 3DNow!
        #endif
    #endif // _MSC_VER >= 1400
    #include <malloc.h>             // For _mm_malloc, _mm_free.

    // macros
    #if (defined(_M_IX86) || defined(_M_X64))
        #if (_MSC_VER >= 1200)      // VC 6.0
            #if defined(_M_X64) && !defined(__INTEL_COMPILER)
                // VC compiler does not support 64 bit MMX instruction.
            #else
                #define __HAS_MMX__     1   // mmintrin.h
                #define __HAS_3dNOW__   1   // mm3dnow.h
            #endif
            #define __HAS_SSE__         1   // xmmintrin.h
            #define __HAS_SSE2__        1   // emmintrin.h
        #endif
        #if (_MSC_VER >= 1300)      // VC 2003
        #endif
        #if (_MSC_VER >= 1400)      // VC 2005
        #endif
        #if (_MSC_VER >= 1500)      // VC 2008
            #define __HAS_SSE3__        1    // pmmintrin.h
            #define __HAS_SSSE3__       1    // tmmintrin.h
            #define __HAS_SSE4_1__      1    // smmintrin.h
            #define __HAS_SSE4_2__      1    // nmmintrin.h
            #define __HAS_POPCNT__      1    // nmmintrin.h
            #define __HAS_SSE4A__       1    // intrin.h
            #define __HAS_LZCNT__       1    // intrin.h
        #endif
        #if (_MSC_VER >= 1600)      // VC 2010
            #define __HAS_AES__         1    // wmmintrin.h
            #define __HAS_PCLMUL__      1    // wmmintrin.h
            #define __HAS_AVX__         1    // immintrin.h
            #define __HAS_FMA4__        1    // ammintrin.h
            #define __HAS_XOP__         1    // ammintrin.h
            #define __HAS_LWP__         1    // ammintrin.h
        #endif
        #if (_MSC_VER >= 1700)      // VC 2012
            //#define __HAS_AVX2__      1    // TODO:
            //#define __HAS_FMA__       1
            //#define __HAS_F16C__      1
            //#define __HAS_RDRND__     1
            //#define __HAS_FSGSBASE__  1
            //#define __HAS_TBM__       1
            //#define __HAS_BMI__       1
            //#define __HAS_BMI2__      1
        #endif
        #if (_MSC_VER >= 1800)      // VC 2013
            #define __HAS_AVX2__        1   // immintrin.h
        #endif
    #endif
    // TODO: VS with the C Intel compiler intrin function support.

    // Have no _mm_cvtss_f32() function before VC 2008.
    #if (_MSC_VER < 1500)    // VC 2008
        // float _mm_cvtss_f32(__m128 _A);
        #ifndef _mm_cvtss_f32
            #define _mm_cvtss_f32(__m128_A)     (*(float*)(void*)&(__m128_A))
        #endif
    #endif
#else
#error Only supports GCC, CLang or MSVC.
#endif // __GNUC__

#endif // !_AUTO_INTRIN_H_
