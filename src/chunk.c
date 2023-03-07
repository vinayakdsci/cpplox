#include <stdlib.h>
#include "chunk.h"
#include "memory.h"
/* #include "value.h" */
/* #include <stdio.h> */

void initChunk(Chunk* chunk) {
    //Initialise chunk
    chunk->count = 0;
    chunk->capacity = 0;
    chunk->code = NULL;
    chunk->lines = NULL;
    init_val_array(&chunk->constants); //Initialise constants along woth the chunk
}

void freeChunk(Chunk* chunk) {
    FREE_ARRAY(uint8_t, chunk->code, chunk->capacity);
    FREE_ARRAY(int, chunk->lines, chunk->capacity);
    free_val_array(&chunk->constants); //Free constants with the chunk
    initChunk(chunk); //Why? -> Zero out all the fields of the chunk, to create a clean state
}

void writeChunk (Chunk* chunk, uint8_t byte, int line){ 
    if(chunk->capacity < chunk->count + 1) {   //count has exceeded capacity
        int oldCapacity = chunk->capacity;
        chunk->capacity = GROW_CAPACITY(oldCapacity);

        chunk->code = GROW_ARRAY(uint8_t, chunk->code, oldCapacity, chunk->capacity);
        chunk->lines = GROW_ARRAY(int, chunk->lines, oldCapacity, chunk->capacity);
    } // This case will be encountered the very first time when the initChuck() func creates a new raw chunk
    chunk->code[chunk->count] = byte;
    chunk->lines[chunk->count] = line;
    chunk->count++;
}

int add_const(Chunk *chunk, Val value) {
    write_val_array(&chunk->constants, value);
    return chunk->constants.count - 1;

}

//The dynamic array of codes start as completely empty
