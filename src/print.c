#include "ast.h"

Internal char* parse_buf = NULL;
Internal char* indent = NULL;

Internal const char* get_indent(void) {
    if (!indent) {
        return "";
    }

    return indent;
}

#define PPRINT(...) BUF_PRINTF(parse_buf, __VA_ARGS__);
#define PLINE(str) PPRINT("%s%s\n", get_indent(), str);
#define INDENT() BUF_PRINTF(indent, "  ");
#define RINDENT() _BUF_HDR(indent)->len -= 2; indent[BUF_LEN(indent)] = '\0'
#define JFIELDS(key, val) PPRINT("%s%s: %s,\n", get_indent(), (key), (val));
#define JFIELDU(key, val) PPRINT("%s%s: %zu,\n", get_indent(), (key), (val));
#define JFIELDI(key, val) PPRINT("%s%s: %d,\n", get_indent(), (key), (val));

// typedef struct ClassDecl {
//     const char* name;
//     VarDecl* vars;
//     size_t num_vars;
//     Subroutine* subs;
//     size_t num_subs;
// } ClassDecl;
Internal void print_class(ClassDecl* c) {
    PLINE("class: {");
    INDENT();

    JFIELDU("nVars", c->num_vars);
    JFIELDU("nSubs", c->num_subs);

    if (c->num_vars) {
        PLINE("vars: [");
        INDENT();

        VarDecl* vars = c->vars;
        for (i32 i = 0; i < c->num_vars; i++) {
            PLINE("{");
            INDENT();


            const char* var_type_keyword = vars[i].var_type == VAR_STATIC ? static_keyword : field_keyword;
            const char* type_name = vars[i].type->name;
            const char* var_name = vars[i].name;

            JFIELDS("access", var_type_keyword);
            JFIELDS("type", type_name);
            JFIELDS("name", var_name);

            printf("indent: %zu\n", BUF_LEN(indent));
            RINDENT();
            printf("indent: %zu\n", BUF_LEN(indent));
            PLINE("},");
        }
        RINDENT();
        PLINE("],");
    }

    RINDENT();
    PLINE("}");
    RINDENT();
}

Internal void flush_parse(void) {
    printf("%s", parse_buf);

    BUF_FREE(parse_buf);
    BUF_FREE(indent);
}

#undef PPRINT
#undef PLINE
#undef INDENT
#undef RINDENT
#undef JFIELDS
#undef JFIELDU
#undef JFIELDI