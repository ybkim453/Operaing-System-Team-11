#include "command.h"
#include <string.h>
#include <stdio.h>

int command_cd(VFS* vfs, UserDB* users, const char* arg) {
    if (!arg) {
        vfs->current = vfs->root;
        UpdateUserDir(users, vfs);
        return 0;
    }

    if (ExistDir(vfs, arg, 'f')) {
        printf("cd: %s: Not a directory\n", arg);
        return -1;
    }

    VNode* target = ExistDir(vfs, arg, 'd');
    if (target && IsPermission(target, 'r', users->current->UID) != 0) {
        printf("cd: %s: Permission denied\n", arg);
        return -1;
    }

    char buf[MAXD];
    strncpy(buf, arg, MAXD - 1);
    buf[MAXD - 1] = '\0';

    if (MovePath(vfs, buf) != 0) {
        printf("cd: %s: No such file or directory\n", arg);
        return -1;
    }

    UpdateUserDir(users, vfs);
    return 0;
}

int MovePath(VFS* vfs, const char* path) {
    VNode* saved = vfs->current;
    char   buf[MAXD];
    strncpy(buf, path, MAXD - 1);
    buf[MAXD - 1] = '\0';

    if (strcmp(buf, "/") == 0) {
        vfs->current = vfs->root;
        return 0;
    }

    char* sp;
    char* seg;
    if (buf[0] == '/') {
        vfs->current = vfs->root;
        seg = strtok_r(buf + 1, "/", &sp);
    } else {
        seg = strtok_r(buf, "/", &sp);
    }

    while (seg) {
        if (step_into(vfs, seg) != 0) {
            vfs->current = saved;
            return -1;
        }
        seg = strtok_r(NULL, "/", &sp);
    }
    return 0;
}

int step_into(VFS* vfs, const char* seg) {
    if (strcmp(seg, ".") == 0) return 0;

    if (strcmp(seg, "..") == 0) {
        if (vfs->current != vfs->root)
            vfs->current = vfs->current->parent;
        return 0;
    }

    VNode* d = ExistDir(vfs, seg, 'd');
    if (!d) return -1;

    vfs->current = d;
    return 0;
}

static void trace_path(VNode* node, char* buf, size_t sz) {
    if (!node) return;
    if (node->parent) trace_path(node->parent, buf, sz);

    if (node->parent) {
        strncat(buf, "/", sz - strlen(buf) - 1);
        strncat(buf, node->name, sz - strlen(buf) - 1);
    } else {
        strncat(buf, "/", sz - 2);
    }
}

void UpdateUserDir(UserDB* users, VFS* vfs) {
    if (!users || !users->current) return;
    char path[MAXD] = {0};
    trace_path(vfs->current, path, sizeof(path));
    strncpy(users->current->dir, path, MAXD - 1);
    users->current->dir[MAXD - 1] = '\0';
}
