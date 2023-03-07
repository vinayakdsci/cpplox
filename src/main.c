#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"


int main () {
    init_vm();
    Chunk chunk;
    initChunk(&chunk);
    writeChunk(&chunk, OP_CONSTANT, 123);

    int constant = add_const(&chunk, 1.5);

    writeChunk(&chunk, constant, 123);
    writeChunk(&chunk, OP_NEGATE, 123);
    writeChunk(&chunk, OP_RETURN, 123);
    
    //Dissasemble the chunk!
    disassembleChunk(&chunk, "test chunk");

    interpret(&chunk);
    free_vm();
    freeChunk(&chunk);
    return 0;
}
