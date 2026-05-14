#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "command.h"
#include "vfs.h"

const char* GetNameByUID(UserDB* users, int uid) {
    for (UNode* n = users->head; n; n = n->linknode)
        if (n->UID == uid) return n->name;
    return "unknown";
}

const char* GetNameByGID(UserDB* users, int gid) {
    for (UNode* n = users->head; n; n = n->linknode)
        if (n->GID == gid) return n->name;
    return "unknown";
}

static int uid_by_name(UserDB* users, const char* name) {
    for (UNode* n = users->head; n; n = n->linknode)
        if (strcmp(n->name, name) == 0) return n->UID;
    return -1;
}

static int gid_by_name(UserDB* users, const char* name) {
    for (UNode* n = users->head; n; n = n->linknode)
        if (strcmp(n->name, name) == 0) return n->GID;
    return -1;
}

static void do_chown(VNode* nd, int uid, int gid, int recursive) {
    if (uid != -1) nd->UID = uid;
    if (gid != -1) nd->GID = gid;

    if (recursive && nd->type == 'd')
        for (VNode* c = nd->child; c; c = c->sibling)
            do_chown(c, uid, gid, 1);
}

int command_chown(int argc, char** argv, VFS* vfs, UserDB* users) {
    if (argc < 3) {
        fprintf(stderr, "Usage: chown [-R] owner[:group] target...\n");
        return -1;
    }

    int recursive = 0, i = 1;

    if (strcmp(argv[1], "-R") == 0 || strcmp(argv[1], "--recursive") == 0) {
        recursive = 1;
        i++;
    }

    if (i >= argc) {
        fprintf(stderr, "chown: missing operand after option\n");
        return -1;
    }

    char* spec   = strdup(argv[i++]);
    char* colon  = strchr(spec, ':');
    char* uname  = NULL;
    char* gname  = NULL;
    int   uid = -1, gid = -1;

    if (colon) {
        *colon = '\0';
        if (colon != spec)        uname = spec;
        if (*(colon + 1) != '\0') gname = colon + 1;
    } else {
        uname = spec;
    }

    if (uname) {
        uid = isdigit((unsigned char)*uname) ? atoi(uname) : uid_by_name(users, uname);
        if (uid == -1) {
            fprintf(stderr, "chown: invalid user '%s'\n", uname);
            free(spec);
            return -1;
        }
    }

    if (gname) {
        gid = isdigit((unsigned char)*gname) ? atoi(gname) : gid_by_name(users, gname);
        if (gid == -1) {
            fprintf(stderr, "chown: invalid group '%s'\n", gname);
            free(spec);
            return -1;
        }
    }

    int result = 0;
    for (; i < argc; i++) {
        VNode* nd = ExistDir(vfs, argv[i], 'd');
        if (!nd) nd = ExistDir(vfs, argv[i], 'f');
        if (!nd) {
            fprintf(stderr, "chown: cannot access '%s': No such file or directory\n", argv[i]);
            result = -1;
            continue;
        }
        do_chown(nd, uid, gid, recursive);
    }

    free(spec);
    return result;
}
