const char* class_keyword;
const char* constructor_keyword;
const char* function_keyword;
const char* method_keyword;
const char* field_keyword;
const char* static_keyword;
const char* var_keyword;
const char* int_keyword;
const char* char_keyword;
const char* boolean_keyword;
const char* void_keyword;
const char* true_keyword;
const char* false_keyword;
const char* null_keyword;
const char* this_keyword;
const char* let_keyword;
const char* do_keyword;
const char* if_keyword;
const char* else_keyword;
const char* while_keyword;
const char* return_keyword;

const char* first_keyword;
const char* last_keyword;
const char** keywords;

#define KEYWORD(name) name##_keyword = str_intern(#name); BUF_PUSH(keywords, name##_keyword)

void init_keywords(void) {
    LocalPersist bool inited;
    if (inited) {
        return;
    }

    //* make sure all keywords are allocated in the same block in arena by comparing block ends
    //* before and after allocation
    KEYWORD(class);
    char* arena_end = intern_arena.end;
    KEYWORD(constructor);
    KEYWORD(function);
    KEYWORD(method);
    KEYWORD(field);
    KEYWORD(static);
    KEYWORD(var);
    KEYWORD(int);
    KEYWORD(char);
    KEYWORD(boolean);
    KEYWORD(void);
    KEYWORD(true);
    KEYWORD(false);
    KEYWORD(null);
    KEYWORD(this);
    KEYWORD(let);
    KEYWORD(do);
    KEYWORD(if);
    KEYWORD(else);
    KEYWORD(while);
    KEYWORD(return);
    assert(intern_arena.end == arena_end);
    first_keyword = class_keyword;
    last_keyword = return_keyword;

    inited = true;
}

#undef KEYWORD

bool is_keyword_name(const char* name) {
    return first_keyword <= name && name <= last_keyword;
}

typedef enum TokenKind {
    TOKEN_EOF,
    TOKEN_LBRACKET,
    TOKEN_RBRACKET,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACE,
    TOKEN_RBRACE,
    TOKEN_DOT,
    TOKEN_COMMA,
    TOKEN_SEMICOLON,
    TOKEN_NEG,
    TOKEN_NOT,
    TOKEN_KEYWORD,
    TOKEN_INT,
    TOKEN_STR,
    TOKEN_NAME,
    //* multiplicative precedence
    TOKEN_FIRST_MUL,
    TOKEN_MUL = TOKEN_FIRST_MUL,
    TOKEN_DIV,
    TOKEN_AND,
    TOKEN_LAST_MUL = TOKEN_AND,
    //* additive precedence
    TOKEN_FIRST_ADD,
    TOKEN_ADD = TOKEN_FIRST_ADD,
    TOKEN_SUB,
    TOKEN_OR,
    TOKEN_LAST_ADD = TOKEN_OR,
    //* comparative precedence
    TOKEN_FIRST_CMP,
    TOKEN_EQ = TOKEN_FIRST_CMP,
    TOKEN_LT,
    TOKEN_GT,
    TOKEN_LAST_CMP = TOKEN_GT,
} TokenKind;

//TODO: revert change of lt and gt from xml friendly versions
const char* token_kind_names[] = {
    [TOKEN_EOF] = "EOF",
    [TOKEN_LBRACKET] = "[",
    [TOKEN_RBRACKET] = "]",
    [TOKEN_LPAREN] = "(",
    [TOKEN_RPAREN] = ")",
    [TOKEN_LBRACE] = "{",
    [TOKEN_RBRACE] = "}",
    [TOKEN_DOT] = ".",
    [TOKEN_COMMA] = ",",
    [TOKEN_SEMICOLON] = ";",
    [TOKEN_NEG] = "-",
    [TOKEN_NOT] = "~",
    [TOKEN_KEYWORD] = "keyword",
    [TOKEN_INT] = "int",
    [TOKEN_STR] = "string",
    [TOKEN_NAME] = "name",
    [TOKEN_MUL] = "*",
    [TOKEN_DIV] = "/",
    [TOKEN_AND] = "&amp;",
    [TOKEN_ADD] = "+",
    [TOKEN_SUB] = "-",
    [TOKEN_OR] = "|",
    [TOKEN_EQ] = "=",
    [TOKEN_LT] = "&lt;",
    [TOKEN_GT] = "&gt;",
};

