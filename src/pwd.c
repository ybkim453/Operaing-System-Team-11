#include <stdio.h>
#include <string.h>
#include "command.h"

int command_pwd(VFS* vfs, Stack* stack, const char* opt) {
    (void)opt;
    print_current_path(vfs, stack);
    return 0;
}

void print_current_path(VFS* vfs, Stack* stack) {
    VNode* cur = vfs->current;

    if (cur == vfs->root) {
        puts("/");
        return;
    }

    while (cur && cur->parent) {
        stack_push(stack, cur->name);
        cur = cur->parent;
    }

    while (!stack_is_empty(stack))
        printf("/%s", stack_pop(stack));

    puts("");
}
