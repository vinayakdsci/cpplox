#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "scanner.h"
#include "compiler.h"
/* #include "vm.h" */
#ifdef DEBUG_PRINT_CODE
#include "debug.h"
#endif

typedef struct {
    token current;
    token previous;
    bool had_error;
    bool panic;
} parser;

/* precedence from the lowest to the highest */
typedef enum {
    PREC_NONE,
    PREC_ASSIGNMENT,
    PREC_OR,
    PREC_AND,
    PREC_EQUALITY,
    PREC_COMPARISON,
    PREC_TERM,
    PREC_FACTOR,
    PREC_UNARY,
    PREC_CALL,
    PREC_PRIMARY
} precedence;

/* handling function pointers kind of sucks, so typedef */
typedef void (*parse_fun)(void);

typedef struct {
    parse_fun prefix;
    parse_fun infix;
    precedence prec;
} parse_rule;


parser parser_obj;
Chunk *compile_chunk;

static Chunk *current_chunk() {
    return compile_chunk;
}

/* define the precedence of operators */

static void error_at(token *tok, const char *message) {
    if(parser_obj.panic)
        return;

    parser_obj.panic = true;

    /* print the line of error first */
    fprintf(stderr, "[line %d] error ", tok->line);
    if(tok->type == TOKEN_EOF) {
        fprintf(stderr, "reached end of file while parsing");
    }
    else if(tok->type == TOKEN_ERROR){
        /* printf("TOKEN_ERROR"); */
    }
    else {
        fprintf(stderr, "at %.*s", tok->length, tok->start);
    }

    fprintf(stderr, ": %s\n", message);
    parser_obj.had_error = true;

}


static void error(const char *message) {
    error_at(&parser_obj.current, message);
}

static void advance() {
    parser_obj.previous = parser_obj.current;


    for(;;) {
        parser_obj.current = scan_token();
        if(parser_obj.current.type != TOKEN_ERROR) break;

        error(parser_obj.current.start);
    }
}

static void consume(token_type type, const char *message) {
    if(parser_obj.current.type == type){
        advance();
        return;
    }

    error(message);
}

static void emit_byte(uint8_t byte) {
    /* printf("Written into chunk"); */
    writeChunk(current_chunk(), byte, parser_obj.previous.line);
}

static void emit_two_bytes(uint8_t byte1, uint8_t byte2){
    emit_byte(byte1);
    emit_byte(byte2);
}


static void emit_return() {
    emit_byte(OP_RETURN);
}

static uint8_t make_constant(Val value) {
    int constant = add_const(current_chunk(), value);
    if(constant > UINT8_MAX) {
        error("byte overflow in chunk");
        return 0;
    }

    return (uint8_t)constant;
}

static void emit_constant(Val value) {
    emit_two_bytes(OP_CONSTANT, make_constant(value));
}

static void wrap_compiler() {
    emit_return();

#ifdef DEBUG_PRINT_CODE
    if(!parser_obj.had_error) {
        disassembleChunk(current_chunk(), "code");
    }
#endif

}

/* forward declare to provide access */
static void expression();
static parse_rule *get_rule(token_type type);
static void parse_precedence(precedence precede);

/* parse infix expression */
static void binary() {
    token_type op_type = parser_obj.previous.type;
    /* printf("%d", opt_type); */
    parse_rule *rule = get_rule(op_type);
    parse_precedence((precedence) (rule->prec + 1));

    switch(op_type) {
        case TOKEN_MINUS: emit_byte(OP_SUBTRACT); break;
        case TOKEN_PLUS:  emit_byte(OP_ADD); break;
        case TOKEN_SLASH: emit_byte(OP_DIVIDE); break;
        case TOKEN_STAR:  emit_byte(OP_MULTIPLY); break;
        default: return;
    }
}

static void grouping() {
    /* group(associate) by brackets */
    /* expression(); //compile the expr */
    consume(TOKEN_RIGHT_PAREN, "expected ')' after expression.");
}



static void number() {
    double value = strtod(parser_obj.previous.start, NULL);
    emit_constant(NUMBER_VAL(value));
}


/* Unary definitions */
static void unary() {
    /*
     * check if the previous token was a negative sign.
     * if yes, push -x onto the stack. 
     */
    token_type op_type = parser_obj.previous.type;

    /* compile */
    /* expression(); */

    parse_precedence(PREC_UNARY);

    switch(op_type) {
        case TOKEN_MINUS: emit_byte(OP_NEGATE); break; 
        default:          return;   
    }
}


