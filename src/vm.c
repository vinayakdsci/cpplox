#include <stdio.h>
#include <stdarg.h>



#include "vm.h"
#include "common.h"
#include "compiler.h"
#include "debug.h"

/* Global decl */
VM vm;

static void reset_stack() {
    vm.stack_top = vm.stack;    //set stack_top to the beginning of the array to indecate it is empty
}

/* a Variadic function */
static void runtime_error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);
    size_t instruction = vm.ip - vm.chunk->code - 1;
    int lines = vm.chunk->lines[instruction];
    fprintf(stderr, "line [%d] in script\n", lines);
    reset_stack();
}

/*
 *The Run function is the core of the VM, the heart that pumps blood to the body
 */

void init_vm() {
    reset_stack();
}


void free_vm() {

}

void push(Val value) {
    *vm.stack_top = value;
    vm.stack_top++;
}

Val pop() {
    vm.stack_top--;
    return *vm.stack_top;
}


static Val peek(int distance) {
    /* returns how far from the stack top to search. 
     * 0 is the top, -1 is the second down, and so on
     * */
    return vm.stack_top[-1 - distance];
}

static interpreted_result run (void) {
#define READ_BYTE() (*vm.ip++)

#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])

#define BIN_OP(v, op)  \
    do { \
        if(!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
            printf("peek() = %d , preek(1) = %d", IS_NUMBER(peek(0)), IS_NUMBER(peek(1))); \
            runtime_error("operands must be numbers."); \
            return INTERPRET_RUNTIME_ERROR; \
        } \
        double b = AS_NUMBER(pop()); \
        double a = AS_NUMBER(pop());  \
        push(v(a op b));  \
    } while(false)  //Execute only once



    for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
        printf("       ");
        for (Val *slot = vm.stack; slot < vm.stack_top; slot++) {
            printf("[ ");
            print_val(*slot);
            printf(" ]");
        }
        printf("\n");
        disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code)); 
#endif
        uint8_t instruction;
        switch(instruction = READ_BYTE()) {
            case OP_CONSTANT: {
                                  Val constant = READ_CONSTANT();
                                  /* print_val(constant); */
                                  /* printf("inside op constant"); */
                                  push(constant);
                                  break;
                              }
            /* add types for nil, true, false */
            case OP_NIL:        push(NIL_VAL);            break;
            case OP_TRUE:       push(BOOL_VAL(true));     break;
            case OP_FALSE:      push(BOOL_VAL(false));    break;
            case OP_ADD:        BIN_OP(NUMBER_VAL, +);    break;
            case OP_SUBTRACT:   BIN_OP(NUMBER_VAL, -);    break;
            case OP_MULTIPLY:   BIN_OP(NUMBER_VAL, *);    break;
            case OP_DIVIDE:     BIN_OP(NUMBER_VAL, /);    break;
            case OP_NEGATE:  
                                /* First check if the element at the top
                                 * of the stack is a number.
                                 * if not, runtime error
                                 * else, keep going
                                 * */
                                if(!IS_NUMBER(peek(0))) {
                                    runtime_error("Operand must be a number.");
                                    return INTERPRET_RUNTIME_ERROR;
                                }
                                push(NUMBER_VAL(-AS_NUMBER(pop())));
                                break;
            case OP_RETURN:  
                                print_val(pop()); 
                                printf("\n");
                                return INTERPRET_OK; 

        }
    }
#undef READ_BYTE
#undef READ_CONSTANT
#undef BIN_OP
}

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
    /* debug(); */
    interpreted_result result = run();
    freeChunk(&chunk);
    return result;
}

