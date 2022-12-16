//Chunks are essentially Sequences of bytecode
#ifndef clox_chunk_h
#define clox_chunk_h


#include "common.h"
//Define opcode -> operation code
//return the kind of opertion that the interpeter is dealing with -> add, subtract etc.
typedef enum {
    OP_RETURN,
} OpCode;

typedef struct {
    int count; //The number of allocated entries of the memory allocated array that are actually in use
    int capacity; //The capacity of the array allocated
    uint8_t *code; //The data code stored along with the bytecode chunk
} Chunk; //A code is a chunk pf size one byte;

void initChunk (Chunk* chunk); //Define in the header file
void freeChunk(Chunk* chunk); //Free the chunk
void writeChunk(Chunk *chunk, uint8_t byte); //append a byte to the chunk

// When the value of count is less than capacity this means that there is remaining space in the array
#endif




//What of there is no more capapcity in the new array?
//Then the following operation is done , byt he reallocate() function
// -> Allocate a new array of the required capacity -> Copy the contents of the old array to the new array
// -> Store the new capacity -> delete the old array
// -> Delete the old array -> update code to point to the new array
// -> Store the new element -> update count
