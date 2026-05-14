#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "command.h"
#include "vfs.h"

static void unlink_node(VNode* nd) {
    if (!nd || !nd->parent) return;
    VNode* par = nd->parent;
    if (par->child == nd) {
        par->child = nd->sibling;
    } else {
        VNode* p = par->child;
        while (p && p->sibling != nd) p = p->sibling;
        if (p) p->sibling = nd->sibling;
    }
    nd->sibling = NULL;
}

static void relink(VNode* nd, VNode* new_par) {
    nd->parent  = new_par;
    nd->sibling = new_par->child;
    new_par->child = nd;
}

int command_mv(VFS* vfs, int argc, char** argv) {
    if (argc != 3) {
        fprintf(stderr, "Usage: mv <source> <destination>\n");
        return -1;
    }

    const char* src_name = argv[1];
    const char* dst_name = argv[2];

    VNode* src = ExistDir(vfs, src_name, 'f');
    if (!src) src = ExistDir(vfs, src_name, 'd');
    if (!src) {
        fprintf(stderr, "mv: '%s': No such file or directory\n", src_name);
        return -1;
    }

    VNode* saved = vfs->current;

    VNode* dst_dir = ExistDir(vfs, dst_name, 'd');
    if (dst_dir) {
        unlink_node(src);
        relink(src, dst_dir);
        return 0;
    }

    if (strchr(dst_name, '/')) {
        char   buf[MAXD];
        char   newname[MAXN];
        strncpy(buf, dst_name, MAXD - 1);
        buf[MAXD - 1] = '\0';

        char* slash = strrchr(buf, '/');
        strncpy(newname, slash + 1, MAXN - 1);
        newname[MAXN - 1] = '\0';
        *slash = '\0';

        if (buf[0] == '\0') {
            vfs->current = vfs->root;
        } else if (MovePath(vfs, buf) != 0) {
            fprintf(stderr, "mv: '%s': No such directory\n", buf);
            vfs->current = saved;
            return -1;
        }
        VNode* dest = vfs->current;

        VNode* conflict = ExistDir(vfs, newname, 'f');
        if (!conflict) conflict = ExistDir(vfs, newname, 'd');
        if (conflict && conflict->type == 'd') {
            fprintf(stderr, "mv: cannot overwrite directory '%s'\n", dst_name);
            vfs->current = saved;
            return -1;
        }
        if (conflict && conflict->type == 'f')
            RemoveDir(vfs, newname);

        vfs->current = saved;
        unlink_node(src);
        strncpy(src->name, newname, MAXN - 1);
        src->name[MAXN - 1] = '\0';
        relink(src, dest);
        return 0;
    }

    VNode* conflict = ExistDir(vfs, dst_name, 'f');
    if (!conflict) conflict = ExistDir(vfs, dst_name, 'd');
    if (conflict && conflict->type == 'd') {
        fprintf(stderr, "mv: cannot overwrite directory '%s'\n", dst_name);
        return -1;
    }
    if (conflict && conflict->type == 'f')
        RemoveDir(vfs, dst_name);

    strncpy(src->name, dst_name, MAXN - 1);
    src->name[MAXN - 1] = '\0';
    return 0;
}
