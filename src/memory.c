#include <stdlib.h>
#include "memory.h"

void* reallocate(void* pointer, size_t oldSize, size_t newSize){
    if(newSize == 0) {
        free(pointer);
        return NULL;
    }
    //Free the memory allocation when the size reduces to 0
    
    void* result = realloc(pointer, newSize); //Works like malloc when oldSize = 0
    if(result == NULL) exit(1); //Memory overflow
    return result;
}
