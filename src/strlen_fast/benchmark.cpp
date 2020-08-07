
#ifdef __cplusplus
#define __STDC_LIMIT_MACROS
#define __STDC_CONSTANT_MACROS
#define __STDC_FORMAT_MACROS
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <memory>

#include <type_traits>
#include <iostream>

#include "strlen_fast.h"
#include "stop_watch.h"

using namespace jimi;

typedef jimi::StopWatch stop_watch;

enum {
    test_fixed_string,
    test_fixed_length,
    test_random_length,
    test_type_max
};

void generate_random_string(char * str, size_t str_len)
{
    unsigned char * dest = (unsigned char *)str;
    for (size_t i = 0; i < str_len; ++i)
        *dest++ = (unsigned char)(rand() % 255) + 1;
    *dest = '\0';
}

void generate_random_string_array(char * str, size_t length, size_t str_len, size_t alloc_size)
{
    unsigned char * dest = (unsigned char *)str;
    memset(dest, 'a', alloc_size);
    for (size_t i = 0; i < length; ++i) {
        //unsigned char * cur = (unsigned char *)dest;
        //for (size_t j = 0; j < str_len; ++j)
        //    *cur++ = (unsigned char)(rand() % 255) + 1;
        size_t base = str_len / 2;
        size_t len = base + (rand() % (str_len - base));
        *(dest + len) = '\0';
        dest += str_len;
    }
}

void strlen_benchmark_fixed_string(uint32_t str_len, uint32_t iterations)
{
    if (str_len <= 0) {
        printf(" type: test_fixed_string, Error: str_len <= 0, str_len = %u\n\n",
            str_len);
        return;
    }

    printf(" type: test_fixed_string, iterations = %u, str_len = %u byte(s)\n\n",
        iterations, str_len);

    std::unique_ptr<char> _str(new char[str_len]);
    char * str2 = _str.get();
    generate_random_string(str2, str_len - 1);

    {
        stop_watch sw;
        size_t sum = 0;
        size_t len;
        int dir = -1;
        (void)dir;
        char * str = _str.get();
        sw.start();
        for (uint32_t i = 0; i < iterations; ++i) {
            len = ::strlen(str);
#if defined(__GNUC__)
            // To avoid the compiler optimization in gcc or clang.
            if ((len == 0 && dir == -1) || (len == (str_len - 1) && dir == 1))
                dir = -dir;
            str += dir;
#endif
            sum += len;
        }
        sw.stop();

        str = _str.get();
        printf(" %26s = %8" PRIuPTR ", time spent: %8.3f ms, sum: %" PRIuPTR "\n",
            "::strlen(str)    ", ::strlen(str), sw.getElapsedMillisec(), sum);
    }

    {
        stop_watch sw;
        size_t sum = 0;
        size_t len;
        char * str = _str.get();

        sw.start();
        for (uint32_t i = 0; i < iterations; ++i) {
            len = strlen_sse2_v1(str);
            sum += len;
        }
        sw.stop();

        printf(" %26s = %8" PRIuPTR ", time spent: %8.3f ms, sum: %" PRIuPTR "\n",
            "strlen_sse2_v1(str)    ", strlen_sse2_v1(str), sw.getElapsedMillisec(), sum);
    }

    {
        stop_watch sw;
        size_t sum = 0;
        size_t len;
        char * str = _str.get();

        sw.start();
        for (uint32_t i = 0; i < iterations; ++i) {
            len = strlen_sse2_v2(str);
            sum += len;
        }
        sw.stop();

        printf(" %26s = %8" PRIuPTR ", time spent: %8.3f ms, sum: %" PRIuPTR "\n",
            "strlen_sse2_v2(str)    ", strlen_sse2_v2(str), sw.getElapsedMillisec(), sum);
    }

#if defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__)
    {
        stop_watch sw;
        size_t sum = 0;
        size_t len;
        char * str = _str.get();

        sw.start();
        for (uint32_t i = 0; i < iterations; ++i) {
            len = strlen_sse2_v1_x64(str);
            sum += len;
        }
        sw.stop();

        printf(" %26s = %8" PRIuPTR ", time spent: %8.3f ms, sum: %" PRIuPTR "\n",
            "strlen_sse2_v1(str) x64", strlen_sse2_v1_x64(str), sw.getElapsedMillisec(), sum);
    }
#endif // _X64

    {
        stop_watch sw;
        size_t sum = 0;
        size_t len;
        char * str = _str.get();

        sw.start();
        for (uint32_t i = 0; i < iterations; ++i) {
            len = strlen_avx_v1a(str);
            sum += len;
        }
        sw.stop();

        printf(" %26s = %8" PRIuPTR ", time spent: %8.3f ms, sum: %" PRIuPTR "\n",
            "strlen_avx_v1(str) (a)", strlen_avx_v1a(str), sw.getElapsedMillisec(), sum);
    }

    {
        stop_watch sw;
        size_t sum = 0;
        size_t len;
        char * str = _str.get();

        sw.start();
        for (uint32_t i = 0; i < iterations; ++i) {
            len = strlen_avx_v1b(str);
            sum += len;
        }
        sw.stop();

        printf(" %26s = %8" PRIuPTR ", time spent: %8.3f ms, sum: %" PRIuPTR "\n",
            "strlen_avx_v1(str) (b)", strlen_avx_v1b(str), sw.getElapsedMillisec(), sum);
    }

    {
        stop_watch sw;
        size_t sum = 0;
        size_t len;
        char * str = _str.get();

        sw.start();
        for (uint32_t i = 0; i < iterations; ++i) {
            len = strlen_avx_v2(str);
            sum += len;
        }
        sw.stop();

        printf(" %26s = %8" PRIuPTR ", time spent: %8.3f ms, sum: %" PRIuPTR "\n",
            "strlen_avx_v2(str)    ", strlen_avx_v2(str), sw.getElapsedMillisec(), sum);
    }

    printf("\n");
}

