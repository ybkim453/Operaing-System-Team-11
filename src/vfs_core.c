#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vfs.h"
#include "vfs_core.h"
#include "command.h"

extern char* file_content_store[256];

Stack* stack_init(void) {
    Stack* s = (Stack*)malloc(sizeof(Stack));
    if (s) s->TopNode = NULL;
    return s;
}

void stack_free(Stack* s) {
    while (s && s->TopNode) {
        SNode* tmp = s->TopNode;
        s->TopNode = tmp->linknode;
        free(tmp);
    }
    free(s);
}

VFS* vfs_init(void) {
    VFS*   vfs  = (VFS*)malloc(sizeof(VFS));
    VNode* root = (VNode*)calloc(1, sizeof(VNode));

    strcpy(root->name, "/");
    root->type         = 'd';
    root->parent       = NULL;
    root->child        = NULL;
    root->sibling      = NULL;
    root->SIZE         = 0;
    root->contentIndex = -1;
    for (int i = 0; i < 9; i++) root->permission[i] = 1;
    root->UID = 0;
    root->GID = 0;
    update_node_time(root);

    vfs->root    = root;
    vfs->current = root;
    return vfs;
}

static void destroy_tree(VNode* nd) {
    if (!nd) return;
    destroy_tree(nd->child);
    destroy_tree(nd->sibling);
    free(nd);
}

void vfs_free(VFS* vfs) {
    if (!vfs) return;
    destroy_tree(vfs->root);
    free(vfs);
}

UserDB* users_init(void) {
    UserDB* db = (UserDB*)malloc(sizeof(UserDB));
    db->head = db->tail = db->current = NULL;
    db->tUID = 0;
    db->tGID = 0;
    return db;
}

void login(UserDB* users, VFS* vfs) {
    (void)vfs;
    UNode* root_u = (UNode*)calloc(1, sizeof(UNode));
    strcpy(root_u->name, "root");
    strcpy(root_u->dir, "/");
    root_u->UID      = 0;
    root_u->GID      = 0;
    root_u->linknode = NULL;
    users->head = users->tail = users->current = root_u;
}

void users_save(UserDB* users) {
    (void)users;
}

void users_free(UserDB* users) {
    if (!users) return;
    UNode* cur = users->head;
    while (cur) {
        UNode* next = cur->linknode;
        free(cur);
        cur = next;
    }
    free(users);
}

void print_prompt(VFS* vfs, Stack* stack, UserDB* users) {
    VNode* cur = vfs->current;

    if (cur == vfs->root) {
        printf("%s@/:~$ ", users->current->name);
        return;
    }

    while (cur && cur->parent) {
        stack_push(stack, cur->name);
        cur = cur->parent;
    }

    printf("%s@/", users->current->name);
    while (!stack_is_empty(stack)) {
        printf("%s", stack_pop(stack));
        if (!stack_is_empty(stack)) putchar('/');
    }
    printf(":~$ ");
}
