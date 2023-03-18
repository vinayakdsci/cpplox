#ifndef clox_compiler_h
#define clox_compiler_h

#include "vm.h"
#include "object.h"
#include "scanner.h"

obj_function *compile(const char *source);

void special_error(const char *msg);
/* void error_at(token *tok, const char *message); */





#endif
