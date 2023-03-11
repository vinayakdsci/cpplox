#include <stdio.h>
#include <string.h>


#include "memory.h"
#include "table.h"
#include "value.h"
#include "object.h"
#include "vm.h"

#define ALLOCATE_OBJ(type, objtype) \
    (type *)allocate_object(sizeof(type), objtype)


/* allocate the OBJECT on the heap */
static Obj *allocate_object(size_t size, object_type type) {
    Obj *object = (Obj *)reallocate(NULL, 0, size);
    object->type = type;
    /* Update the GC list */
    object->next = vm.objects;
    vm.objects = object;
    return object;
}

/* allocate the string on the heap */
static obj_string *allocate_string(char *chars, int length, uint32_t hash) {
    obj_string *string = ALLOCATE_OBJ(obj_string, OBJ_STRING);
    string->length = length;
    string->chars = chars;
    string->hash = hash;
    /* whenever a new string is created, add it to the hash */
    /* push(OBJ_VAL(string)); */
    set_table(&vm.strings, string, NIL_VAL);
    /* pop(); */
    return string;
}

static uint32_t hash_function(const char *key, int length) {
    /* the FNV-1a hash function */
    uint32_t hash = 216613626u;
    for (int i = 0; i< length; ++i) {
        hash ^= (uint8_t)key[i];
        hash *= 16777619;
    }
    return hash;
}

/* claims ownership of the passed string */
obj_string *take_string(char *chars, int length) {

    uint32_t hash = hash_function(chars, length);
    obj_string *intern = table_find(&vm.strings, chars, length, hash);
    if(intern != NULL) {
        FREE_ARRAY(char, chars, length + 1);
        return  intern;
    }

    return allocate_string(chars, length, hash);
}

obj_string *copy_string(const char *chars, int length) {
    uint32_t hash = hash_function(chars, length);

    obj_string *intern = table_find(&vm.strings, chars, length, hash);
    if(intern != NULL) 
        return  intern;

    char *heap_char = ALLOCATE(char, length + 1);
    memcpy(heap_char, chars, length);
    heap_char[length] = '\0';

    return allocate_string(heap_char, length, hash);
}

void print_object(Val value) {
    switch (OBJ_TYPE(value)) {
        case OBJ_STRING:
            printf("%s", AS_CSTRING(value));
            break;
    }
}
