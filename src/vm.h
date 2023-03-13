#ifndef clox_vm_h
#define clox_vm_h

#include "chunk.h"
#include "value.h"
#include "table.h"
#include "object.h"

#define FRAMES_MAX 64
#define STACK_MAX (FRAMES_MAX + UINT8_COUNT)   //Are 8 bytes enough?

typedef struct {
    obj_function *function;
    uint8_t *ip;
    Val *slots;
} call_frame;

typedef struct {
    call_frame frame[FRAMES_MAX];
    int frame_count;
    Val stack[STACK_MAX]; //My stack based proglang!
    Val *stack_top;
    table strings; //String interning
    table globals;
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
