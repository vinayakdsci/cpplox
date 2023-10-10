#include <stdio.h>
#include <string.h>

#include "object.h"
#include "memory.h"
/* #include "value.h" */

void init_val_array(val_array *array) {
    array->values = NULL;
    array->capacity = 0;
    array->count = 0;
}

void write_val_array(val_array *array, Val value){
    if(array->capacity < array->count + 1) {
        int old_capacity = array->capacity;
        array->capacity = GROW_CAPACITY(old_capacity);
        array->values = GROW_ARRAY(Val, array->values, old_capacity, array->capacity);
    }
    array->values[array->count++] = value;
}

void print_val(Val value){
    switch(value.type) {
        case VAL_BOOL :
            printf(AS_BOOL(value) ? "true" : "false");
            break;
        case VAL_NIL: printf("nil"); break;
        case VAL_NUMBER: printf("%g", AS_NUMBER(value)); break;
        case VAL_OBJ: print_object(value); break;
    }
}

bool is_equal(Val a, Val b) {
    if(a.type != b.type) return false;

    switch(a.type) {
        case VAL_BOOL:   return AS_BOOL(a) == AS_BOOL(b);
        case VAL_NIL:    return true;
        case VAL_NUMBER: return AS_NUMBER(a) == AS_NUMBER(b);
                         /* handle equality of strings */
        case VAL_OBJ:   
                         /* print_val(a); */
                         /* print_val(b); */
                         return AS_OBJ(a) == AS_OBJ(b);
        default: return false;
    }
}


//Finally, release memory
void free_val_array(val_array *array){
    FREE_ARRAY(Val, array->values, array->capacity);
    init_val_array(array);
}
