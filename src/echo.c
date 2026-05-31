#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "command.h"
#include "vfs.h"

extern char* file_content_store[256];

static VNode* get_file(VNode* cur, const char* name) {
    for (VNode* n = cur->child; n; n = n->sibling)
        if (strcmp(n->name, name) == 0 && n->type == 'f')
            return n;
    return NULL;
}

static const char* nav_to_dir(VFS* vfs, const char* path) {
    if (!strchr(path, '/')) return path;

    static char dirpart[MAXD];
    static char base[MAXN];

    strncpy(dirpart, path, MAXD - 1);
    dirpart[MAXD - 1] = '\0';

    char* slash = strrchr(dirpart, '/');
    strncpy(base, slash + 1, MAXN - 1);
    base[MAXN - 1] = '\0';
    *slash = '\0';

    if (dirpart[0] == '\0')
        vfs->current = vfs->root;
    else if (MovePath(vfs, dirpart) != 0)
        return NULL;

    return base;
}

void command_echo(VFS* vfs, UserDB* users, int argc, char** argv) {
    int newline = 1;
    int is_write = 0;
    int is_append = 0;
    const char* filename = NULL;

    char buf[4096] = {0};
    int first = 1;

    for (int i = 1; i < argc; i++) {
        if (i == 1 && strcmp(argv[i], "-n") == 0) {
            newline = 0;
        } else if (strcmp(argv[i], ">") == 0) {
            is_write = 1;
            is_append = 0;
            if (i + 1 < argc) filename = argv[++i];
            break;
        } else if (strcmp(argv[i], ">>") == 0) {
            is_write = 1;
            is_append = 1;
            if (i + 1 < argc) filename = argv[++i];
            break;
        } else if (argv[i][0] == '>' && argv[i][1] != '\0') {
            is_write = 1;

            if (argv[i][1] == '>') {
                is_append = 1;
                filename = argv[i] + 2;
            } else {
                is_append = 0;
                filename = argv[i] + 1;
            }
            break;
        } else {
            if (!first)
                strncat(buf, " ", sizeof(buf) - strlen(buf) - 1);

            strncat(buf, argv[i], sizeof(buf) - strlen(buf) - 1);
            first = 0;
        }
    }

    if (newline)
        strncat(buf, "\n", sizeof(buf) - strlen(buf) - 1);

    if (!is_write) {
        printf("%s", buf);
        return;
    }

    if (!filename) {
        printf("echo: missing filename\n");
        return;
    }

    VNode* orig = vfs->current;

    const char* base = nav_to_dir(vfs, filename);
    if (!base) {
        printf("echo: %s: No such file or directory\n", filename);
        vfs->current = orig;
        return;
    }

    VNode* file = get_file(vfs->current, base);
    if (!file) {
        file = (VNode*)calloc(1, sizeof(VNode));
        strncpy(file->name, base, MAXN - 1);
        file->name[MAXN - 1] = '\0';
        file->type   = 'f';
        file->parent = vfs->current;
        for (int j = 0; j < 9; j++) file->permission[j] = 1;
        file->sibling       = vfs->current->child;
        vfs->current->child = file;

        int idx = -1;
        for (int i = 0; i < 256; i++) {
            if (!file_content_store[i]) { idx = i; break; }
        }
        if (idx == -1) {
            fprintf(stderr, "echo: no storage slot available\n");
            vfs->current->child = file->sibling;
            free(file);
            vfs->current = orig;
            return;
        }
        file->contentIndex = idx;
    } else {
        if (IsPermission(file, 'w', users->current->UID) != 0) {
            printf("echo: %s: Permission denied\n", file->name);
            vfs->current = orig;
            return;
        }
    }

    if (is_append && file_content_store[file->contentIndex]) {
        size_t old_len = strlen(file_content_store[file->contentIndex]);
        size_t add_len = strlen(buf);

        char* new_content = (char*)malloc(old_len + add_len + 1);
        if (!new_content) {
            fprintf(stderr, "echo: memory allocation failed\n");
            vfs->current = orig;
            return;
        }

        strcpy(new_content, file_content_store[file->contentIndex]);
        strcat(new_content, buf);

        free(file_content_store[file->contentIndex]);
        file_content_store[file->contentIndex] = new_content;
    } else {
        if (file_content_store[file->contentIndex])
            free(file_content_store[file->contentIndex]);

        file_content_store[file->contentIndex] = strdup(buf);
    }

    file->SIZE = (int)strlen(file_content_store[file->contentIndex]);
    update_node_time(file);

    vfs->current = orig;
}