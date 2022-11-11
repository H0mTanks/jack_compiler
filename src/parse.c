Internal char* parse_buf = NULL;

Internal Type* parse_type() {
    TypeKind kind;
    if (token.name == int_keyword) {
        kind = TYPE_INT;
    }
    else if (token.name == char_keyword) {
        kind = TYPE_CHAR;
    }
    else if (token.name == boolean_keyword) {
        kind = TYPE_BOOLEAN;
    }
    else {
        fatal_syntax_error("Unidentified typename: %s", token.name);
    }
    const char* type_name = token.name;

    expect_token(TOKEN_KEYWORD);

    return type_new(kind, type_name);
}

Internal ClassDecl* parse_class() {
    const char* class_name = token.name;
    expect_token(TOKEN_NAME);

    expect_token(TOKEN_LBRACE);

    VarDecl* vars = NULL;
    size_t num_vars = 0;
    while (token.name == static_keyword || token.name == field_keyword) {
        VarDecl var_decl = { 0 };
        var_decl.var_type = token.name == static_keyword ? VAR_STATIC : VAR_FIELD;
        expect_token(TOKEN_KEYWORD);

        var_decl.type = parse_type();

        var_decl.name = token.name;
        expect_token(TOKEN_NAME);

        BUF_PUSH(vars, var_decl);
        num_vars++;
    }

    expect_token(TOKEN_SEMICOLON);

    expect_token(TOKEN_RBRACE);

    return class_new(class_name, vars, num_vars, NULL, 0);
}

Internal void parse_tests() {
    init_stream("parse_tests", "class Test { field int a; }");
    match_token(TOKEN_KEYWORD);
    ClassDecl* c = parse_class();
}