Internal const char* token_kind_name(TokenKind kind) {
    if (kind < sizeof(token_kind_names) / sizeof(*token_kind_names)) {
        return token_kind_names[kind];
    }
    else {
        return "<unknown>";
    }
}

typedef struct SrcPos {
    const char* name;
    i32 line;
} SrcPos;

typedef struct Token {
    TokenKind kind;
    SrcPos pos;
    const char* start;
    const char* end;
    union {
        i32 int_val;
        const char* str_val;
        const char* name;
    };
} Token;

Token token;
const char* stream;
const char* line_start;
char* file_buf;

void error(SrcPos pos, const char* fmt, ...) {
    va_list args;
    va_start(args, fmt);
    printf("%s(%d): ", pos.name, pos.line);
    vprintf(fmt, args);
    printf("\n");
    va_end(args);
}

#define fatal_error(...) (error(__VA_ARGS__), exit(1))
#define syntax_error(...) (error(token.pos, __VA_ARGS__))
#define fatal_syntax_error(...) (syntax_error(__VA_ARGS__), exit(1))

Internal const char* token_info(void) {
    if (token.kind == TOKEN_NAME || token.kind == TOKEN_KEYWORD) {
        return token.name;
    }
    else {
        return token_kind_name(token.kind);
    }
}

char escape_to_char[256] = {
    ['n'] = '\n',
    ['r'] = '\r',
    ['t'] = '\t',
    ['v'] = '\v',
    ['b'] = '\b',
    ['a'] = '\a',
    ['0'] = 0,
};

Internal void scan_str() {
    assert(*stream == '"');
    stream++;
    char* str = NULL;
    while (*stream && *stream != '"') {
        char val = *stream;
        if (val == '\n') {
            syntax_error("String literal cannot contain newline");
            break;
        }
        else if (val == '\\') {
            stream++;
            val = escape_to_char[*(unsigned char*)stream];
            if (val == 0 && *stream != '0') {
                syntax_error("Invalid string literal escape '\\%c'", *stream);
            }
        }

        BUF_PUSH(str, val);
        stream++;
    }

    if (*stream) {
        assert(*stream == '"');
        stream++;
    }
    else {
        syntax_error("Unexpected end of file within string literal");
    }

    BUF_PUSH(str, 0);
    token.kind = TOKEN_STR;
    token.str_val = str;
}

u8 char_to_digit(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }

    return 0;
}

Internal void scan_int() {
    i32 base = 10;
    i32 val = 0;
    while (true) {
        i32 digit = char_to_digit(*stream);
        if (digit == 0 && *stream != '0') {
            break;
        }

        val = val * base + digit;
        stream++;
    }

    token.kind = TOKEN_INT;
    token.int_val = val;
}

Internal void xml_token();

