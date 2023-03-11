#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "value.h"
#include "table.h"

#define STACK_MAX 256   //Are 8 bytes enough?

typedef struct {
    Chunk *chunk;
    uint8_t *ip;
    Val stack[STACK_MAX]; //My stack based proglang!
    Val *stack_top;
    table strings; //String interning
    /* point to the head of the object heap */
    Obj *objects;
} VM;

typedef enum {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR
} interpreted_result;

/* externally expose the vm module */
extern VM vm;

void init_vm();
void free_vm();
interpreted_result interpret(const char *source);

void push(Val value);
Val pop();

#endif
