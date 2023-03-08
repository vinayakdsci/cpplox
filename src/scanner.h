#ifndef clox_scanner_h
#define clox_scanner_h

/* Define the varous tokens that the language uses  */
typedef enum {
    /* single_char tokens */
    TOKEN_LEFT_PAREN, TOKEN_RIGHT_PAREN,
    TOKEN_LEFT_BRACE, TOKEN_RIGHT_BRACE,
    TOKEN_COMMA, TOKEN_PERIOD, TOKEN_MINUS, TOKEN_PLUS,
    TOKEN_SEMICOLON, TOKEN_SLASH, TOKEN_STAR,
    /* One or More than one character tokens */
    TOKEN_BANG, TOKEN_BANG_EQUAL,
    TOKEN_EQUAL, TOKEN_EQUAL_EQUAL,
    TOKEN_GREATER, TOKEN_GREATER_EQUAL,
    TOKEN_LESS, TOKEN_LESS_EQUAL,
    /* Literal */
    TOKEN_IDENTIFIER, TOKEN_STRING, TOKEN_NUMBER,
    /* Keywords */ 
    TOKEN_CLASS, TOKEN_AND, TOKEN_ELSE, TOKEN_IF,
    TOKEN_FALSE, TOKEN_FUN, TOKEN_NIL, TOKEN_OR, TOKEN_FOR,
    TOKEN_PRINT, TOKEN_RETURN, TOKEN_SUPER, TOKEN_THIS,
    TOKEN_TRUE, TOKEN_VAR, TOKEN_WHILE,
    TOKEN_ERROR, TOKEN_EOF

} token_type;


typedef struct {
    token_type type;
    const char *start;
    int length;
    int line;
} token;


void init_scanner(const char *source);
token scan_token();
#endif
