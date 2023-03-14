#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <time.h>

#include "debug.h"
#include "object.h"
#include "memory.h"
#include "vm.h"
#include "common.h"
#include "compiler.h"

/* Global decl */
VM vm;

static void reset_stack() {
    vm.stack_top = vm.stack;    //set stack_top to the beginning of the array to indecate it is empty
    vm.frame_count = 0;
}

/* a Variadic function */
static void runtime_error(const char *format, ...) {
    va_list args;
    va_start(args, format);
    vfprintf(stderr, format, args);
    va_end(args);
    fputs("\n", stderr);
    /* call_frame *frame = &vm.frame[vm.frame_count - 1]; */
    /* size_t instruction = frame->ip - frame->function->chunk.code - 1; */
    /* int lines = frame->function->chunk.lines[instruction]; */
    /* fprintf(stderr, "line [%d] in script\n", lines); */

    for(int i = vm.frame_count - 1; i >= 0; i--) {
        call_frame *frame = &vm.frame[i];
        obj_function *function = frame->closure->function;
        size_t inst = frame->ip - function->chunk.code - 1;

        fprintf(stderr, "line [%d] in ", function->chunk.lines[inst]);
        if(function->name == NULL) {
            fprintf(stderr, "script\n");
        }
        else {
            fprintf(stderr, "%s()\n", function->name->chars);
        }
    }
    reset_stack();
}

static Val native_clock(int argcount, Val *args) {
    return NUMBER_VAL((double)clock() / CLOCKS_PER_SEC);
}

static void native_define(const char *name, native function) {
    push(OBJ_VAL(copy_string(name, (int)(strlen(name)))));
    push(OBJ_VAL(new_native(function)));
    set_table(&vm.globals, AS_STRING(vm.stack[0]), vm.stack[1]);
    pop();
    pop();
}

/*
 *The Run function is the core of the VM, the heart that pumps blood to the body
 */

void init_vm() {
    reset_stack();
    /*No objects on the heap at the moment*/
    vm.objects = NULL;
    init_table(&vm.globals);
    init_table(&vm.strings);
    native_define("clock", native_clock);
}


