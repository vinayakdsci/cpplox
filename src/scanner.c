#include <stdio.h>
#include <string.h>

#include "vm.h"
#include "scanner.h"

typedef struct {
    const char *start;
    const char *current;
    int line;
} scanner;


scanner scanner_object;

void init_scanner(const char *source) {
    scanner_object.start = source;
    scanner_object.current = source;
    scanner_object.line = 1;
}

static bool is_at_end() {
    return *scanner_object.current == '\0';
}

static char advance() {
    scanner_object.current++;
    return scanner_object.current[-1];
    //Also written as scanner_object[-1].
}
static char peek() {
    return *scanner_object.current;
}
static char peek_next() {
    if(is_at_end()) return '\0';
    return scanner_object.current[1];
}
static bool match(char c) {
    if(is_at_end()) return false;
    if(*scanner_object.current != c) return false;

    scanner_object.current++;
    return true;
}
static bool is_digit(char c) {
    return c >= '0' && c <= '9';
}
static bool is_alpha(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}


static token make_token(token_type type){
    token token_object;
    token_object.type = type;
    token_object.start = scanner_object.start;
    token_object.length = (int)(scanner_object.current - scanner_object.start);
    token_object.line = scanner_object.line;

    return token_object;
}

static token error_token(const char *message){
    token token_obj;
    token_obj.type = TOKEN_ERROR;
    token_obj.start = message;
    token_obj.length = strlen(message);
    token_obj.line = scanner_object.line;
    return token_obj;
}

static token string() {
    /* Traverse the whole string */
    while(peek() != '"' && !is_at_end()){
        if(peek() == '\n') scanner_object.line++;
        advance();
    }
    if(is_at_end()) {
        return error_token("unterminated string");
    }
    advance();
    return make_token(TOKEN_STRING);
}
static token number() {
    while(is_digit(peek())) advance(); 
    /* check for a decimal place */
    if(peek() == '.' && is_digit(peek_next())) {
        advance();
        while(is_digit(peek())) advance();
    }
    return make_token(TOKEN_NUMBER);
}


static token_type check_key(int start, int count, const char *rem, token_type type){
    if(scanner_object.current - scanner_object.start == start + count &&
            memcmp(scanner_object.start + start, rem, count) == 0) {
        return type;

    }
    return TOKEN_IDENTIFIER;
}

static token_type identifier_type() {
    /* Checking for keywords through a trie */
    switch(scanner_object.start[0]) {
        case 'a' : return check_key(1, 2, "nd", TOKEN_AND);
        case 'c' : return check_key(1, 4, "lass", TOKEN_CLASS);
        case 'e' : return check_key(1, 3, "lse", TOKEN_ELSE);
        case 'o' : return check_key(1, 1, "r", TOKEN_OR);
        case 'n' : return check_key(1, 2, "il", TOKEN_NIL);
        case 'i' : return check_key(1, 1, "f", TOKEN_IF);
        case 'p' : return check_key(1, 4, "rint", TOKEN_PRINT);
        case 'v' : return check_key(1, 2, "ar", TOKEN_VAR);
        case 'r' : return check_key(1, 5, "eturn", TOKEN_RETURN);
        case 's' : return check_key(1, 4, "uper", TOKEN_SUPER);
        case 'w' : return check_key(1, 4, "hile", TOKEN_WHILE);
        case 'f' :
                   if (scanner_object.current - scanner_object.start > 1) {
                       switch (scanner_object.start[1]) {
                           case 'o': return check_key(2, 1, "r", TOKEN_FOR);
                           case 'u': return check_key(2, 1, "n", TOKEN_FUN);
                           case 'a': return check_key(2, 3, "lse", TOKEN_FALSE);
                       }
                   }
                   break;
        case 't' :
                   if(scanner_object.current - scanner_object.start > 1) {
                       switch(scanner_object.start[1]) {
                           case 'r': return check_key(2, 2, "ue", TOKEN_TRUE);
                           case 'h': return check_key(2, 2, "is", TOKEN_THIS);
                       }
                   }
                   break;
    }



    return TOKEN_IDENTIFIER;
}

static token identifier() {
    while(is_alpha(peek()) || is_digit(peek())) advance();
    return make_token(identifier_type());
}



static void skip_whitespace() {
    for(;;){
        char cur = *scanner_object.current;
        switch(cur) {
            case ' ':
            case '\r':
            case '\t':
                advance();
                break;
            case '\n':
                scanner_object.line++;
                advance();
                break;
            case '/':
                if(peek_next() == '/'){
                    while(peek() != '\n' && !is_at_end()) advance();
                }
                else {
                    return ;
                }
                break;
            default:
                return;
        }

    }
}

token scan_token() {
    skip_whitespace();
    scanner_object.start = scanner_object.current;
    if(is_at_end()) return make_token(TOKEN_EOF);

    /* Define grammar for single char tokens */
    char c = advance();
    if(is_alpha(c)) return identifier();
    if(is_digit(c)) return number();

    switch(c) {
        case '{' : return make_token(TOKEN_LEFT_BRACE);
        case '}' : return make_token(TOKEN_RIGHT_BRACE);
        case '(' : return make_token(TOKEN_LEFT_PAREN);
        case ')' : return make_token(TOKEN_RIGHT_PAREN);
        case ';' : return make_token(TOKEN_SEMICOLON);
        case '.' : return make_token(TOKEN_PERIOD);
        case ',' : return make_token(TOKEN_COMMA);
        case '-' : return make_token(TOKEN_MINUS);
        case '+' : return make_token(TOKEN_PLUS);
        case '/' : return make_token(TOKEN_SLASH);
        case '*' : return make_token(TOKEN_STAR);
                   /* Handle two character tokens */
        case '!':
                   return make_token(match('=') ? TOKEN_BANG_EQUAL : TOKEN_BANG);
        case '=':
                   return make_token(match('=') ? TOKEN_EQUAL_EQUAL : TOKEN_EQUAL);
        case '>':
                   return make_token(match('=') ? TOKEN_GREATER_EQUAL : TOKEN_GREATER);
        case '<':
                   return make_token(match('=') ? TOKEN_LESS_EQUAL : TOKEN_LESS);
        case '"':  return string();
    }

    return error_token("encountered unexpected character\n");
}