parse_rule rules[] = {
    [TOKEN_LEFT_PAREN]      = {grouping, NULL, PREC_NONE},
    [TOKEN_RIGHT_PAREN]     = {NULL, NULL, PREC_NONE},
    [TOKEN_LEFT_BRACE]      = {NULL, NULL, PREC_NONE},
    [TOKEN_RIGHT_BRACE]     = {NULL, NULL, PREC_NONE},
    [TOKEN_COMMA]           = {NULL, NULL, PREC_NONE},
    [TOKEN_PERIOD]          = {NULL, NULL, PREC_NONE},
    [TOKEN_MINUS]           = {unary, binary, PREC_TERM},
    [TOKEN_PLUS]            = {NULL, binary, PREC_TERM},
    [TOKEN_SEMICOLON]       = {NULL, NULL, PREC_NONE},
    [TOKEN_SLASH]           = {NULL, binary, PREC_FACTOR},
    [TOKEN_STAR]            = {NULL, binary, PREC_FACTOR},
    [TOKEN_BANG]            = {NULL, NULL, PREC_NONE},
    [TOKEN_BANG_EQUAL]      = {NULL, NULL, PREC_NONE},
    [TOKEN_EQUAL]           = {NULL, NULL, PREC_NONE},
    [TOKEN_EQUAL_EQUAL]     = {NULL, NULL, PREC_NONE},
    [TOKEN_GREATER]         = {NULL, NULL, PREC_NONE},
    [TOKEN_GREATER_EQUAL]   = {NULL, NULL, PREC_NONE},
    [TOKEN_LESS]            = {NULL, NULL, PREC_NONE},
    [TOKEN_LESS_EQUAL]      = {NULL, NULL, PREC_NONE},
    [TOKEN_IDENTIFIER]      = {NULL, NULL, PREC_NONE},
    [TOKEN_STRING]          = {NULL, NULL, PREC_NONE},
    [TOKEN_NUMBER]          = {number, NULL, PREC_NONE},
    [TOKEN_AND]             = {NULL, NULL, PREC_NONE},
    [TOKEN_CLASS]           = {NULL, NULL, PREC_NONE},
    [TOKEN_ELSE]            = {NULL, NULL, PREC_NONE},
    [TOKEN_IF]              = {NULL, NULL, PREC_NONE},
    [TOKEN_FALSE]           = {NULL, NULL, PREC_NONE},
    [TOKEN_FUN]             = {NULL, NULL, PREC_NONE},
    [TOKEN_NIL]             = {NULL, NULL, PREC_NONE},
    [TOKEN_OR]              = {NULL, NULL, PREC_NONE},
    [TOKEN_FOR]             = {NULL, NULL, PREC_NONE},
    [TOKEN_PRINT]           = {NULL, NULL, PREC_NONE},
    [TOKEN_RETURN]          = {NULL, NULL, PREC_NONE},
    [TOKEN_SUPER]           = {NULL, NULL, PREC_NONE},
    [TOKEN_THIS]            = {NULL, NULL, PREC_NONE},
    [TOKEN_TRUE]            = {NULL, NULL, PREC_NONE},
    [TOKEN_VAR]             = {NULL, NULL, PREC_NONE},
    [TOKEN_WHILE]           = {NULL, NULL, PREC_NONE},
    [TOKEN_ERROR]           = {NULL, NULL, PREC_NONE},
    [TOKEN_EOF]             = {NULL, NULL, PREC_NONE}
};



static void parse_precedence(precedence precede){
    /* parse prefix expr */
    /* printf("%d", parser_obj.previous.type); */
    advance();
    parse_fun prefix_rule = get_rule(parser_obj.previous.type)->prefix;
    if(prefix_rule == NULL){
        error("prefix error: expected expression");
        return;
    }

    prefix_rule();

    /* infix expr employ precedence 
     * the first token is always a prefix expr from left to right
     * after parsing the prefix expression, look for rules to parse the infix expr
     * if one is found, the just-compiled prefix should be an operand for it
     * proceed only till the next token has higher precedence
     */

    while(precede <= get_rule(parser_obj.current.type)->prec){
        advance(); //consume the next token
        parse_fun infix_rule = get_rule(parser_obj.previous.type)->infix;
        infix_rule();
    }
}


static parse_rule *get_rule(token_type type) {
    return &rules[type];
}

static void expression() {
    parse_precedence(PREC_ASSIGNMENT);
}




bool compile(const char *source, Chunk *chunk) {
    init_scanner(source);
    compile_chunk = chunk;

    parser_obj.had_error = false;
    parser_obj.panic = false;
    advance();
    expression();
    consume(TOKEN_EOF, "Expected end of expression.");
    wrap_compiler();
    return !parser_obj.had_error;

}


/* 
 * the language at the moment consists of only
 * arithmetic and literal expression.
 *
 * to compile each kind of literal, we map each type of token to an
 * expression, and then create an array of function pointers,
 * each of which points to the specific function code for compilation of
 * the token type represented by the index of the array.
 *
 * */
