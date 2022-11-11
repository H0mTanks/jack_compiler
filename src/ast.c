Arena ast_arena;

void* ast_alloc(size_t size) {
    assert(size != 0);
    void* ptr = arena_alloc(&ast_arena, size);
    memset(ptr, 0, size);
    return ptr;
}

void* ast_dup(const void* src, size_t size) {
    if (size == 0) {
        return NULL;
    }
    void* ptr = arena_alloc(&ast_arena, size);
    memcpy(ptr, src, size);
    return ptr;
}

void* ast_rep(const void* src, size_t size) {
    void* ptr = ast_dup(src, size);
    BUF_FREE(src);
    return ptr;
}

Type* type_new(TypeKind kind, const char* name) {
    Type* t = ast_alloc(sizeof(Type));
    t->kind = kind;
    t->name = name;
    return t;
}

ClassDecl* class_new(const char* name, VarDecl* vars, size_t num_vars, Subroutine* subs, size_t num_subs) {
    ClassDecl* c = ast_alloc(sizeof(ClassDecl));
    c->name = name;
    c->vars = ast_rep(vars, num_vars * sizeof(*vars));
    c->num_vars = num_vars;
    c->subs = ast_rep(subs, num_subs * sizeof(*subs));
    c->num_subs = num_subs;

    return c;
}