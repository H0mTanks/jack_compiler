#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <assert.h>
#include <types.c>
#include "common.c"

void tests() {
    common_tests();
}

int main() {
    printf("Hello\n");

    tests();
}