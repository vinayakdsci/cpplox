#include <stdio.h>
#include "common.h"
#include "debug.h"
#include "vm.h"
#include "compiler.h"

/* Global decl */
VM vm;

static void reset_stack() {
    vm.stack_top = vm.stack;    //set stack_top to the beginning of the array to indecate it is empty
}

void push(Val value) {
    *vm.stack_top = value;
    vm.stack_top++;
}

Val pop() {
    vm.stack_top--;
    return *vm.stack_top;
}

/*
 *The Run function is the core of the VM, the heart that pumps blood to the body
 */

void init_vm() {
    reset_stack();
}


void free_vm() {

}


static interpreted_result run (void) {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])

#define BIN_OP(op) \
    do { \
        Val b = pop(); \
        Val a = pop();  \
        push(a op b);  \
    } while(false)  //Execute only once

    for (;;) {
        uint8_t instruction;
        switch(instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                                  Val constant = READ_CONSTANT();
                                  print_val(constant);
                                  printf("\n");
                                  push(constant);
                                  break;
                              }

            case OP_NEGATE:     push(-pop()); break;
            case OP_ADD:        BIN_OP(+);    break;
            case OP_SUBTRACT:   BIN_OP(-);    break;
            case OP_MULTIPLY:   BIN_OP(*);    break;
            case OP_DIVIDE:     BIN_OP(/);    break;

            case OP_RETURN: { 
                                print_val(pop()); 
                                printf("\n");
                                return INTERPRET_OK; 
                            }
        }
    }
#undef READ_BYTE
#undef READ_CONSTANT
#undef BIN_OP
}


interpreted_result interpret(const char *source) {
    /* vm.chunk = chunk; */
    /* vm.ip = vm.chunk->code; */
    /* return run(); */
    compile(source);
    return INTERPRET_OK;
}

