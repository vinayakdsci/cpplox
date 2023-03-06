#include <stdio.h>
#include "debug.h"
#include "value.h"


void disassembleChunk(Chunk* chunk, const char *name){
    printf("--------%s---------\n", name);
    for(int offset = 0; offset < chunk->count;){
        offset = disassembleInstruction(chunk, offset);
    }
}

static int simpleInstruction(const char* name, int offset) {
    printf("%s\n", name);
    return offset + 1;
}

static int const_instruction(const char *name, Chunk *chunk,
                             int offset)
{
    uint8_t constant = chunk->code[offset+1];
    printf("%-16s %4d ", name, constant);
    print_val(chunk->constants.values[constant]);
    printf("\n");
    return offset+2;
}

int disassembleInstruction(Chunk* chunk, int offset) {
    printf("%04d   ", offset);

    uint8_t instruction = chunk->code[offset];
    switch(instruction) {
        case OP_RETURN:
            return simpleInstruction("OP_RETURN", offset);
        case OP_CONSTANT:
            return const_instruction("OP_CONSTANT", chunk, offset);
        default:
            printf("Unknown opcode %d\n", instruction);
            return offset + 1;
    }
}

