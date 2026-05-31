#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "command.h"
#include "vfs.h"

extern char* file_content_store[256];

void command_head(VFS* vfs, UserDB* users, int argc, char** argv) {
    int lines = 10;
    const char* filename = NULL;

    if (argc < 2) {
        printf("Usage: head [-n lines] <filename>\n");
        return;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0) {
            if (i + 1 >= argc) {
                printf("head: option requires an argument -- n\n");
                return;
            }
            lines = atoi(argv[++i]);
        } else {
            filename = argv[i];
        }
    }

    if (!filename) {
        printf("head: missing filename\n");
        return;
    }

    if (lines <= 0) {
        printf("head: invalid number of lines\n");
        return;
    }

    VNode* file = ExistDir(vfs, filename, 'f');

    if (!file || file->contentIndex < 0 || file->contentIndex >= 256
            || !file_content_store[file->contentIndex]) {
        printf("head: cannot open file: %s\n", filename);
        return;
    }

    if (IsPermission(file, 'r', users->current->UID) != 0) {
        printf("head: %s: Permission denied\n", file->name);
        return;
    }

    char buf[4096];
    strncpy(buf, file_content_store[file->contentIndex], sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char* sp;
    char* ln = strtok_r(buf, "\n", &sp);
    int cnt = 0;

    while (ln && cnt < lines) {
        printf("%s\n", ln);
        ln = strtok_r(NULL, "\n", &sp);
        cnt++;
    }
}
