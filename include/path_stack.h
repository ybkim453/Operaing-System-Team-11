#ifndef PATH_STACK_H
#define PATH_STACK_H

#include "vfs.h"

#define STACK_MAX 128

void  stack_push(Stack* s, const char* str);
char* stack_pop(Stack* s);
int   stack_is_empty(const Stack* s);

#endif
