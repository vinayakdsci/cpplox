#include "common.h"
#include "chunk.h"
#include "debug.h"
int main (int argc, char *argv[]) {
    Chunk chunk;
    initChunk(&chunk);
    writeChunk(&chunk, OP_CONSTANT, 123);
    int constant = add_const(&chunk, 1.5);
    writeChunk(&chunk, constant, 123);
    writeChunk(&chunk, OP_RETURN, 123);
    //Dissasemble the chunk!
    disassembleChunk(&chunk, "test chunk");
    freeChunk(&chunk);
    return 0;
}
