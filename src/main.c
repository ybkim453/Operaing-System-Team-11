#include "command.h"
#include "parser.h"
#include "vfs_core.h"

#include <stdio.h>
#include <string.h>

char* file_content_store[256];

int main(void) {
    char cmd[256];

    VFS*    vfs   = vfs_init();
    Stack*  stack = stack_init();
    UserDB* users = users_init();

    login(users, vfs);
    users_save(users);

    while (1) {
        print_prompt(vfs, stack, users);

        if (!fgets(cmd, sizeof(cmd), stdin)) {
            clearerr(stdin);
            continue;
        }

        cmd[strcspn(cmd, "\n")] = '\0';

        if (strlen(cmd) == 0) continue;
        if (strcmp(cmd, "exit") == 0) break;

        ParseAndExecute(vfs, stack, users, cmd);
    }

    stack_free(stack);
    vfs_free(vfs);
    users_free(users);

    return 0;
}
