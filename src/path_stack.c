#include "path_stack.h"
#include <stdlib.h>
#include <string.h>

void stack_push(Stack* s, const char* str) {
    SNode* nd = (SNode*)malloc(sizeof(SNode));
    if (!nd) return;
    strncpy(nd->name, str, MAXN - 1);
    nd->name[MAXN - 1] = '\0';
    nd->linknode = s->TopNode;
    s->TopNode   = nd;
}

char* stack_pop(Stack* s) {
    static char buf[MAXN];
    if (!s->TopNode) return "";
    SNode* nd  = s->TopNode;
    s->TopNode = nd->linknode;
    strncpy(buf, nd->name, MAXN - 1);
    buf[MAXN - 1] = '\0';
    free(nd);
    return buf;
}

int stack_is_empty(const Stack* s) {
    return s->TopNode == NULL;
}
