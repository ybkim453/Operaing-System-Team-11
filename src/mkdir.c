#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "vfs.h"
#include "command.h"

static pthread_mutex_t tree_lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    VFS*    vfs;
    char    path[MAXD];
    int     parents;
    int     mode;
    UserDB* users;
    int     result;
} MkdirJob;

static void apply_dir_meta(VNode* nd, int mode, UserDB* users) {
    for (int j = 0; j < 9; j++)
        nd->permission[j] = (mode >> (8 - j)) & 1;
    nd->UID = users->current->UID;
    nd->GID = users->current->GID;
}

static int do_mkdir(VFS* vfs, const char* name, int mode, UserDB* users) {
    if (ExistDir(vfs, name, 'd')) {
        fprintf(stderr, "mkdir: cannot create directory '%s': Directory exists\n", name);
        return -1;
    }
    MakeDir(vfs, (char*)name, 'd');
    VNode* nd = ExistDir(vfs, name, 'd');
    if (nd) apply_dir_meta(nd, mode, users);
    return 0;
}

static int do_mkdir_p(VFS* vfs, const char* fullpath, int mode, UserDB* users) {
    char  buf[MAXD];
    char* parts[MAXD];
    int   np = 0;

    strncpy(buf, fullpath, MAXD - 1);
    buf[MAXD - 1] = '\0';

    char* sp;
    char* tok = strtok_r(buf, "/", &sp);
    while (tok && np < MAXD)
        parts[np++] = tok, tok = strtok_r(NULL, "/", &sp);

    for (int i = 0; i < np; i++) {
        if (!ExistDir(vfs, parts[i], 'd')) {
            MakeDir(vfs, parts[i], 'd');
            VNode* nd = ExistDir(vfs, parts[i], 'd');
            if (nd) apply_dir_meta(nd, mode, users);
        }
        if (i < np - 1) MovePath(vfs, parts[i]);
    }

    for (int i = 0; i < np - 1; i++)
        MovePath(vfs, "..");

    return 0;
}

static void* thread_mkdir(void* arg) {
    MkdirJob* j = (MkdirJob*)arg;
    pthread_mutex_lock(&tree_lock);
    j->result = j->parents
        ? do_mkdir_p(j->vfs, j->path, j->mode, j->users)
        : do_mkdir(j->vfs, j->path, j->mode, j->users);
    pthread_mutex_unlock(&tree_lock);
    return NULL;
}

int command_mkdir(int argc, char** argv, VFS* vfs, UserDB* users) {
    if (argc < 2) {
        fprintf(stderr, "mkdir: missing operand\n");
        return -1;
    }

    int parents = 0, mode = 0777, i = 1;

    for (; i < argc && argv[i][0] == '-'; i++) {
        if (strcmp(argv[i], "-p") == 0 || strcmp(argv[i], "--parents") == 0) {
            parents = 1;
        } else if (strcmp(argv[i], "-m") == 0) {
            if (i + 1 >= argc) {
                fprintf(stderr, "mkdir: option requires an argument -- 'm'\n");
                return -1;
            }
            mode = (int)strtol(argv[++i], NULL, 8);
        } else {
            fprintf(stderr, "mkdir: invalid option -- '%s'\n", argv[i]);
            return -1;
        }
    }

    int n = argc - i;
    if (n == 0) {
        fprintf(stderr, "mkdir: missing operand\n");
        return -1;
    }

    if (n == 1)
        return parents ? do_mkdir_p(vfs, argv[i], mode, users)
                       : do_mkdir(vfs, argv[i], mode, users);

    MkdirJob*  jobs    = (MkdirJob*)calloc(n, sizeof(MkdirJob));
    pthread_t* threads = (pthread_t*)malloc(n * sizeof(pthread_t));

    for (int k = 0; k < n; k++) {
        jobs[k].vfs     = vfs;
        jobs[k].parents = parents;
        jobs[k].mode    = mode;
        jobs[k].users   = users;
        jobs[k].result  = 0;
        strncpy(jobs[k].path, argv[i + k], MAXD - 1);
        jobs[k].path[MAXD - 1] = '\0';
        pthread_create(&threads[k], NULL, thread_mkdir, &jobs[k]);
    }

    int ret = 0;
    for (int k = 0; k < n; k++) {
        pthread_join(threads[k], NULL);
        if (jobs[k].result != 0) ret = -1;
    }

    free(jobs);
    free(threads);
    return ret;
}
