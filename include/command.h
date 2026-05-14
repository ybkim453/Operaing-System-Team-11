#ifndef COMMAND_H
#define COMMAND_H

#include "vfs.h"
#include "path_stack.h"

void    command_ls(VFS* vfs, UserDB* users, int argc, char** argv);
int     command_cd(VFS* vfs, UserDB* users, const char* arg);
void    command_cat(VFS* vfs, UserDB* users, int argc, char** argv);
void    command_rm(VFS* vfs, UserDB* users, int argc, char** argv);
void    command_grep(VFS* vfs, int argc, char** argv);
int     command_mkdir(int argc, char** argv, VFS* vfs, UserDB* users);
int     command_chown(int argc, char** argv, VFS* vfs, UserDB* users);
int     command_pwd(VFS* vfs, Stack* stack, const char* arg);
int     command_mv(VFS* vfs, int argc, char** argv);

int     command_useradd(UserDB* users, int argc, char** argv);
int     command_userdel(UserDB* users, int argc, char** argv);
int     command_userlist(UserDB* users);

int     MovePath(VFS* vfs, const char* path);
int     step_into(VFS* vfs, const char* segment);
VNode*  ExistDir(VFS* vfs, const char* name, char type);
int     IsPermission(VNode* node, char mode, int uid);
void    RemoveDir(VFS* vfs, const char* name);
void    UpdateUserDir(UserDB* users, VFS* vfs);
void    MakeDir(VFS* vfs, char* name, char type);
void    update_node_time(VNode* node);

void    print_current_path(VFS* vfs, Stack* stack);

const char* GetNameByUID(UserDB* users, int uid);
const char* GetNameByGID(UserDB* users, int gid);

#endif
