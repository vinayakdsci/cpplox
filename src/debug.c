#include "debug.h"
#include "value.h"
#include "object.h"

#include <stdio.h>

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
        printf("%-16s %4d  ", name, constant);
        print_val(chunk->constants.values[constant]);
        printf("\n");
        return offset+2;
}

static int byte_instruction(const char *inst, Chunk *chunk, int offset) {
        uint8_t slot = chunk->code[offset + 1];
        printf("%-16s %4d", inst, slot);
        return offset + 2;
}
static int jump_instruction(const char *name, int sign, Chunk *chunk, int offset){
        uint16_t jump = (uint16_t)(chunk->code[offset + 1] << 8);
        jump |= chunk->code[offset + 2];
        printf("%-16s %4d -> %d\n", name , offset, offset+3 + sign *jump);
        return offset + 3;
}

int disassembleInstruction(Chunk* chunk, int offset) {
        printf("%04d   ", offset);

        if(offset > 0 && chunk->lines[offset] == chunk->lines[offset - 1]) {
                printf("   |   ");
        }
        else {
                printf("%5d  ", chunk->lines[offset]);
        }

        uint8_t instruction = chunk->code[offset];
        switch(instruction) {
                case OP_RETURN:
                        return simpleInstruction("OP_RETURN", offset);
                case OP_CONSTANT:
                        return const_instruction("OP_CONSTANT", chunk, offset);
                case OP_NEGATE:
                        return simpleInstruction("OP_NEGATE", offset);
                case OP_PRINT:
                        return simpleInstruction("OP_PRINT", offset);
                case OP_GET_GLOBAL:
                        return const_instruction("OP_GET_GLOBAL", chunk, offset);
                case OP_SET_GLOBAL:
                        return const_instruction("OP_SET_GLOBAL", chunk, offset);
                case OP_POP:
                        return simpleInstruction("OP_POP", offset);
                case OP_DEF_GLOBAL:
                        return const_instruction("OP_DEF_GLOBAL", chunk, offset);
                case OP_CALL:
                        return byte_instruction("OP_CALL", chunk, offset);
                case OP_SET_UPVALUE:
                        return byte_instruction("OP_SET_UPVALUE", chunk, offset);
                case OP_GET_UPVALUE:
                        return byte_instruction("OP_GET_UPVALUE", chunk, offset);
                case OP_CLOSE_UPVALUE:
                        return simpleInstruction("OP_CLOSE_UPVALUE", offset);
                case OP_CLOSURE: {
                                         offset++;
                                         uint8_t constant = chunk->code[offset++];
                                         printf("%-16s %4d","OP_CLOSURE", constant);
                                         print_val(chunk->constants.values[constant]);
                                         printf("\n");
                                         obj_function *fn = AS_FUNCTION(chunk->constants.values[constant]);
                                         for (int x = 0; x < fn->up_count; x++) {
                                                 int lc = chunk->code[offset++];
                                                 int index = chunk->code[offset++];
                                                 printf("%04d  |         %s %d\n", offset - 2 , lc ? "local" : "upvalue", index);
                                         }
                                         return offset;
                                 }
                case OP_GET_LOCAL:
                        return byte_instruction("OP_GET_LOCAL", chunk, offset);
                case OP_SET_LOCAL:
                        return byte_instruction("OP_SET_LOCAL", chunk, offset);
                case OP_JUMP_IF_FALSE:
                        return jump_instruction("OP_JUMP_IF_FALSE", 1, chunk, offset);
                case OP_JUMP:
                        return jump_instruction("OP_JUMP", 1, chunk, offset);
                case OP_ADD:
                        return simpleInstruction("OP_ADD", offset);
                case OP_LOOP:
                        return jump_instruction("OP_LOOP", -1, chunk,  offset);
                case OP_SUBTRACT:
                        return simpleInstruction("OP_SUBTRACT", offset);
                case OP_MULTIPLY:
                        return simpleInstruction("OP_MULTIPLY", offset);
                case OP_DIVIDE:
                        return simpleInstruction("OP_DIVIDE", offset);
                case OP_GREATER:
                        return simpleInstruction("OP_GREATER", offset);
                case OP_LESS:
                        return simpleInstruction("OP_LESS", offset);
                case OP_EQUAL:
                        return simpleInstruction("OP_EQUAL", offset);
                case OP_NOT:
                        return simpleInstruction("OP_NOT", offset);
                case OP_NIL:
                        return simpleInstruction("OP_NIL", offset);
                case OP_TRUE:
                        return simpleInstruction("OP_TRUE", offset);
                case OP_FALSE:
                        return simpleInstruction("OP_FALSE", offset);
                default:
                        printf("Unknown opcode %d\n", instruction);
                        return offset + 1;
        }
}

