#ifndef clox_common_h
#define clox_common_h

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <limits.h>

/* #define DEBUG_PRINT_CODE */
/* #define DEBUG_TRACE_EXECUTION */

#define UINT8_COUNT (UINT8_MAX + 1)

/* #ifdef DEBUG_TRACE_EXECUTION */
/* int debug() { */
/*     printf("       "); */
/*     for (Val *slot = vm.stack; slot < vm.stack_top; slot++) { */
/*         printf("{ "); */
/*         print_val(*slot); */
/*         printf(" }"); */
/*     } */
/*     printf("\n"); */
/*     return disassembleInstruction(vm.chunk, (int)(vm.ip - vm.chunk->code)); */ 
/* } */
/* int x = debug(); */
/* print(x); */
/* //Use bool, size_t, uint8_t */
/* #endif */
#endif
