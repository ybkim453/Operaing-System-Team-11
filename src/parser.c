#include "command.h"
#include "parser.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void ParseAndExecute(VFS* vfs, Stack* stack, UserDB* users, char* input) {
    char* argv[16];
    int argc = 0;
    char* saveptr;
    char* token = strtok_r(input, " ", &saveptr);
    while (token && argc < 16) {
        argv[argc++] = token;
        token = strtok_r(NULL, " ", &saveptr);
    }

    if (argc == 0) return;

    if (strcmp(argv[0], "ls") == 0) {
        command_ls(vfs, users, argc, argv);
    } else if (strcmp(argv[0], "cd") == 0) {
        command_cd(vfs, users, argc > 1 ? argv[1] : NULL);
    } else if (strcmp(argv[0], "cat") == 0) {
        command_cat(vfs, users, argc, argv);
    } else if (strcmp(argv[0], "pwd") == 0) {
        command_pwd(vfs, stack, argc > 1 ? argv[1] : NULL);
    } else if (strcmp(argv[0], "mkdir") == 0) {
        command_mkdir(argc, argv, vfs, users);
    } else if (strcmp(argv[0], "rm") == 0) {
        command_rm(vfs, users, argc, argv);
    } else if (strcmp(argv[0], "mv") == 0) {
        command_mv(vfs, argc, argv);
    } else if (strcmp(argv[0], "grep") == 0) {
        command_grep(vfs, argc, argv);
    } else if (strcmp(argv[0], "chown") == 0) {
        command_chown(argc, argv, vfs, users);
    } else if (strcmp(argv[0], "useradd") == 0) {
        command_useradd(users, argc, argv);
    } else if (strcmp(argv[0], "userdel") == 0) {
        command_userdel(users, argc, argv);
    } else if (strcmp(argv[0], "userlist") == 0) {
        command_userlist(users);
    } else {
        printf("Unknown command: %s\n", argv[0]);
    }
}
