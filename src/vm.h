#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "value.h"

#define STACK_MAX 256   //Are 8 bytes enough?

typedef struct {
    Chunk *chunk;
    uint8_t *ip;
    Val stack[STACK_MAX]; //My stack based proglang!
    Val *stack_top;
} VM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} interpreted_result;

void init_vm();
void free_vm();
interpreted_result interpret(Chunk *chunk);

void push(Val value);
Val pop();

#endif
