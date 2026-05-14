#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "command.h"
#include "vfs.h"

static UNode* alloc_unode(const char* name, int uid, int gid) {
    UNode* u = (UNode*)calloc(1, sizeof(UNode));
    if (!u) return NULL;
    strncpy(u->name, name, MAXN - 1);
    u->name[MAXN - 1] = '\0';
    strcpy(u->dir, "/");
    u->UID = uid;
    u->GID = gid;
    u->linknode = NULL;
    return u;
}

int command_useradd(UserDB* users, int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: useradd <username>\n");
        return -1;
    }

    const char* name = argv[1];
    for (UNode* n = users->head; n; n = n->linknode) {
        if (strcmp(n->name, name) == 0) {
            fprintf(stderr, "useradd: '%s' already exists\n", name);
            return -1;
        }
    }

    int uid = (users->tUID < 1000) ? 1000 : users->tUID + 1;
    int gid = (users->tGID < 1000) ? 1000 : users->tGID + 1;

    UNode* u = alloc_unode(name, uid, gid);
    if (!u) return -1;

    if (!users->head) users->head = users->tail = u;
    else { users->tail->linknode = u; users->tail = u; }

    users->tUID = uid;
    users->tGID = gid;

    printf("useradd: %s (uid=%d, gid=%d)\n", name, uid, gid);
    return 0;
}

int command_userdel(UserDB* users, int argc, char** argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: userdel <username>\n");
        return -1;
    }

    const char* name = argv[1];
    if (users->current && strcmp(users->current->name, name) == 0) {
        fprintf(stderr, "userdel: cannot delete current user\n");
        return -1;
    }

    UNode* prev = NULL;
    UNode* cur  = users->head;
    while (cur) {
        if (strcmp(cur->name, name) == 0) {
            if (prev) {
                prev->linknode = cur->linknode;
                if (cur == users->tail) users->tail = prev;
            } else {
                users->head = cur->linknode;
                if (cur == users->tail) users->tail = NULL;
            }
            free(cur);
            printf("userdel: %s removed\n", name);
            return 0;
        }
        prev = cur;
        cur  = cur->linknode;
    }

    fprintf(stderr, "userdel: '%s' not found\n", name);
    return -1;
}

int command_userlist(UserDB* users) {
    printf("%-16s %4s %4s\n", "USER", "UID", "GID");
    for (UNode* n = users->head; n; n = n->linknode)
        printf("%-16s %4d %4d\n", n->name, n->UID, n->GID);
    return 0;
}
