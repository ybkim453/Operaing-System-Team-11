#include "command.h"
#include <stdlib.h>
#include <string.h>
#include <time.h>

void update_node_time(VNode* node) {
    time_t raw = time(NULL);
    struct tm* t = localtime(&raw);
    node->month  = t->tm_mon + 1;
    node->day    = t->tm_mday;
    node->hour   = t->tm_hour;
    node->minute = t->tm_min;
}

void MakeDir(VFS* vfs, char* name, char type) {
    VNode* nd = (VNode*)calloc(1, sizeof(VNode));
    if (!nd) return;

    strncpy(nd->name, name, MAXN - 1);
    nd->name[MAXN - 1] = '\0';
    nd->type    = type;
    nd->child   = NULL;
    nd->sibling = vfs->current->child;
    nd->parent  = vfs->current;
    nd->SIZE    = (type == 'd') ? 4096 : 0;
    nd->contentIndex = -1;

    update_node_time(nd);
    vfs->current->child = nd;
}

int IsPermission(VNode* node, char mode, int uid) {
    if (!node) return -1;

    int off;
    if      (mode == 'r') off = 0;
    else if (mode == 'w') off = 1;
    else if (mode == 'x') off = 2;
    else return -1;

    if (uid == 0) return 0;

    int base = (uid == node->UID) ? 0 : 6;
    return node->permission[base + off] ? 0 : -1;
}
