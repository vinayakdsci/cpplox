#ifndef clox_common_h
#define clox_common_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef DEBUG_TRACE_EXECUTION
    printf("       ");
    for (Val *slot = vm.stack; slot < vm.stack_top; slot++) {
        printf("{ ");
        print_val(*slot);
        printf(" }");
    }
    printf("\n");
    disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code));
//Use bool, size_t, uint8_t
#endif
#endif
