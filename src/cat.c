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

void command_cat(VFS* vfs, UserDB* users, int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: cat [-n] <filename> or cat > <filename>\n");
        return;
    }

    int show_num  = 0;
    int is_write  = 0;
    const char* filename = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-n") == 0) {
            show_num = 1;
        } else if (strcmp(argv[i], ">") == 0) {
            is_write = 1;
            if (i + 1 < argc) filename = argv[++i];
        } else if (argv[i][0] == '>' && argv[i][1] != '\0') {
            is_write = 1;
            filename = argv[i] + 1;
        } else {
            filename = argv[i];
        }
    }

    if (!filename) {
        printf("cat: missing filename\n");
        return;
    }

    VNode* orig = vfs->current;

    const char* base = nav_to_dir(vfs, filename);
    if (!base) {
        printf("cat: %s: No such file or directory\n", filename);
        vfs->current = orig;
        return;
    }

    if (is_write) {
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
                fprintf(stderr, "cat: no storage slot available\n");
                vfs->current->child = file->sibling;
                free(file);
                vfs->current = orig;
                return;
            }
            file->contentIndex = idx;
        }

        char   buf[4096] = {0};
        char   line[256];
        size_t nbytes = 0;
        while (fgets(line, sizeof(line), stdin)) {
            strncat(buf, line, sizeof(buf) - strlen(buf) - 1);
            nbytes += strlen(line);
        }
        if (feof(stdin)) clearerr(stdin);

        if (file->contentIndex != -1 && file_content_store[file->contentIndex])
            free(file_content_store[file->contentIndex]);

        file_content_store[file->contentIndex] = strdup(buf);
        file->SIZE = (int)nbytes;
        update_node_time(file);
        vfs->current = orig;
        return;
    }

    VNode* file = get_file(vfs->current, base);
    if (!file) {
        printf("cat: %s: No such file\n", filename);
        vfs->current = orig;
        return;
    }

    if (IsPermission(file, 'r', users->current->UID) != 0) {
        printf("cat: %s: Permission denied\n", file->name);
        vfs->current = orig;
        return;
    }

    if (file->contentIndex < 0 || file->contentIndex >= 256
            || !file_content_store[file->contentIndex]) {
        printf("\n");
        vfs->current = orig;
        return;
    }

    if (show_num) {
        char* dup = strdup(file_content_store[file->contentIndex]);
        char* sp;
        char* ln = strtok_r(dup, "\n", &sp);
        int   num = 1;
        while (ln) {
            printf("%4d  %s\n", num++, ln);
            ln = strtok_r(NULL, "\n", &sp);
        }
        free(dup);
    } else {
        printf("%s", file_content_store[file->contentIndex]);
    }

    vfs->current = orig;
}