Internal void next_token(void) {
repeat:
    token.start = stream;
    switch (*stream) {
        case ' ': case '\n': case '\r': case '\t': case '\v': {
            while (isspace(*stream))
            {
                if (*stream++ == '\n') {
                    line_start = stream;
                    token.pos.line++;
                }
            }
            goto repeat;
        }
        case '"': {
            scan_str();
            break;
        }
        case '.': {
            token.kind = TOKEN_DOT;
            stream++;
            break;
        }
        case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': {
            scan_int();
            break;
        }
        case 'a': case 'b': case 'c': case 'd': case 'e': case 'f': case 'g': case 'h': case 'i': case 'j':
        case 'k': case 'l': case 'm': case 'n': case 'o': case 'p': case 'q': case 'r': case 's': case 't':
        case 'u': case 'v': case 'w': case 'x': case 'y': case 'z':
        case 'A': case 'B': case 'C': case 'D': case 'E': case 'F': case 'G': case 'H': case 'I': case 'J':
        case 'K': case 'L': case 'M': case 'N': case 'O': case 'P': case 'Q': case 'R': case 'S': case 'T':
        case 'U': case 'V': case 'W': case 'X': case 'Y': case 'Z':
        case '_': {
            while (isalnum(*stream) || *stream == '_') {
                stream++;
            }
            token.name = str_intern_range(token.start, stream);
            token.kind = is_keyword_name(token.name) ? TOKEN_KEYWORD : TOKEN_NAME;
            break;
        }
        case '\0': {
            token.kind = TOKEN_EOF;
            break;
        }
        case '[': {
            token.kind = TOKEN_LBRACKET;
            stream++;
            break;
        }
        case ']': {
            token.kind = TOKEN_RBRACKET;
            stream++;
            break;
        }
        case '(': {
            token.kind = TOKEN_LPAREN;
            stream++;
            break;
        }
        case ')': {
            token.kind = TOKEN_RPAREN;
            stream++;
            break;
        }
        case '{': {
            token.kind = TOKEN_LBRACE;
            stream++;
            break;
        }
        case '}': {
            token.kind = TOKEN_RBRACE;
            stream++;
            break;
        }
        case ',': {
            token.kind = TOKEN_COMMA;
            stream++;
            break;
        }
        case ';': {
            token.kind = TOKEN_SEMICOLON;
            stream++;
            break;
        }
        case '~': {
            token.kind = TOKEN_NOT;
            stream++;
            break;
        }
        case '*': {
            token.kind = TOKEN_MUL;
            stream++;
            break;
        }
        case '/': {
            token.kind = TOKEN_DIV;
            stream++;
            if (*stream == '/') {
                stream++;
                while (*stream && *stream != '\n') {
                    stream++;
                }

                goto repeat;
            }
            else if (*stream == '*') {
                stream++;
                while (*stream) {
                    if (*stream == '*') {
                        stream++;
                        if (*stream == '/') {
                            stream++;
                            break;
                        }
                    }
                    stream++;
                }

                goto repeat;
            }
            break;
        }
        case '&': {
            token.kind = TOKEN_AND;
            stream++;
            break;
        }
        case '+': {
            token.kind = TOKEN_ADD;
            stream++;
            break;
        }
        case '-': {
            token.kind = TOKEN_SUB;
            stream++;
            break;
        }
        case '|': {
            token.kind = TOKEN_OR;
            stream++;
            break;
        }
        case '=': {
            token.kind = TOKEN_EQ;
            stream++;
            break;
        }
        case '<': {
            token.kind = TOKEN_LT;
            stream++;
            break;
        }
        case '>': {
            token.kind = TOKEN_GT;
            stream++;
            break;
        }
        default: {
            syntax_error("Invalid '%c' token, skipping", *stream);
            stream++;
            goto repeat;
        }
    }
    token.end = stream;

    xml_token();
    // printf("str: %s\n", token_info());
}

Internal void init_stream(const char* name, const char* buf) {
    stream = buf;
    line_start = stream;
    token.pos.name = name ? name : "<string>";
    token.pos.line = 1;
    next_token();
}


Internal bool is_token_eof(void) {
    return token.kind == TOKEN_EOF;
}

Internal bool is_token(TokenKind kind) {
    return token.kind == kind;
}

Internal bool is_token_symbol() {
    if ((token.kind >= TOKEN_LBRACKET && token.kind <= TOKEN_NOT) || (token.kind >= TOKEN_FIRST_MUL && token.kind <= TOKEN_LAST_CMP)) {
        return true;
    }

    return false;
}

