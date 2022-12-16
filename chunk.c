#include <stdlib.h>
#include "chunk.h"
#include "memory.h"

void initChunk(Chunk* chunk) {
    //Initialise chunk
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;
}

void writeChunk (Chunk *chunk, uint8_t byte){ 
    if(chunk->capacity < chunk->count + 1) {   //count has exceeded capacity
        int oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);

        chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
    } // This case will be encountered the very first time when the initChuck() func creates a new raw chunk
}

void freeChunk(Chunk* chunk) {
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    initChunk(chunk); //Why? -> Zero out all the fields of the chunk, to create a clean state
}
//The dynamic array of codes start as completely empty
