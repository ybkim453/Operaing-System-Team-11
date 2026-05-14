#ifndef VFS_CORE_H
#define VFS_CORE_H

#include "vfs.h"

Stack*  stack_init(void);
void    stack_free(Stack* s);

VFS*    vfs_init(void);
void    vfs_free(VFS* vfs);

UserDB* users_init(void);
void    users_save(UserDB* users);
void    users_free(UserDB* users);
void    login(UserDB* users, VFS* vfs);
void    print_prompt(VFS* vfs, Stack* stack, UserDB* users);

#endif
