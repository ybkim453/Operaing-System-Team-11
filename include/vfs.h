#ifndef VFS_H
#define VFS_H

#define MAXN 64
#define MAXD 256
#include <time.h>

typedef struct tagUN {
    char name[MAXN];
    char dir[MAXD];
    int year, month, wday, day, hour, minute, sec;
    int UID, GID;
    struct tagUN* linknode;
} UNode;

typedef struct tagUserDB {
    int tUID;
    int tGID;
    UNode* head;
    UNode* tail;
    UNode* current;
} UserDB;

typedef struct tagVN {
    char name[MAXN];
    char type;
    int mode;
    int permission[9];
    int SIZE;
    int contentIndex;
    int UID, GID;
    int month, day, hour, minute;
    struct tagVN* parent;
    struct tagVN* child;
    struct tagVN* sibling;
} VNode;

typedef struct tagVFS {
    VNode* root;
    VNode* current;
} VFS;

typedef struct tagSNode {
    char name[MAXN];
    struct tagSNode* linknode;
} SNode;

typedef struct tagStack {
    SNode* TopNode;
} Stack;

#endif
