// Chunks are essentially Sequences of bytecode
#ifndef clox_chunk_h
#define clox_chunk_h

#include "common.h"
#include "value.h"

//Define opcode -> operation code
//return the kind of opertion that the interpeter is dealing with -> add, subtract etc.
typedef enum {
    
  OP_CONSTANT,
  OP_NIL,
  OP_TRUE,
  OP_FALSE,
  OP_POP,
  OP_GET_LOCAL,
  OP_SET_LOCAL,
  OP_GET_GLOBAL,
  OP_DEF_GLOBAL,
  OP_SET_GLOBAL,
  OP_GET_UPVALUE,
  OP_SET_UPVALUE,
  OP_GET_PROPERTY,
  OP_SET_PROPERTY,
  OP_GET_SUPER,
  OP_EQUAL,
  OP_GREATER,
  OP_LESS,
  OP_ADD,
  OP_SUBTRACT,
  OP_MULTIPLY,
  OP_DIVIDE,
  OP_NOT,
  OP_NEGATE,
  OP_PRINT,
  OP_JUMP,
  OP_JUMP_IF_FALSE,
  OP_LOOP,
  OP_CALL,
  OP_INVOKE,
  OP_SUPER_INVOKE,
  OP_CLOSURE,
  OP_CLOSE_UPVALUE,
  OP_RETURN,
  OP_CLASS,
  OP_INHERIT,
  OP_METHOD
} OpCode;

typedef struct {
    int count; //The number of allocated entries of the memory allocated array that are actually in use
    int capacity; //The capacity of the array allocated
    uint8_t *code; //The data code stored along with the bytecode chunk
    int *lines;
    val_array constants;
} Chunk; //A code is a chunk pf size one byte;

void initChunk (Chunk* chunk); //Define in the header file
void freeChunk(Chunk* chunk); //Free the chunk
void writeChunk(Chunk *chunk, uint8_t byte, int line); //append a byte to the chunk
int add_const(Chunk *chunk, Val value); //returns the count
// When the value of count is less than capacity this means that there is remaining space in the array
#endif




//What of there is no more capapcity in the new array?
//Then the following operation is done , byt he reallocate() function
// -> Allocate a new array of the required capacity -> Copy the contents of the old array to the new array
// -> Store the new capacity -> delete the old array
// -> Delete the old array -> update code to point to the new array
// -> Store the new element -> update count