void strlen_benchmark_random_length(uint32_t str_len, uint32_t iterations)
{
    (void)iterations;

    if (str_len <= 0) {
        printf(" type: test_random_length, Error: str_len <= 0, max_str_len = %u\n\n", str_len);
        return;
    }

    uint32_t test_length = iterations;

    printf(" type: test_random_length, iterations = %u, str_len = %u byte(s)\n\n",
        test_length, str_len);

    size_t alloc_size = str_len * test_length;
    alloc_size = (alloc_size + 64 + 63) & ((size_t)~((size_t)63UL));
    std::unique_ptr<char> _str(new char[alloc_size]);
    generate_random_string_array(_str.get(), test_length, str_len, alloc_size);

    {
        stop_watch sw;
        size_t sum = 0;
        size_t len;
        char * str = _str.get();

        sw.start();
        for (uint32_t i = 0; i < test_length; ++i) {
            len = ::strlen(str);
            sum += len;
            str += str_len;
        }
        sw.stop();

        str = _str.get();
        printf(" %26s = %8" PRIuPTR ", time spent: %8.3f ms, sum: %" PRIuPTR "\n",
            "::strlen(str)    ", ::strlen(str), sw.getElapsedMillisec(), sum);
    }

    {
        stop_watch sw;
        size_t sum = 0;
        size_t len;
        char * str = _str.get();

        sw.start();
        for (uint32_t i = 0; i < test_length; ++i) {
            len = strlen_sse2_v1(str);
            sum += len;
            str += str_len;
        }
        sw.stop();

        str = _str.get();
        printf(" %26s = %8" PRIuPTR ", time spent: %8.3f ms, sum: %" PRIuPTR "\n",
            "strlen_sse2_v1(str)    ", strlen_sse2_v1(str), sw.getElapsedMillisec(), sum);
    }

    {
        stop_watch sw;
        size_t sum = 0;
        size_t len;
        char * str = _str.get();

        sw.start();
        for (uint32_t i = 0; i < test_length; ++i) {
            len = strlen_sse2_v2(str);
            sum += len;
            str += str_len;
        }
        sw.stop();

        str = _str.get();
        double elapsed_time = sw.getElapsedNanosec() / (double)(test_length * str_len);
        printf(" %26s = %8" PRIuPTR ", time spent: %8.3f ms, sum: %" PRIuPTR "\n",
            "strlen_sse2_v2(str)    ", strlen_sse2_v2(str), sw.getElapsedMillisec(), sum);
    }

#if defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__)
    {
        stop_watch sw;
        size_t sum = 0;
        size_t len;
        char * str = _str.get();

        sw.start();
        for (uint32_t i = 0; i < test_length; ++i) {
            len = strlen_sse2_v1_x64(str);
            sum += len;
            str += str_len;
        }
        sw.stop();

        str = _str.get();
        printf(" %26s = %8" PRIuPTR ", time spent: %8.3f ms, sum: %" PRIuPTR "\n",
            "strlen_sse2_v1(str) x64", strlen_sse2_v1_x64(str), sw.getElapsedMillisec(), sum);
    }
#endif // _X64

    {
        stop_watch sw;
        size_t sum = 0;
        size_t len;
        char * str = _str.get();

        sw.start();
        for (uint32_t i = 0; i < test_length; ++i) {
            len = strlen_avx_v1a(str);
            sum += len;
            str += str_len;
        }
        sw.stop();

        str = _str.get();
        printf(" %26s = %8" PRIuPTR ", time spent: %8.3f ms, sum: %" PRIuPTR "\n",
            "strlen_avx_v1(str) (a)", strlen_avx_v1a(str), sw.getElapsedMillisec(), sum);
    }

    {
        stop_watch sw;
        size_t sum = 0;
        size_t len;
        char * str = _str.get();

        sw.start();
        for (uint32_t i = 0; i < test_length; ++i) {
            len = strlen_avx_v1b(str);
            sum += len;
            str += str_len;
        }
        sw.stop();

        str = _str.get();
        printf(" %26s = %8" PRIuPTR ", time spent: %8.3f ms, sum: %" PRIuPTR "\n",
            "strlen_avx_v1(str) (b)", strlen_avx_v1b(str), sw.getElapsedMillisec(), sum);
    }

    {
        stop_watch sw;
        size_t sum = 0;
        size_t len;
        char * str = _str.get();

        sw.start();
        for (uint32_t i = 0; i < test_length; ++i) {
            len = strlen_avx_v2(str);
            sum += len;
            str += str_len;
        }
        sw.stop();

        str = _str.get();
        printf(" %26s = %8" PRIuPTR ", time spent: %8.3f ms, sum: %" PRIuPTR "\n",
            "strlen_avx_v2(str)    ", strlen_avx_v2(str), sw.getElapsedMillisec(), sum);
    }

    printf("\n");
}

