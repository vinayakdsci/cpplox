#include "compiler.h"
#include "common.h"
#include "scanner.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
typedef void (*parse_fun)(bool assignable);

typedef struct {
    parse_fun prefix;
    parse_fun infix;
    precedence prec;
} parse_rule;

typedef struct {
    token name;
    int depth;
} local;

typedef enum {
    type_function,
    type_script
} function_type;

/* definition for locals allocated on the clox stack */
typedef struct compiler {
    struct compiler *encl;
    obj_function *function;
    function_type type;

    local locals[UINT8_COUNT]; //array order analogous to declaration
    int local_count;
    int scope_depth;
} compiler;

parser parser_obj;
compiler *cur = NULL;
Chunk *compile_chunk;

static Chunk *current_chunk() {
    return &cur->function->chunk;
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


static void emit_byte(uint8_t byte) {
    /* printf("Written into chunk"); */
    writeChunk(current_chunk(), byte, parser_obj.previous.line);
}

static void emit_two_bytes(uint8_t byte1, uint8_t byte2){
    emit_byte(byte1);
    emit_byte(byte2);
}


static void emit_return() {
    emit_byte(OP_NIL);
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

static void init_compiler(compiler *comp, function_type type) {
    comp->encl = cur;
    comp->function = NULL;
    comp->type = type;
    comp->local_count = 0;
    comp->scope_depth = 0;
    //new function to compile into
    comp->function = new_function();
    cur = comp;
    if(type != type_script) {
        cur->function->name = copy_string(parser_obj.previous.start, parser_obj.previous.length);
    }
    local *loc = &cur->locals[cur->local_count++];
    loc->depth = 0;
    loc->name.start = "";
    loc->name.length = 0;
}

static obj_function *wrap_compiler() {
    emit_return();
    obj_function *function = cur->function;

#ifdef DEBUG_PRINT_CODE
    if(!parser_obj.had_error) {
        disassembleChunk(current_chunk(), function->name != NULL ? function->name->chars : "<script>");
    }
#endif
    /* when the fn compiler is done , simply pops itself off the stack
     * by setting the prv compiler to be the current one.
     * */
    cur = cur->encl;
    return function;
}

/* forward declare to provide access */
static void expression();
static void statement();
static void declaration();
static parse_rule *get_rule(token_type type);
static void parse_precedence(precedence precede);

/* parse infix expression */
static void binary(bool assignable) {
    token_type op_type = parser_obj.previous.type;
    /* printf("%d", opt_type); */
    parse_rule *rule = get_rule(op_type);
    parse_precedence((precedence) (rule->prec + 1));

    switch(op_type) {
        case TOKEN_BANG_EQUAL:    emit_two_bytes(OP_EQUAL, OP_NOT); break;
        case TOKEN_EQUAL_EQUAL:   emit_byte(OP_EQUAL); break;
        case TOKEN_GREATER:       emit_byte(OP_GREATER); break;
        case TOKEN_GREATER_EQUAL: emit_two_bytes(OP_LESS, OP_NOT); break;
        case TOKEN_LESS:        emit_byte(OP_LESS); break;                       
        case TOKEN_LESS_EQUAL:  emit_two_bytes(OP_GREATER, OP_NOT); break;
        case TOKEN_MINUS:       emit_byte(OP_SUBTRACT); break;
        case TOKEN_PLUS:        emit_byte(OP_ADD); break;
        case TOKEN_SLASH:       emit_byte(OP_DIVIDE); break;
        case TOKEN_STAR:        emit_byte(OP_MULTIPLY); break;
        default: return;
    }
}

/* compile function for true,false,nil */
static void literal(bool assignable) {
    switch(parser_obj.previous.type) {
        case TOKEN_FALSE: emit_byte(OP_FALSE); break;
        case TOKEN_TRUE:  emit_byte(OP_TRUE); break;
        case TOKEN_NIL:   emit_byte(OP_NIL); break;
        default: return;
    }
}

static void consume(token_type type, const char *message) {
    if(parser_obj.current.type == type){
        advance();
        return;
    }

    error(message);
}

static void grouping(bool assignable) {
    /* group(associate) by brackets */
    expression(); //compile the expr
    consume(TOKEN_RIGHT_PAREN, "expected ')' after expression.");
}



static void number(bool assignable) {
    double value = strtod(parser_obj.previous.start, NULL);
    emit_constant(NUMBER_VAL(value));
}

static void string(bool assignable) {
    emit_constant(OBJ_VAL(copy_string(parser_obj.previous.start + 1, parser_obj.previous.length - 2)));
        /* trim the first and the last quote */
}
static uint8_t iden_constant(token *tok) {
    /* add the lexeme of the given token to the chunk constant table 
     * return the respective index
     * global vars should be looked up by their names at runtime.
     * therefore, too avoid storing the whole string into the bytecode stream,
     * save it in the constant table
     * */
    return make_constant(OBJ_VAL(copy_string(tok->start, tok->length)));
}

static void add_local(token name) {
    if(cur->local_count == UINT8_COUNT) {
        error("too many local variables declared in function");
        return;
    }

    local *loc = &cur->locals[cur->local_count++];
    loc->name = name;
    loc->depth = cur->scope_depth;
}

static bool iden_equal(token *a, token *b) {
    if(a->length != b->length) return false;

    return memcmp(a->start, b->start, a->length) == 0;
}

static void local_decl() {
    if(cur->scope_depth == 0) return;

    token *name = &parser_obj.previous;

    /* two variables with the same name not allowed in the same scope */
    for(int i = cur->local_count - 1; i >= 0; i--) {
        local *loc = &cur->locals[i];
        if(loc->depth != -1 && loc->depth < cur->scope_depth){
            break;
        }
        //(TODO) rename
        if(iden_equal(name, &loc->name)) {
            error("a variable with the same name already exists in this scope.");
        }
    }
    add_local(*name);
}

static bool check(token_type type) {
    /* parser already stores this info */
    return parser_obj.current.type == type;
}

static bool match(token_type t) {

    /* if we have the required type of token, 
     * consume it and ret true, else false 
     * */
    if(!check(t)) return false;
    advance();
    return true;
}

static int resolve(compiler *comp, token *name) {
    for(int i = comp->local_count - 1; i >= 0; i--) {
        local *l = &comp->locals[i];
        if(iden_equal(name, &l->name)) {
            if(l->depth == -1) {
                error("local var cannot be read in its own initializer.");
            }
            return i;
        }
    }

    return -1;
}

static void named_var(token name, bool assignable) {
    /* take the current token, add it's lexeme to the constant table */
    uint8_t get_opcode, set_opcode;
    int arg = resolve(cur, &name);
    if(arg != -1) {
        get_opcode = OP_GET_LOCAL;
        set_opcode = OP_SET_LOCAL;
    }
    else {
        arg = iden_constant(&name);
        get_opcode = OP_GET_GLOBAL;
        set_opcode = OP_SET_GLOBAL;
    }
    /* handle assignment */
    if(match(TOKEN_EQUAL) && assignable) {
        expression();
        emit_two_bytes(set_opcode, (uint8_t)arg);
    }
    else
        emit_two_bytes(get_opcode, (uint8_t)arg);
}

static void variable(bool assignable) {
    named_var(parser_obj.previous, assignable);
}

static int emit_jump(uint8_t inst) {
    emit_byte(inst);
    emit_byte(0xff);  //placeholder
    emit_byte(0xff);
    return current_chunk()->count - 2;
}

static void patch_jump(int offset) {
    int jump = current_chunk()->count - offset - 2;
    if(jump > UINT16_MAX) {
        error("too much code in the block to jump.");
    }

    current_chunk()->code[offset] = (jump >> 8) & 0xff;
    current_chunk()->code[offset + 1] = jump & 0xff;
}

static void and_ (bool assignable) {
    int end_jump = emit_jump(OP_JUMP_IF_FALSE);

    emit_byte(OP_POP);
    parse_precedence(PREC_AND);
    patch_jump(end_jump);
}

static void or_ (bool assignable) {
    int else_jump = emit_jump(OP_JUMP_IF_FALSE);
    int end_jump = emit_jump(OP_JUMP);

    patch_jump(else_jump);
    emit_byte(OP_POP);

    parse_precedence(PREC_OR);
    patch_jump(end_jump);
}


/* Unary definitions */
static void unary(bool assignable) {
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
        case TOKEN_BANG:  emit_byte(OP_NOT); break;
        default:          return;   
    }
}

static uint8_t arg_list() {
    uint8_t count = 0;
    if(!check(TOKEN_RIGHT_PAREN)) {
        do {
            expression();
            if(count == 255)
                error("can't have more than 255 params.");
            count++;
        } while(match(TOKEN_COMMA));
    }
    consume(TOKEN_RIGHT_PAREN, "expected ')' after params list.");
    return count;
}

static void call(bool assignable) {
    uint8_t arg_count = arg_list();
    emit_two_bytes(OP_CALL, arg_count);
}

parse_rule rules[] = {
    [TOKEN_LEFT_PAREN]      = {grouping, call, PREC_CALL},
    [TOKEN_RIGHT_PAREN]     = {NULL, NULL, PREC_NONE},
    [TOKEN_LEFT_BRACE]      = {NULL, NULL, PREC_NONE},
    [TOKEN_RIGHT_BRACE]     = {NULL, NULL, PREC_NONE},
    [TOKEN_COMMA]           = {NULL, NULL, PREC_NONE},
    [TOKEN_PERIOD]          = {NULL, NULL, PREC_NONE},
    [TOKEN_MINUS]           = {unary, binary, PREC_TERM},
    [TOKEN_PLUS]            = {NULL, binary, PREC_TERM},
    [TOKEN_SEMICOLON]       = {NULL, NULL, PREC_NONE},
    [TOKEN_COLON]           = {NULL, NULL, PREC_NONE},
    [TOKEN_SLASH]           = {NULL, binary, PREC_FACTOR},
    [TOKEN_STAR]            = {NULL, binary, PREC_FACTOR},
    [TOKEN_BANG]            = {unary, NULL, PREC_NONE},
    [TOKEN_BANG_EQUAL]      = {NULL, binary, PREC_EQUALITY},
    [TOKEN_EQUAL]           = {NULL, NULL, PREC_NONE},
    [TOKEN_EQUAL_EQUAL]     = {NULL, binary, PREC_EQUALITY},
    [TOKEN_GREATER]         = {NULL, binary, PREC_COMPARISON},
    [TOKEN_GREATER_EQUAL]   = {NULL, binary, PREC_COMPARISON},
    [TOKEN_LESS]            = {NULL, binary, PREC_COMPARISON},
    [TOKEN_LESS_EQUAL]      = {NULL, binary, PREC_COMPARISON},
    [TOKEN_IDENTIFIER]      = {variable, NULL, PREC_NONE},
    [TOKEN_STRING]          = {string, NULL, PREC_NONE},
    [TOKEN_NUMBER]          = {number, NULL, PREC_NONE},
    [TOKEN_AND]             = {NULL, and_, PREC_AND},
    [TOKEN_CLASS]           = {NULL, NULL, PREC_NONE},
    [TOKEN_ELSE]            = {NULL, NULL, PREC_NONE},
    [TOKEN_IF]              = {NULL, NULL, PREC_NONE},
    [TOKEN_FALSE]           = {literal, NULL, PREC_NONE},
    [TOKEN_FUN]             = {NULL, NULL, PREC_NONE},
    [TOKEN_NIL]             = {literal, NULL, PREC_NONE},
    [TOKEN_OR]              = {NULL, or_, PREC_OR},
    [TOKEN_FOR]             = {NULL, NULL, PREC_NONE},
    [TOKEN_PRINT]           = {NULL, NULL, PREC_NONE},
    [TOKEN_RETURN]          = {NULL, NULL, PREC_NONE},
    [TOKEN_SUPER]           = {NULL, NULL, PREC_NONE},
    [TOKEN_THIS]            = {NULL, NULL, PREC_NONE},
    [TOKEN_TRUE]            = {literal, NULL, PREC_NONE},
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
    /* consume an '=' only in the context of a lower precedence operator */
    bool assignable = precede <= PREC_ASSIGNMENT;
    prefix_rule(assignable);

    /* infix expr employ precedence 
     * the first token is always a prefix expr from left to right
     * after parsing the prefix expression, look for rules to parse the infix expr
     * if one is found, the just-compiled prefix should be an operand for it
     * proceed only till the next token has higher precedence
     */

    while(precede <= get_rule(parser_obj.current.type)->prec){
        advance(); //consume the next token
        parse_fun infix_rule = get_rule(parser_obj.previous.type)->infix;
        infix_rule(assignable);
    }

    if(assignable && match(TOKEN_EQUAL)){
        error("invalid assignment.");
    }
}



//(TODO) rename iden_constant
static uint8_t parse_variable(const char *err_message) {
    /* next token must be an identifier */
    consume(TOKEN_IDENTIFIER, err_message);
    local_decl();
    if(cur->scope_depth > 0)
        return 0;
    return iden_constant(&parser_obj.previous);
}

static void mark_init() {
    if(cur->scope_depth == 0) return;
    cur->locals[cur->local_count - 1].depth = cur->scope_depth;
}

static void var_define(uint8_t global) {
    /* recieves the bytecode */ 
    if(cur->scope_depth > 0) {
        mark_init();
        return;
    }

    emit_two_bytes(OP_DEF_GLOBAL, global);
}

static parse_rule *get_rule(token_type type) {
    return &rules[type];
}

static void expression() {
    parse_precedence(PREC_ASSIGNMENT);
}

static void var_declare() {
    uint8_t global_var = parse_variable("expected variable name.");

    if(match(TOKEN_EQUAL)) 
        expression();  //compile
    else 
        emit_byte(OP_NIL);
        /* essentially, when the compiler sees var variable;
         * it emits a NIL byte, essentially setting it as var variable = nil;
         * this must set .number as 0
         * */
    consume(TOKEN_SEMICOLON, "expected ';' after variable declaration");

    var_define(global_var);
}

static void print_statement() {
    expression();  //compile!
    consume(TOKEN_SEMICOLON, "expected ';' after value");
    emit_byte(OP_PRINT);
}

static void synchronize() {
    parser_obj.panic = false;

    while(parser_obj.current.type != TOKEN_EOF) {
        if(parser_obj.previous.type == TOKEN_SEMICOLON)
            return;

        switch(parser_obj.current.type) {
            case TOKEN_CLASS:
            case TOKEN_FUN:
            case TOKEN_VAR:
            case TOKEN_FOR:
            case TOKEN_IF:
            case TOKEN_WHILE:
            case TOKEN_PRINT:
            case TOKEN_RETURN:
                return;
            default:
                /* do nothing */
                ;
        }
        advance();  //keep consuming!
    }
}
static void begin_scope() {
    cur->scope_depth++;
}


static void end_scope() {
    cur->scope_depth--;
    while(cur->local_count > 0 &&
            cur->locals[cur->local_count - 1].depth > cur->scope_depth) {

        emit_byte(OP_POP);
        cur->local_count--;
    }
}

static void block() {
    while(!check(TOKEN_RIGHT_BRACE) && !check(TOKEN_EOF)) {
        declaration();
    }
    consume(TOKEN_RIGHT_BRACE, "expected '}' after block.");
}

static void function(function_type type) {
	compiler comp;
	init_compiler(&comp, type);
	begin_scope();
	consume(TOKEN_LEFT_PAREN, "expected '(' after fn name.");
        if(!check(TOKEN_RIGHT_PAREN)){
            do {
                cur->function->arity++;
                if(cur->function->arity > 255) {
                    error("more than 255 params not allowed in function call.");
                }
                uint8_t constant = parse_variable("expected param name.") ;
                var_define(constant);
            } while(match(TOKEN_COMMA));
        }
        
	consume(TOKEN_RIGHT_PAREN, "expected ')' after fn args.");
	consume(TOKEN_LEFT_BRACE, "expected '{' to begin function body.");
	block();
	obj_function *fn = wrap_compiler();
        emit_two_bytes(OP_CLOSURE, make_constant(OBJ_VAL(fn)));
	emit_two_bytes(OP_CONSTANT, make_constant(OBJ_VAL(fn)));

}

static void fn_declare() {
    uint8_t global = parse_variable("expected function name.");
    mark_init();
    function(type_function);
    var_define(global);
}

static void declaration() {
    if(match(TOKEN_FUN)) 
        fn_declare();
    else if(match(TOKEN_VAR))
        var_declare();
    else
        statement();

    if(parser_obj.panic) synchronize();
}


static void if_statement() {
    consume(TOKEN_LEFT_PAREN, "expected '(' before if");
    expression();
    consume(TOKEN_RIGHT_PAREN, "expected ')' after if");

    int now_jump = emit_jump(OP_JUMP_IF_FALSE);
    emit_byte(OP_POP);
    statement();

    int else_jump = emit_jump(OP_JUMP);
    patch_jump(now_jump);
    emit_byte(OP_POP);

    if(match(TOKEN_ELSE)){
        if(match(TOKEN_IF)) {
            if_statement();
        }
        else
            statement();
    }
    patch_jump(else_jump);

}

static void emit_loop(int loop_start) {
    emit_byte(OP_LOOP);

    int offset = current_chunk()->count - loop_start + 2;
    if(offset > UINT16_MAX) error("loop body too large.");

    emit_byte((offset >> 8) & 0xff);
    emit_byte(offset & 0xff);
}

static void while_statement() {
    int loop_start = current_chunk()->count;
    consume(TOKEN_LEFT_PAREN, "expected '(' after while.");
    expression();
    consume(TOKEN_RIGHT_PAREN, "expected ')' after condition.");

    int exit_jump = emit_jump(OP_JUMP_IF_FALSE);
    emit_byte(OP_POP);
    statement();
    emit_loop(loop_start);

    patch_jump(exit_jump);
    emit_byte(OP_POP);
}

static void for_statement() {
    begin_scope();
    consume(TOKEN_LEFT_PAREN, "expected '(' after 'for'.");
    if(match(TOKEN_SEMICOLON)) {
        //empty!
    }
    else if (match(TOKEN_VAR)) {
        var_declare();
    }
    else {
        expression();
        consume(TOKEN_SEMICOLON, "expected ';' after return statement.");
        emit_byte(OP_RETURN);
    }
    int loop_start = current_chunk()->count;

    /* condition clause */

    int exit_jump = -1;
    if(!match(TOKEN_SEMICOLON)) {
        expression();
        consume(TOKEN_SEMICOLON, "expected ';' after loop condition.");
        //exit loop for false condition
        exit_jump = emit_jump(OP_JUMP_IF_FALSE);
        emit_byte(OP_POP);
    }

    if(!match(TOKEN_RIGHT_PAREN)) {
        int body_jump = emit_jump(OP_JUMP);
        int increment = current_chunk()->count;
        expression();
        emit_byte(OP_POP);
        consume(TOKEN_RIGHT_PAREN, "expected ')' after for clauses.");

        emit_loop(loop_start);
        loop_start = increment;
        patch_jump(body_jump);
    }

    statement();
    emit_loop(loop_start);

    if(exit_jump != -1){
        patch_jump(exit_jump);
        emit_byte(OP_POP);
    }

    end_scope();
}

static void return_statement() {
    if(cur->type == type_script)
        error("can't return from top-level");

    if(match(TOKEN_SEMICOLON)) {
        emit_return();
    }
    else {
        expression();
        consume(TOKEN_SEMICOLON, "expected ';' after return statement.");
        emit_byte(OP_RETURN);
    }
}

static void statement() {
    if(match(TOKEN_PRINT)) print_statement();
    else if(match(TOKEN_IF)){
        if_statement();
    }
    else if(match(TOKEN_RETURN)){
        return_statement();
    }
    else if(match(TOKEN_WHILE)) {
        while_statement();
    }
    else if (match(TOKEN_LEFT_BRACE)) {
        begin_scope();
        block();
        end_scope();
    }
    else if(match(TOKEN_FOR)) {
        for_statement();
    }
    else {
        /* got an expression evaluation */
        expression();  //compile
        consume(TOKEN_SEMICOLON, "expected ';' after expression");
        emit_byte(OP_POP);
    }
}

obj_function *compile(const char *source) {
    init_scanner(source);
    compiler comp;
    init_compiler(&comp, type_script);

    parser_obj.had_error = false;
    parser_obj.panic = false;
    advance();

    while(!match(TOKEN_EOF)) {
        declaration();
    }

    obj_function *fun = wrap_compiler();

    return parser_obj.had_error ? NULL : fun;

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
