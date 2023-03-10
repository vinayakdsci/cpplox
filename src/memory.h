#ifndef clox_memory_h
#define clox_memory_h

#define ALLOCATE(type, length) \
    (type *)reallocate(NULL, 0, sizeof(type) * (length))

#define GROW_CAPACITY(capacity) \
    ((capacity) < 8 ? 8 : (capacity) * 2) //Amortized analysis -> When capacity is increase by a multiple of the original size, the process is much efficient

#define GROW_ARRAY(type, pointer, oldCount, newCount) \
    (type*)reallocate(pointer, sizeof(type)*(oldCount), sizeof(type)*(newCount)) //see the prototype of reallocate below

#define FREE_ARRAY(type, pointer, oldCount) \
    reallocate(pointer, sizeof(type)*(oldCount),0) //->newSize = 0

void* reallocate(void* pointer, size_t oldSize, size_t newSize); //return a void pointer that is type-casted

#endif
