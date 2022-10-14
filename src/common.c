#define MAX(x, y) ((x) >= (y) ? (x) : (y))
#define IS_POW2(x) (((x) != 0) && ((x) & ((x)-1)) == 0)
#define ALIGN_DOWN(n, a) ((n) & ~((a) - 1))
#define ALIGN_UP(n, a) ALIGN_DOWN((n) + (a) - 1, (a))
#define ALIGN_DOWN_PTR(p, a) ((void *)ALIGN_DOWN((uintptr_t)(p), (a)))
#define ALIGN_UP_PTR(p, a) ((void *)ALIGN_UP((uintptr_t)(p), (a)))

Internal void* xcalloc(size_t num_elems, size_t elem_size) {
    void* ptr = calloc(num_elems, elem_size);
    if (!ptr) {
        perror("xcalloc failed");
        exit(1);
    }

    return ptr;
}

Internal void* xrealloc(void* ptr, size_t num_bytes) {
    ptr = realloc(ptr, num_bytes);
    if (!ptr) {
        perror("xrealloc failed");
        exit(1);
    }
    return ptr;
}

Internal void* xmalloc(size_t num_bytes) {
    void* ptr = malloc(num_bytes);
    if (!ptr) {
        perror("xmalloc failed");
        exit(1);
    }
    return ptr;
}

Internal void fatal(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    printf("FATAL: ");
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
    exit(1);
}

Internal void syntax_error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    printf("Syntax Error: ");
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
}

Internal void fatal_syntax_error(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    printf("Syntax Error: ");
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
    exit(1);
}

Internal void* memdup(void* src, size_t size) {
    void* dest = xmalloc(size);
    memcpy(dest, src, size);
    return dest;
}

Internal char* strf(const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    size_t n = 1 + vsnprintf(NULL, 0, fmt, args);
    va_end(args);
    char* str = xmalloc(n);
    va_start(args, fmt);
    vsnprintf(str, n, fmt, args);
    va_end(args);
    return str;
}

typedef struct BufHdr {
    size_t len;
    size_t cap;
    char buf[];
} BufHdr;

//* BufHdr pointer to the start of BufHdr where len is stored
#define _BUF_HDR(b) ((BufHdr*)((char*)(b) - offsetof(BufHdr, buf)))
#define _BUF_HDR_PRINT(b) (printf("len: %zu, cap: %zu\n", BUF_LEN(b), BUF_CAP(b)))

#define BUF_LEN(b) ((b) ? _BUF_HDR(b)->len : 0)
#define BUF_CAP(b) ((b) ? _BUF_HDR(b)->cap : 0)
#define BUF_END(b) ((b) + BUF_LEN(b))
#define BUF_SIZEOF(b) ((b) ? BUF_LEN(b) * sizeof(*b) : 0)

#define BUF_FREE(b) ((b) ? (free(_BUF_HDR(b)), (b) = NULL) : 0)
#define BUF_FIT(b, n) ((n) <= BUF_CAP(b) ? 0 : ((b) = _buf_grow((b), (n), sizeof(*(b)))))
#define BUF_PUSH(b, ...) (BUF_FIT((b), 1 + BUF_LEN(b)), (b)[_BUF_HDR(b)->len++] = (__VA_ARGS__))
#define BUF_CLEAR(b) ((b) ? _BUF_HDR(b)->len = 0 : 0)

Internal void* _buf_grow(const void* buf, size_t new_len, size_t elem_size) {
    assert(BUF_CAP(buf) <= (SIZE_MAX - 1) / 2);
    size_t new_cap = MAX(16, MAX(1 + 2 * BUF_CAP(buf), new_len));
    assert(new_len <= new_cap);
    assert(new_cap <= (SIZE_MAX - offsetof(BufHdr, buf)) / elem_size);
    size_t new_size = offsetof(BufHdr, buf) + new_cap * elem_size;
    BufHdr* new_hdr;

    if (buf) {
        new_hdr = xrealloc(_BUF_HDR(buf), new_size);
    }
    else {
        new_hdr = xmalloc(new_size);
        new_hdr->len = 0;
    }

    new_hdr->cap = new_cap;
    return new_hdr->buf;
}

