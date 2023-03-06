#include "common.h"
#include "chunk.h"
#include "debug.h"
int main (int argc, char *argv[]) {
    Chunk chunk;
    initChunk(&chunk);
    int constant = add_const(&chunk, 1.5);
    writeChunk(&chunk, OP_RETURN);
    writeChunk(&chunk, OP_CONSTANT);
    //Dissasemble the chunk!
    disassembleChunk(&chunk, "test chunk");
    freeChunk(&chunk);
    return 0;
}