void strlen_benchmark_impl(int test_type, uint32_t str_len, uint32_t iterations = 0)
{
    if (test_type == test_fixed_string) {
        // test_fixed_string
        strlen_benchmark_fixed_string(str_len, iterations);
    }
    else if (test_type == test_random_length) {
        // test_random_length
        strlen_benchmark_random_length(str_len, iterations);
    }
    else {
        printf("Unknown test type: %d.\n\n", test_type);
    }
}

void strlen_benchmark()
{
    uint32_t length, iterations;

    printf("--------------------------------------------------------------------------------------\n\n");

    static const int s_max_iterations_0[] = { 96000, 80000, 72000, 64000, 56000,
                                              48000, 40000, 36000, 32000, 28000,
                                              24000, 20000, 16000, 12000, 10000,
                                              8000,  6000,  5500,  5000,  4000,
                                              3000,  0 };
    length = 1;
    for (int i = 0; i < 22; ++i) {
        iterations = s_max_iterations_0[i];
        if (iterations == 0)
            break;
        strlen_benchmark_impl(test_fixed_string, length, iterations);
        length <<= 1;
    }

    printf("--------------------------------------------------------------------------------------\n\n");

    static const int s_max_iterations_2[] = { 65536 * 2, 65536, 32768, 16364, 8192,
                                              4096, 2048, 1024, 512, 256, 128, 64,
                                              32, 0 };
    length = (1 << 4);
    for (int i = 0; i < 12; ++i) {
        iterations = s_max_iterations_2[i];
        if (iterations == 0)
            break;
        strlen_benchmark_impl(test_random_length, length, iterations);
        length <<= 1;
    }

    printf("--------------------------------------------------------------------------------------\n\n");
}

int main(int argc, char * argv[])
{
    (void)argc;
    (void)argv;

#if 0
    char str[] = "abcdefggfdagafdgfadgdfgfdf";
    size_t len_v1 = strlen_sse2_v1(str);
    size_t len_v2 = strlen_sse2_v2(str);
    printf("\n");
    printf("str = \"%s\", address = 0x%016" PRIXPTR "\n"
        "strlen = %" PRIuPTR ", strlen_fast_v1 = %" PRIuPTR ", strlen_fast_v2 = %" PRIuPTR ".\n",
        str, (uintptr_t)&str[0], ::strlen(str), len_v1, len_v2);
    printf("\n");
#endif

    strlen_benchmark();

#if defined(_WIN32) || defined(WIN32) || defined(OS_WINDOWS) || defined(__WINDOWS__)
    ::system("pause");
#endif
    return 0;
}
