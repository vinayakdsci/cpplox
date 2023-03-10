#include <stdio.h>

#include "memory.h"
#include "value.h"


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
    }
}


//Finally, release memory
void free_val_array(val_array *array){
    FREE_ARRAY(Val, array->values, array->capacity);
    init_val_array(array);
}