Internal void buffer_tests() {
    //* declares a buffer pointer arr and checks if it has no length, i.e 0 elements
    int* buf = NULL;
    assert(BUF_LEN(buf) == 0);

    //*we will push 1024 elements into arr
    int n = 1024;
    for (int i = 0; i < n; i++) {
        BUF_PUSH(buf, i);
    }

    //*assert that they were inserted
    assert(BUF_LEN(buf) == n);
    for (size_t i = 0; i < BUF_LEN(buf); i++) {
        assert(buf[i] == i);
    }

    //* free the array, check if pointer is set to null and buffer length is 0
    BUF_FREE(buf);
    assert(buf == NULL);
    assert(BUF_LEN(buf) == 0);
}

//Arena allocator
typedef struct Arena {
    char* ptr;
    char* end;
    char** blocks;
} Arena;

#define ARENA_ALIGNMENT 8
#define ARENA_BLOCK_SIZE 1024

Internal void arena_grow(Arena* arena, size_t min_size) {
    size_t size = ALIGN_UP(MAX(ARENA_BLOCK_SIZE, min_size), ARENA_ALIGNMENT);
    arena->ptr = xmalloc(size);
    assert(arena->ptr == ALIGN_DOWN_PTR(arena->ptr, ARENA_ALIGNMENT));

    arena->end = arena->ptr + size;
    BUF_PUSH(arena->blocks, arena->ptr);
}

Internal void* arena_alloc(Arena* arena, size_t size) {
    if (size > (size_t)(arena->end - arena->ptr)) {
        arena_grow(arena, size);
        assert(size <= (size_t)(arena->end - arena->ptr));
    }

    void* ptr = arena->ptr;
    arena->ptr = ALIGN_UP_PTR(arena->ptr + size, ARENA_ALIGNMENT);
    assert(arena->ptr <= arena->end);
    assert(ptr == ALIGN_DOWN_PTR(ptr, ARENA_ALIGNMENT));
    return ptr;
}

Internal void arena_free(Arena* arena) {
    for (char** it = arena->blocks; it != BUF_END(arena->blocks); it++) {
        free(*it);
    }
    BUF_FREE(arena->blocks);
}

//*Hash map

u64 hash_u64(u64 x) {
    x *= 0xff51afd7ed558ccd;
    x ^= x >> 32;
    return x;
}

u64 hash_ptr(void* ptr) {
    return hash_u64((uintptr_t)ptr);
}

u64 hash_bytes(const char* buf, size_t len) {
    uint64_t x = 0xcbf29ce484222325;
    for (size_t i = 0; i < len; i++) {
        x ^= buf[i];
        x *= 0x100000001b3;
        x ^= x >> 32;
    }
    return x;
}

//*String interning
//*A string consists of an Intern struct which consists of it's length and pointer to the string.
typedef struct Intern {
    size_t len;
    const char* str;
} Intern;

//*The memory for all the strings
Arena intern_arena;
//*pointer to the structs which hold pointers to the strings, it is a stretchy buffer
Intern* interns;

//*checks if the new string is part of the existing list of strings in the intern table
//*if it already exists, return a pointer to the underlying char buffer
//*if it does not exist, allocate memory for it and add it to the intern table.
Internal const char* str_intern_range(const char* start, const char* end) {
    size_t len = end - start;

    for (Intern* it = interns; it != BUF_END(interns); it++) {
        if (it->len == len && strncmp(it->str, start, len) == 0) {
            return it->str;
        }
    }

    char* str = arena_alloc(&intern_arena, len + 1);
    memcpy(str, start, len);
    str[len] = 0;
    BUF_PUSH(interns, (Intern) { len, str });

    return str;
}

//*assumes null terminated strings because of strlen, wrapper for str_intern_range
Internal const char* str_intern(const char* str) {
    return str_intern_range(str, str + strlen(str));
}

Internal void intern_tests() {
    char a[] = "hello";
    //*equality by strcmp
    assert(strcmp(a, str_intern(a)) == 0);

    //*idempotence tests
    assert(str_intern(a) == str_intern(a));
    assert(str_intern(str_intern(a)) == str_intern(a));

    char b[] = "hello";
    assert(a != b);
    assert(str_intern(a) == str_intern(b));

    //*prefix test
    char c[] = "hello!";
    assert(str_intern(a) != str_intern(c));

    //*suffix test
    char d[] = "hell";
    assert(str_intern(a) != str_intern(d));
}

Internal void common_tests() {
    buffer_tests();
    intern_tests();
}