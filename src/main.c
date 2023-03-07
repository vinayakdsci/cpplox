#include <stdio.h>
#include <stdlib.h>
#include <string.h>



#include "common.h"
#include "chunk.h"
#include "debug.h"
#include "vm.h"


static void repl() {
    char line[1024];
    for (;;) {
        printf("CCPLOX> ");
        if(fgets(line, sizeof(line), stdin)){
            printf("\n");
            break;
        }
        interpret(line);
    }
}


static void run_file(const char *path){
    char *source = read_file(path);
    interpreted_result res = interpret(source);
    free(source);

    if(res == INTERPRET_COMPILER_ERROR){
        printf("COMPILE ERROR\n");
        exit(65);
    }
    if(res == INTERPRET_RUNTIME_ERROR) {
        printf("RUNTIME ERROR\n");
        exit(70);
    }
}

static char *read_file(const char *path) {
    FILE *file = fopem(path, "rb");
    //Manual check for errors, no exceptions
    if(file == NULL) {
        frpintf(stderr, "Coudl not open \"%s\".\n", path);
        exit(74);
    }

    fseek(file, 0L, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);
    char *buffer = (char*)malloc(file_size + 1);
    /* Manually check for memory errors */
    if(buffer == NULL) {
        fprintf(stderr, "No enough memeory to read buffer at \"%s\".\n",path);
        exit(74);
    }

    size_t byte_len = fread(buffer, sizeof(char), file_size, file);
    if(byte_len < file_size) {
        fprintf(stderr, "File could not  be read.\n");
    }

    /* Terminate the buffer */
    buffer[byte_len] = '\0';  
    fclose(file);
    return buffer;
}


int main (int argc, const char *argv[]) {
    init_vm();
    //REPL
    if(argc == 1) {
        repl();
    }
    else if (argc == 2) {
        run_file(argv[1]);
    }
    else {
        fprintf(stderr, "USAGE: ./pplox [path]\n");
        exit(64);
    }
    
    free_vm();
    freeChunk(&chunk);
    return 0;
}
