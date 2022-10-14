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
    Map test_map = { 0 };
    // test_map.keys = 0;
    i32 a = 56;
    i32 b = 89;
    u64 ha = hash_u64(a);
    u64 hb = hash_u64(b);
    void* ka = (void*)(uintptr_t)(ha ? ha : 1);
    void* kb = (void*)(uintptr_t)(hb ? hb : 1);

    map_put(&test_map, ka, &a);
    map_put(&test_map, kb, &b);

    i32* ia = map_get(&test_map, ka);
    i32* ib = map_get(&test_map, kb);
}