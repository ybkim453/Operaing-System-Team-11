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

void command_diff(VFS* vfs, UserDB* users, int argc, char** argv) {
    if (argc < 3) {
        printf("Usage: diff <file1> <file2>\n");
        return;
    }

    const char* path1 = argv[1];
    const char* path2 = argv[2];

    VNode* orig = vfs->current;

    const char* base1 = nav_to_dir(vfs, path1);
    if (!base1) {
        printf("diff: %s: No such file or directory\n", path1);
        vfs->current = orig;
        return;
    }
    VNode* file1 = get_file(vfs->current, base1);
    if (!file1) {
        printf("diff: %s: No such file\n", path1);
        vfs->current = orig;
        return;
    }

    vfs->current = orig;

    const char* base2 = nav_to_dir(vfs, path2);
    if (!base2) {
        printf("diff: %s: No such file or directory\n", path2);
        vfs->current = orig;
        return;
    }
    VNode* file2 = get_file(vfs->current, base2);
    if (!file2) {
        printf("diff: %s: No such file\n", path2);
        vfs->current = orig;
        return;
    }

    vfs->current = orig;

    char* content1 = (file1->contentIndex >= 0 && file1->contentIndex < 256) ? file_content_store[file1->contentIndex] : NULL;
    char* content2 = (file2->contentIndex >= 0 && file2->contentIndex < 256) ? file_content_store[file2->contentIndex] : NULL;

    if (!content1 && !content2) return;

    char* dup1 = content1 ? strdup(content1) : strdup("");
    char* dup2 = content2 ? strdup(content2) : strdup("");

    char* sp1;
    char* sp2;
    char* ln1 = strtok_r(dup1, "\n", &sp1);
    char* ln2 = strtok_r(dup2, "\n", &sp2);

    int line_num = 1;

    while (ln1 || ln2) {
        if (ln1 && ln2) {
            if (strcmp(ln1, ln2) != 0) {
                printf("Line %d differ\n", line_num);
                printf("< %s\n", ln1);
                printf("> %s\n", ln2);
            }
        } else if (ln1 && !ln2) {
            printf("Line %d differ\n", line_num);
            printf("< %s\n", ln1);
            printf("> [EOF]\n");
        } else if (!ln1 && ln2) {
            printf("Line %d differ\n", line_num);
            printf("< [EOF]\n");
            printf("> %s\n", ln2);
        }

        ln1 = strtok_r(NULL, "\n", &sp1);
        ln2 = strtok_r(NULL, "\n", &sp2);
        line_num++;
    }

    free(dup1);
    free(dup2);
    vfs->current = orig;
}
