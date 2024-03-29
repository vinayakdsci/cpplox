#ifndef clox_object_h
#define clox_object_h

#include "common.h"
#include "value.h"
#include "chunk.h"

#define OBJ_TYPE(value)      (AS_OBJ(value)->type)
#define IS_NATIVE(value)     is_object_type(value, OBJ_NATIVE)
#define IS_STRING(str)       is_object_type(str, OBJ_STRING)
#define AS_STRING(value)     ((obj_string*)AS_OBJ(value))
#define AS_FUNCTION(value)   ((obj_function*)AS_OBJ(value))
#define AS_CSTRING(value)    (((obj_string*)AS_OBJ(value))->chars)
#define AS_CLOSURE(value)           ((obj_closure*) AS_OBJ(value))
#define IS_FUNCTION(value)   is_object_type(value, OBJ_FUNCTION)
#define IS_CLOSURE(value)    is_object_type(value, OBJ_CLOSURE);

/* define the object held in the Obj */
typedef enum {
    OBJ_CLOSURE,
    OBJ_FUNCTION,
    OBJ_NATIVE,
    OBJ_STRING,
    OBJ_UPVALUE
} object_type;

struct Obj {
    object_type type;
    /*My own small garbage collector*/
    struct Obj *next;
};

typedef struct obj_upvalue{
    Obj obj;
    Val *location;
    Val closed;
    struct obj_upvalue *next;
} obj_upvalue;

/* first class construct */
typedef struct {
    Obj obj;
    int arity; //arg count of the function
    int up_count;
    Chunk chunk;
    obj_string *name;
} obj_function;

typedef Val (*native)(int arg_count, Val *args);
typedef struct {
    Obj obj;
    native function;
} obj_native;

struct obj_string {
    Obj obj;
    int length;
    char *chars;  //heap allocation for the chars
    /* cache the string name for a variable */
    uint32_t hash;
};

typedef struct {
    Obj obj;
    obj_function *function;
    obj_upvalue **upvalues;
    int upvalue_count;
} obj_closure;

obj_closure *new_closure(obj_function *function);
obj_function *new_function();
obj_native *new_native(native function);
obj_upvalue *new_upvalue(Val *slot);
/* ensure cast safety */

static inline bool is_object_type(Val value, object_type type) {
    return IS_OBJ(value) && AS_OBJ(value)->type == type;
}


obj_string *take_string(char *chars, int length);
obj_string *copy_string(const char *chars, int length);
void print_object(Val value);

/* This is essentially the definition of a string.
 * Because it also has the type Obj, it must share metadata that all
 * Obj objects share.
 * NB: first field MUST be held by the Obj object
 * as c packs the struct members in the order they are specified.
 * Because the first field is perfectly aligned in the struct,
 * it allows to safely cast the child into the parent pointer,
 * like casting obj_string* to Obj*.
 * C mandate is pretty neat, innit?
 * */

#endif