void free_vm() {
    free_table(&vm.globals);
    free_table(&vm.strings);
    free_objects();
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

static bool call(obj_closure *closure, int arg_count) {
    if(closure->function->arity != arg_count){
        runtime_error("expected %d args, but got %d", closure->function->arity,  arg_count);
        return false;
    }

    if(vm.frame_count == FRAMES_MAX){
        runtime_error("Stack overflow!");
        return false;
    }

    call_frame *frame = &vm.frame[vm.frame_count++];
    frame->closure = closure;
    frame->ip = closure->function->chunk.code;
    frame->slots = vm.stack_top - arg_count - 1;
    return true;
}

static bool call_val(Val value, int arg_count) {
    if(IS_OBJ(value)) {
        switch(OBJ_TYPE(value)) {
            case OBJ_CLOSURE:
                return call(AS_CLOSURE(value), arg_count);
            /* case OBJ_FUNCTION: */
            /*     return call(AS_FUNCTION(value), arg_count); */
            case OBJ_NATIVE: {
                                 native n = AS_NATIVE(value);
                                 Val result = n(arg_count, vm.stack_top - arg_count);
                                 vm.stack_top -= arg_count + 1;
                                 push(result);
                                 return true;
                             }
            default: break;
        }
    }
    runtime_error("can only call functions.");
    return false;
}

static bool is_false(Val value) {
    return IS_NIL(value) || (IS_BOOL(value) && !AS_BOOL(value));
}

static void concatenate() {
    obj_string *b = AS_STRING(pop());
    obj_string *a = AS_STRING(pop());

    /* calculate the new length */
    int new_length = a->length + b->length;
    char *new_chars = ALLOCATE(char, new_length + 1);
    memcpy(new_chars, a->chars, a->length);
    memcpy(new_chars + a->length, b->chars, b->length);
    new_chars[new_length] = '\0';

    obj_string *result = take_string(new_chars, new_length);
    push(OBJ_VAL(result));
}

static interpreted_result run (void) {
    call_frame *frame = &vm.frame[vm.frame_count - 1];
#define READ_BYTE() (*frame->ip++)
#define READ_CONSTANT() (frame->closure->function->chunk.constants.values[READ_BYTE()])
    /* read a single byte constant from the bytecode chunk */
#define READ_STRING()   AS_STRING(READ_CONSTANT())
#define READ_SHORT()   \
    (frame->ip += 2, (uint16_t)((frame->ip[-2] << 8) | frame->ip[-1]))
#define BIN_OP(v, op)  \
    do { \
        if(!IS_NUMBER(peek(0)) || !IS_NUMBER(peek(1))) { \
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
        disassembleInstruction(&frame->closure->function->chunk, (int)(frame->ip - frame->closure->function->chunk.code)); 
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
            case OP_DEF_GLOBAL: {
                                    /* pretty self-explanatory? 
                                     * do not check to see if the variable is already
                                     * defined, simply overwrite 
                                     * */
                                    obj_string *name = READ_STRING();
                                    set_table(&vm.globals, name, peek(0));
                                    pop();
                                    break;
                                }
            case OP_GET_GLOBAL: {
                                    obj_string *name = READ_STRING();
                                    Val value;
                                    if(!get_table(&vm.globals, name, &value)) {
                                        /* if you can't find the var, it's undefined */
                                        runtime_error("undefined variable '%s in get_glob'.", name->chars);
                                        return INTERPRET_RUNTIME_ERROR;
                                    }
                                    push(value);
                                    break;
                                }
            case OP_SET_GLOBAL: {
                                    /* lookup in the constant table */
                                    obj_string *name = READ_STRING();
                                    if(set_table(&vm.globals, name, peek(0))) {
                                        delete_table(&vm.globals, name);
                                        runtime_error("Undefined variable %s in set_glob", name->chars);
                                        return INTERPRET_RUNTIME_ERROR;
                                    }
                                    break;
                                }
            case OP_GET_LOCAL: {
                                   uint8_t slot = READ_BYTE();
                                   push(frame->slots[slot]);
                                   break;
                               }
            case OP_SET_LOCAL: {
                                   uint8_t slot = READ_BYTE();
                                   frame->slots[slot] = peek(0);
                                   break;
                               }
                               /* add types for nil, true, false */
            case OP_EQUAL: {
                               Val a  = pop();
                               Val b = pop();
                               push(BOOL_VAL(is_equal(a,b)));
                               break;
                           }
            case OP_GREATER:    BIN_OP(BOOL_VAL, >);    break;
            case OP_LESS:       BIN_OP(BOOL_VAL, <);    break;
            case OP_NIL:        push(NIL_VAL);            break;
            case OP_TRUE:       push(BOOL_VAL(true));     break;
            case OP_FALSE:      push(BOOL_VAL(false));    break;
            case OP_JUMP_IF_FALSE: {
                                       uint16_t offset = READ_SHORT();
                                       if(is_false(peek(0)))
                                           frame->ip += offset;
                                       break;
                                   }
            case OP_JUMP: {
                              uint16_t offset = READ_SHORT();
                              frame->ip += offset;
                              break;
                          }
            case OP_CALL: {
                              int arg_count = READ_BYTE();
                              if(!call_val(peek(arg_count), arg_count))
                                  return INTERPRET_RUNTIME_ERROR;
                              frame = &vm.frame[vm.frame_count - 1];                              
                              break;
			  }
            case OP_CLOSURE: {
                                 obj_function *function = AS_FUNCTION(READ_CONSTANT());
                                 obj_closure *closure = new_closure(function);
                                 push(OBJ_VAL(closure));
                                 break;
                             }
            case OP_ADD:       {
                                   /* check if string */
                                   if(IS_STRING(peek(0)) && IS_STRING(peek(1))) {
                                       concatenate();
                                   }
                                   else if(IS_NUMBER(peek(0)) && IS_NUMBER(peek(1))) {
                                       double a = AS_NUMBER(pop());
                                       double b = AS_NUMBER(pop());

                                       push(NUMBER_VAL(a + b));
                                   }
                                   else {
                                       runtime_error("Operands to '+' must be two numbers or two strings");
                                       return INTERPRET_RUNTIME_ERROR;
                                   }
                                   break;
                                }
            case OP_SUBTRACT:   BIN_OP(NUMBER_VAL, -);    break;
            case OP_MULTIPLY:   BIN_OP(NUMBER_VAL, *);    break;
            case OP_DIVIDE:     BIN_OP(NUMBER_VAL, /);    break;
            case OP_NOT:
                                push(BOOL_VAL(is_false(pop())));
                                break;
            case OP_LOOP:       {
                                    uint16_t offset = READ_SHORT();
                                    frame->ip -= offset;
                                    break;
                                }
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
            case OP_PRINT:      {print_val(pop()); break;}
            case OP_POP:        pop(); break;
            case OP_RETURN:  {
                                 //simply exit, as print has been intro'd
                                 Val result = pop();
                                 vm.frame_count--;
                                 if(vm.frame_count == 0) {
                                     pop();
                                     return INTERPRET_OK;
                                 }
                                 vm.stack_top = frame->slots;
                                 push(result);
                                 frame = &vm.frame[vm.frame_count - 1];
                                 break;
                             }
        }
    }
#undef READ_BYTE
#undef READ_CONSTANT
#undef READ_STRING
#undef READ_SHORT
#undef BIN_OP
}

interpreted_result interpret(const char *source) {
    /* return run(); */
    obj_function *function = compile(source);
    if(function == NULL) {
        return INTERPRET_COMPILE_ERROR;
    }
    push(OBJ_VAL(function));
    obj_closure *closure = new_closure(function);
    pop();
    push(OBJ_VAL(closure));
    /* call_frame *frame = &vm.frame[vm.frame_count++]; */
    /* frame->function = function; */
    /* frame->ip = function->chunk.code; */
    /* frame->slots = vm.stack; */

    
    /* top level function call */
    call(closure, 0);

    return run();
}

