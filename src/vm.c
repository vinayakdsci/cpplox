#include "vm.h"
#include "common.h"
#include "compiler.h"
#include "debug.h"
#include <stdio.h>

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
                                  double constant = READ_CONSTANT();
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


#ifdef DEBUG_TRACE_EXECUTION
#include <stdio.h>
void debug(){
    printf("       ");
    for (Val *slot = vm.stack; slot < vm.stack_top; slot++) {
        printf("{ ");
        print_val(*slot);
        printf(" }");
    }
    printf("\n");
    printf("%4d", disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code))); 
}
//Use bool, size_t, uint8_t
#endif


interpreted_result interpret(const char *source) {
    /* return run(); */
    Chunk chunk;
    initChunk(&chunk);

    if(!compile(source, &chunk)) {
        freeChunk(&chunk);
        return INTERPRET_COMPILE_ERROR;
    }

    vm.chunk = &chunk;
    vm.ip = vm.chunk->code;
    debug();
    interpreted_result result = run();
    freeChunk(&chunk);
    return result;
}

