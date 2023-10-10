#include <stdlib.h>

#include "memory.h"
#include "vm.h"

void *reallocate(void *pointer, size_t oldSize, size_t newSize) {
  if (newSize == 0) {
    free(pointer);
    return NULL;
  }
  // Free the memory allocation when the size reduces to 0
  void *result = realloc(pointer, newSize); // Works like malloc when oldSize = 0
  
  if (result == NULL)
    exit(1); // Memory overflow
  return result;
}

static void free_ob(Obj *object) {
    /* each type of object must be handled differently */
    switch(object->type) {
        case OBJ_CLOSURE:  {
                               obj_closure *closure = (obj_closure*)object;
                               FREE_ARRAY(obj_upvalue*, closure->upvalues, closure->upvalue_count);
                               FREE(obj_closure, object);
                               break;
                           }
        case OBJ_FUNCTION: {
                               obj_function *function = (obj_function*)object;
                               freeChunk(&function->chunk);
                               FREE(obj_function, object);
                               break;
                           }
        case OBJ_NATIVE: {
                             FREE(obj_native, object);
                             break;
                         }
        case OBJ_STRING: {
                             obj_string *string = (obj_string*)object;
                             /* free the char array */
                             FREE_ARRAY(char, string->chars, string->length + 1);

                             /* free the string object itself and the 
                              * memory it has itself allocated 
                              * to some other resources
                              * */
                             FREE(obj_string, object);
                             break;
                         }
        case OBJ_UPVALUE:
                         FREE(obj_upvalue, object);
                         break;
    }
}

void free_objects() {
    /* simply traverse the linked list and free each node */
    Obj *object = vm.objects;
#ifdef DEBUG_TRACE_EXECUTION
    int counter = 0;
#endif
    while(object != NULL) {
        Obj *next = object->next;
        free_ob(object);
        object = next;
#ifdef DEBUG_TRACE_EXECUTION
        counter++;
#endif
    }
#ifdef DEBUG_TRACE_EXECUTION
    if(counter > 0)
        printf("\nfreed %d allocated objects before exiting\n", counter);
#endif
}