Internal bool match_token(TokenKind kind) {
    if (is_token(kind)) {
        next_token();
        return true;
    }
    else {
        return false;
    }
}

Internal bool expect_token(TokenKind kind) {
    if (is_token(kind)) {
        next_token();
        return true;
    }
    else {
        fatal_syntax_error("expected token %s, got %s", token_kind_name(kind), token_info());
        // return false;
    }
}

Internal void print_xml(const char* tag, const char* name) {
    BUF_PRINTF(file_buf, "<%s> %s </%s>\n", tag, name, tag);
}

Internal void print_xml_int(const char* tag, i32 int_val) {
    BUF_PRINTF(file_buf, "<%s> %d </%s>\n", tag, int_val, tag);
}

Internal void xml_token() {
    if (is_token(TOKEN_KEYWORD)) {
        print_xml("keyword", token.name);
    }
    else if (is_token(TOKEN_NAME)) {
        print_xml("identifier", token.name);
    }
    else if (is_token_symbol()) {
        print_xml("symbol", token_kind_name(token.kind));
    }
    else if (is_token(TOKEN_STR)) {
        print_xml("stringConstant", token.str_val);
    }
    else if (is_token(TOKEN_INT)) {
        print_xml_int("integerConstant", token.int_val);
    }
}

Internal void keyword_tests(void) {
    init_keywords();
    assert(is_keyword_name(first_keyword));
    assert(is_keyword_name(last_keyword));

    for (const char** it = keywords; it != BUF_END(keywords); it++) {
        assert(is_keyword_name(*it));
    }

    assert(!is_keyword_name(str_intern("foo")));
}

#define assert_token(x) assert(match_token(x))
#define assert_token_name(x) assert(token.name == str_intern(x) && match_token(TOKEN_NAME))
#define assert_token_int(x) assert(token.int_val == (x) && match_token(TOKEN_INT));
#define assert_token_str(x) assert(strcmp(token.str_val, (x)) == 0 && match_token(TOKEN_STR))
#define assert_token_eof() assert(is_token(0))

Internal void lex_tests(void) {
    keyword_tests();
    assert(str_intern("function") == function_keyword);

    // Integer literal tests
    init_stream(NULL, "0 2147483647 042");
    assert_token_int(0);
    assert_token_int(2147483647);
    assert_token_int(42);
    assert_token_eof();

    // String literal tests
    init_stream(NULL, "\"foo\" \"a\\nb\"");
    assert_token_str("foo");
    assert(token.kind == TOKEN_STR);
    assert_token_str("a\nb");
    assert_token_eof();

    // Operator tests
    init_stream(NULL, "- + < > = / * &");
    assert_token(TOKEN_SUB);
    assert_token(TOKEN_ADD);
    assert_token(TOKEN_LT);
    assert_token(TOKEN_GT);
    assert_token(TOKEN_EQ);
    assert_token(TOKEN_DIV);
    assert_token(TOKEN_MUL);
    assert_token(TOKEN_AND);

    // Misc tests
    init_stream(NULL, "XY+(XY)_HELLO1,234+994");
    assert_token_name("XY");
    assert_token(TOKEN_ADD);
    assert_token(TOKEN_LPAREN);
    assert_token_name("XY");
    assert_token(TOKEN_RPAREN);
    assert_token_name("_HELLO1");
    assert_token(TOKEN_COMMA);
    assert_token_int(234);
    assert_token(TOKEN_ADD);
    assert_token_int(994);
    assert_token_eof();
}

#undef assert_token
#undef assert_token_name
#undef assert_token_int
#undef assert_token_str
#undef assert_token_eof

Internal void lex(const char* filestream) {
    init_keywords();

    BUF_PRINTF(file_buf, "<tokens>\n");
    init_stream(NULL, filestream);
    while (!is_token_eof()) {
        next_token();
    }
    BUF_PRINTF(file_buf, "</tokens>\n");
}