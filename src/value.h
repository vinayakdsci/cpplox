#ifndef clox_value_h
#define clox_value_h


#include "common.h"

//Support on double precision values in the interpreter
typedef double Val;


/*
 * Constant pool is an array of values, and should be stored as such in the compiled class
 * This is the approach followed by most VMs. JVM, for example.
*/

typedef struct {
    int capacity;
    int count;
    Val *values;
} val_array;

/* 
 * As in Chunk, three functios are required to handle a val_array.
 * Defined below
*/

void init_val_array(val_array *array);
void write_val_array(val_array *array, Val value);
void free_val_array(val_array *array);
void print_val(Val value);
#endif

