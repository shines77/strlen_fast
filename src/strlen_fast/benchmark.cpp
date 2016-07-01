
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>
#include <memory>

#include "strlen_fast.h"
#include "stop_watch.h"

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

void strlen_benchmark_fixed_string(int str_len, int iterations)
{
    if (str_len <= 0) {
        printf("Type: test_fixed_string, Error: str_len <= 0, str_len = %d\n\n",
            str_len);
        return;
    }

    printf("Type: test_fixed_string, iterations = %d, str_len = %d byte(s)\n\n",
        iterations, str_len);

    {
        StopWatch sw;
        size_t sum = 0, len;
        int dir = -1;
        std::unique_ptr<char> _str(new char[str_len]);
        char * str = _str.get();
        generate_random_string(str, str_len - 1);

        sw.start();
        for (int i = 0; i < iterations; ++i) {
            len = ::strlen(str);
            if ((len == 0 && dir == -1) || (len == (str_len - 1) && dir == 1))
                dir = -dir;
            str += dir;
            sum += len;
        }
        sw.stop();

        str = _str.get();
        printf("%30s = %8" PRIuPTR ", time spent: %8.3f ms, sum: %" PRIuPTR "\n",
            "::strlen(str)", ::strlen(str), sw.getMillisec(), sum);
    }

    {
        StopWatch sw;
        size_t sum = 0, len;
        std::unique_ptr<char> _str(new char[str_len]);
        char * str = _str.get();
        generate_random_string(str, str_len - 1);

        sw.start();
        for (int i = 0; i < iterations; ++i) {
            len = strlen_fast_v1_sse2(str);
            sum += len;
        }
        sw.stop();

        printf("%30s = %8" PRIuPTR ", time spent: %8.3f ms, sum: %" PRIuPTR "\n",
            "strlen_fast_v1_sse2(str)", strlen_fast_v1_sse2(str), sw.getMillisec(), sum);
    }

    {
        StopWatch sw;
        size_t sum = 0, len;
        std::unique_ptr<char> _str(new char[str_len]);
        char * str = _str.get();
        generate_random_string(str, str_len - 1);

        sw.start();
        for (int i = 0; i < iterations; ++i) {
            len = strlen_fast_v2_sse2(str);
            sum += len;
        }
        sw.stop();

        printf("%30s = %8" PRIuPTR ", time spent: %8.3f ms, sum: %" PRIuPTR "\n",
            "strlen_fast_v2_sse2(str)", strlen_fast_v2_sse2(str), sw.getMillisec(), sum);
    }

#if defined(_WIN64) || defined(_M_X64) || defined(_M_AMD64) \
 || defined(_M_IA64) || defined(__amd64__) || defined(__x86_64__)
    {
        StopWatch sw;
        size_t sum = 0, len;
        std::unique_ptr<char> _str(new char[str_len]);
        char * str = _str.get();
        generate_random_string(str, str_len - 1);

        sw.start();
        for (int i = 0; i < iterations; ++i) {
            len = strlen_fast_v1_sse2_x64(str);
            sum += len;
        }
        sw.stop();

        printf("%30s = %8" PRIuPTR ", time spent: %8.3f ms, sum: %" PRIuPTR "\n",
            "strlen_fast_v1_sse2_x64(str)", strlen_fast_v1_sse2_x64(str), sw.getMillisec(), sum);
    }
#endif // _X64

    printf("\n");
}

void strlen_benchmark_random_length(int str_len, int iterations)
{
    (void)iterations;

    if (str_len <= 0) {
        printf("Type: test_random_length, Error: str_len <= 0, str_len = %d\n\n", str_len);
        return;
    }
}

void strlen_benchmark_impl(int test_type, int str_len, int iterations = 0)
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
    int length, iterations;

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

    static const int s_max_iterations_2[] = { 16000, 16000, 16000, 8000, 8000,
                                              4000, 2000, 1000, 500, 250, 0 };
    length = (1 << 4);
    for (int i = 0; i < 10; ++i) {
        iterations = s_max_iterations_2[i];
        if (iterations == 0)
            break;
        strlen_benchmark_impl(test_random_length, length, iterations);
        length <<= 2;
    }
}

int main(int argc, char * argv[])
{
    (void)argc;
    (void)argv;

#if 0
    char str[] = "abcdefggfdagafdgfadgdfgfdf";
    size_t len_v1 = strlen_fast_v1_sse2(str);
    size_t len_v2 = strlen_fast_v2_sse2(str);
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
