
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <inttypes.h>

#include "strlen_fast.h"

int main(int argn, char *argv[])
{
    char str[] = "abcdefggfdagafdgfadgdfgfdf";
    size_t len_v1 = strlen_fast_v1(str);
    size_t len_v2 = strlen_fast_v2(str);
    printf("\n");
    printf("str = \"%s\", address = 0x%016" PRIXPTR "\n"
        "strlen = %" PRIuPTR ", strlen_fast_v1 = %" PRIuPTR ", strlen_fast_v2 = %" PRIuPTR ".\n",
        str, (uintptr_t)&str[0], ::strlen(str), len_v1, len_v2);
    printf("\n");

    ::system("pause");
    return 0;
}
