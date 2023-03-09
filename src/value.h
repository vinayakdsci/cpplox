#ifndef clox_value_h
#define clox_value_h


#include "common.h"

/* Two main challenges face me -> 1. To differentiate between values of 
 * different types, as you can;t multiply a true by 3.
 * Also, 3 si different from 4. How can the compiler distinguish this in a 
 * dynamically typed language like this?
 * Best solution -> use `tagged union`
 * Create tags for the data types we support, and defin them in enums successively
 * Following are data types with in-built support
 * */

typedef enum {
    VAL_BOOL,
    VAL_NIL,
    VAL_NUMBER,
} value_type;

/* Here is the tagged-union!
 * A union allowws reuse of bits, allowing to optimize the language in terms of space.
 * HOWEVER, it is unsafe to use! it is easy to access the same bit many
 * times, and reinterpret them through different pathways. Also, it provides
 * low level memory access, so be careful and don't mess things up..
 * */
typedef struct{
    value_type type;
    union {
        bool boolean;
        double number;
    } as;   // Q/A: why did I use the name `as`?
} Val;

/* it is a MUST to use the correct type for the data type in the AS_ macro 
 * HIGHLY UNSAFE otherwise.
 * following code ensures this
 * */
#define IS_BOOL(value)    ((value).type == VAL_BOOL)
#define IS_NIL(value)     ((value).type == VAL_NIL)
#define IS_NUMBER(value)  ((value).type == VAL_NUMBER)

/* unpack the union */
#define AS_BOOL(value)     ((value).as.boolean)
#define AS_NUMBER(value)   ((value).as.number)
/* convert C's static types to dynamic types in cpplox */
#define BOOL_VAL(value)    ((Val){VAL_BOOL,   {.boolean = value}})
#define NIL_VAL            ((Val){VAL_NIL,    {.number = 0}})
#define NUMBER_VAL(value)  ((Val){VAL_NUMBER, {.number = value}})



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

