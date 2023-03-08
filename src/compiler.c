#include <stdio.h>
#include <stdlib.h>
#include "common.h"
#include "scanner.h"
#include "compiler.h"
/* #include "vm.h" */

typedef struct {
    token current;
    token previous;
    bool had_error;
    bool panic;
} parser;

parser parser_obj;
Chunk *compile_chunk;

static Chunk *current_chunk() {
    return compile_chunk;
}

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
        printf("TOKEN_ERROR");
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
    printf("Written into chunk");
    writeChunk(current_chunk(), byte, parser_obj.previous.line);
}

static void emit_two_bytes(uint8_t byte1, uint8_t byte2){
    emit_byte(byte1);
    emit_byte(byte2);
}


static void emit_return() {
    emit_byte(OP_RETURN);
}

static uint8_t make_constant(double value) {
    int constant = add_const(current_chunk(), value);
    if(constant > UINT8_MAX) {
        error("byte overflow in chunk");
        return 0;
    }

    return (uint8_t)constant;
}

static void emit_constant(double value) {
    emit_two_bytes(OP_CONSTANT, make_constant(value));
}

static void wrap_compiler() {
    emit_return();
}

static void number() {
    double value = strtod(parser_obj.previous.start, NULL);
    emit_constant(value);
}

static void expression() {
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
    return parser_obj.had_error;

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
