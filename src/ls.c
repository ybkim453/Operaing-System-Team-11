#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "command.h"

#define LS_MAX 128

typedef struct {
    VNode*      node;
    const char* label;
    int         owned;
} Entry;

static void fmt_perm(int p[9]) {
    const char* sym = "rwx";
    for (int i = 0; i < 9; i++)
        putchar(p[i] ? sym[i % 3] : '-');
}

static void print_detail(VNode* n, const char* lbl, UserDB* users) {
    putchar(n->type);
    fmt_perm(n->permission);
    printf(" %-10s %-10s %5d %02d-%02d %02d:%02d %s\n",
        GetNameByUID(users, n->UID),
        GetNameByGID(users, n->GID),
        n->SIZE,
        n->month, n->day, n->hour, n->minute,
        lbl ? lbl : n->name);
}

static void print_short(VNode* n, const char* lbl) {
    const char* name = lbl ? lbl : n->name;
    printf("%s%s", name, strlen(name) < 8 ? "\t\t" : "\t");
}

static int entry_cmp(const void* a, const void* b) {
    const Entry* x = (const Entry*)a;
    const Entry* y = (const Entry*)b;
    return strcmp(x->label ? x->label : x->node->name,
                  y->label ? y->label : y->node->name);
}

void command_ls(VFS* vfs, UserDB* users, int argc, char** argv) {
    int all = 0, detail = 0;
    for (int i = 1; i < argc; i++) {
        if      (strcmp(argv[i], "-a") == 0)                 all = 1;
        else if (strcmp(argv[i], "-l") == 0)                 detail = 1;
        else if (strcmp(argv[i], "-al") == 0
              || strcmp(argv[i], "-la") == 0) all = detail = 1;
    }

    Entry entries[LS_MAX];
    int count = 0;

    if (all) {
        VNode* dot = (VNode*)malloc(sizeof(VNode));
        memcpy(dot, vfs->current, sizeof(VNode));
        entries[count++] = (Entry){ dot, ".", 1 };

        VNode* par = vfs->current->parent ? vfs->current->parent : vfs->root;
        VNode* dotdot = (VNode*)malloc(sizeof(VNode));
        memcpy(dotdot, par, sizeof(VNode));
        entries[count++] = (Entry){ dotdot, "..", 1 };
    }

    for (VNode* n = vfs->current->child; n && count < LS_MAX; n = n->sibling) {
        if (!all && n->name[0] == '.') continue;
        entries[count++] = (Entry){ n, NULL, 0 };
    }

    qsort(entries, count, sizeof(Entry), entry_cmp);

    int col = 0;
    for (int i = 0; i < count; i++) {
        if (detail) {
            print_detail(entries[i].node, entries[i].label, users);
        } else {
            print_short(entries[i].node, entries[i].label);
            if (++col % 5 == 0) putchar('\n');
        }
    }
    if (!detail && col % 5 != 0) putchar('\n');

    for (int i = 0; i < count; i++)
        if (entries[i].owned) free(entries[i].node);
}
