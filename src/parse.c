Internal Type* parse_type(void) {
    TypeKind kind;
    if (token.name == void_keyword) {
        kind = TYPE_VOID;
    }
    else if (token.name == int_keyword) {
        kind = TYPE_INT;
    }
    else if (token.name == char_keyword) {
        kind = TYPE_CHAR;
    }
    else if (token.name == boolean_keyword) {
        kind = TYPE_BOOLEAN;
    }
    else {
        kind = TYPE_CLASSNAME;
    }
    const char* type_name = token.name;

    if (kind == TYPE_CLASSNAME) {
        expect_token(TOKEN_NAME);
    }
    else {
        expect_token(TOKEN_KEYWORD);
    }

    return type_new(kind, type_name);
}

Internal const char* parse_name(void) {
    const char* name = token.name;
    expect_token(TOKEN_NAME);

    return name;
}

Internal VarDecl parse_var(void) {
    VarDecl var = { 0 };
    var.type = parse_type();
    var.name = parse_name();

    return var;
}

Internal ClassDecl* parse_class(void) {
    const char* class_name = token.name;
    expect_token(TOKEN_NAME);

    expect_token(TOKEN_LBRACE);

    ClassVarDecl* class_vars = NULL;
    size_t num_classvars = 0;
    while (token.name == static_keyword || token.name == field_keyword) {
        VarType var_type = token.name == static_keyword ? VAR_STATIC : VAR_FIELD;
        expect_token(TOKEN_KEYWORD);

        Type* type = parse_type();

        BUF_PUSH(class_vars, (ClassVarDecl) { var_type, type, parse_name() });
        num_classvars++;
        while (match_token(TOKEN_COMMA)) {
            BUF_PUSH(class_vars, (ClassVarDecl) { var_type, type, parse_name() });
            num_classvars++;
        }

        expect_token(TOKEN_SEMICOLON);
    }

    // typedef enum SubroutineType {
    //     SUB_CONSTRUCTOR,
    //     SUB_FUNCTION,
    //     SUB_METHOD,
    // } SubroutineType;

    // typedef struct Subroutine {
    //     SubroutineType sub_type;
    //     FuncParam* params;
    //     size_t num_params;
    //     Type* ret_type;
    //     VarDecl* vars;
    //     StmtList block;
    // } Subroutine;
    Subroutine* subs = NULL;
    size_t num_subs = 0;
    while (token.name == constructor_keyword || token.name == method_keyword || token.name == function_keyword) {
        SubroutineType sub_type = token.name == constructor_keyword ? SUB_CONSTRUCTOR : SUB_FUNCTION;
        sub_type = token.name == method_keyword ? SUB_METHOD : sub_type;
        expect_token(TOKEN_KEYWORD);

        Type* ret_type = parse_type();

        const char* sub_name = parse_name();
        expect_token(TOKEN_LPAREN);

        size_t num_params = 0;
        VarDecl* params = NULL;
        if (!is_token(TOKEN_RPAREN)) {
            BUF_PUSH(params, parse_var());
            num_params++;
            while (match_token(TOKEN_COMMA)) {
                BUF_PUSH(params, parse_var());
                num_params++;
            }
        }
        expect_token(TOKEN_RPAREN);

        //*subroutine body
        expect_token(TOKEN_LBRACE);

        VarDecl* vars = NULL;
        size_t num_vars = 0;
        while (token.name == var_keyword) {
            expect_token(TOKEN_KEYWORD);

            VarDecl first_var = parse_var();
            BUF_PUSH(vars, first_var);
            num_vars++;
            while (match_token(TOKEN_COMMA)) {
                BUF_PUSH(vars, (VarDecl) { first_var.type, parse_name() });
                num_vars++;
            }
            expect_token(TOKEN_SEMICOLON);
        }

        expect_token(TOKEN_RBRACE);

        BUF_PUSH(subs, (Subroutine) { sub_type, sub_name, ast_rep(params, num_params * sizeof(VarDecl)), num_params, ret_type, ast_rep(vars, num_vars * sizeof(VarDecl)), num_vars, (StmtList) { 0 } });
        num_subs++;
    }

    expect_token(TOKEN_RBRACE);

    return class_new(class_name, class_vars, num_classvars, subs, num_subs);
}

Internal void parse_tests() {
    init_stream("parse_tests", "class\n Test {\n field int a, c, d;\n static char b;\n function void func(int a, char b) {\nvar int bar, foo, sdf;\n var char t;}\n method Foo meth() {\nvar Square asd;\n}\n}\n");
    match_token(TOKEN_KEYWORD);
    ClassDecl* c = parse_class();
    print_class(c);
    flush_parse();
}