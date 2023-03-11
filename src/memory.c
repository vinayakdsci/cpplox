#include <stdlib.h>

#include "memory.h"
#include "vm.h"

void *reallocate(void *pointer, size_t oldSize, size_t newSize) {
  if (newSize == 0) {
    free(pointer);
    return NULL;
  }
  // Free the memory allocation when the size reduces to 0

  void *result = realloc(pointer, newSize); // Works like malloc when oldSize =
					    // 0
  if (result == NULL)
    exit(1); // Memory overflow
  return result;
}

static void free_ob(Obj *object) {
    /* each type of object must be handled differently */
    switch(object->type) {
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
    }
}

void free_objects() {
    /* simply traverse the linked list and free each node */
    Obj *object = vm.objects;
    int counter = 0;
    while(object != NULL) {
        Obj *next = object->next;
        free_ob(object);
        object = next;
        counter++;
    }
    if(counter > 0)
        printf("Freed %d objects from the heap\n", counter);
}
