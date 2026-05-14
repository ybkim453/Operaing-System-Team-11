#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include "vfs.h"
#include "command.h"

extern char* file_content_store[256];

static pthread_mutex_t rm_lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    VFS*    vfs;
    UserDB* users;
    char    path[MAXD];
    int     recursive;
    int     force;
    int     verbose;
} RmJob;

static char* parent_path(const char* path) {
    static char dir[MAXD];
    strncpy(dir, path, MAXD - 1);
    dir[MAXD - 1] = '\0';

    char* last = strrchr(dir, '/');
    if (last) {
        if (last == dir) *(last + 1) = '\0';
        else             *last = '\0';
    } else {
        dir[0] = '\0';
    }
    return dir;
}

VNode* ExistDir(VFS* vfs, const char* name, char type) {
    for (VNode* t = vfs->current->child; t != NULL; t = t->sibling)
        if (strcmp(t->name, name) == 0 && t->type == type)
            return t;
    return NULL;
}

void RemoveDir(VFS* vfs, const char* name) {
    VNode* prev = NULL;
    VNode* cur  = vfs->current->child;

    while (cur) {
        if (strcmp(cur->name, name) == 0) {
            if (prev) prev->sibling = cur->sibling;
            else      vfs->current->child = cur->sibling;

            if (cur->type == 'f' && cur->contentIndex >= 0
                    && cur->contentIndex < 256
                    && file_content_store[cur->contentIndex]) {
                free(file_content_store[cur->contentIndex]);
                file_content_store[cur->contentIndex] = NULL;
            }
            free(cur);
            return;
        }
        prev = cur;
        cur  = cur->sibling;
    }
}

static void do_rm(VFS* vfs, UserDB* users, const char* path,
                  int recursive, int force, int verbose) {
    VNode* orig = vfs->current;
    char   pathbuf[MAXD], dirpart[MAXD], base[MAXN];

    strncpy(pathbuf, path, MAXD - 1);
    pathbuf[MAXD - 1] = '\0';

    if (strchr(path, '/')) {
        strncpy(dirpart, parent_path(path), MAXD - 1);
        dirpart[MAXD - 1] = '\0';

        if (MovePath(vfs, dirpart) != 0) {
            if (!force)
                printf("rm: '%s': No such file or directory.\n", dirpart);
            return;
        }

        char* sp;
        char* seg = strtok_r(pathbuf, "/", &sp);
        while (seg) {
            strncpy(base, seg, MAXN - 1);
            base[MAXN - 1] = '\0';
            seg = strtok_r(NULL, "/", &sp);
        }
    } else {
        strncpy(base, path, MAXN - 1);
        base[MAXN - 1] = '\0';
    }

    VNode* tgt = ExistDir(vfs, base, 'd');
    if (tgt && !recursive) {
        if (!force)
            printf("rm: cannot remove '%s': recursive option required.\n", base);
        vfs->current = orig;
        return;
    }
    if (!tgt) tgt = ExistDir(vfs, base, 'f');

    if (!tgt) {
        if (!force)
            printf("rm: '%s': No such file or directory.\n", base);
        vfs->current = orig;
        return;
    }

    if (!force && IsPermission(tgt, 'w', users->current->UID) != 0) {
        printf("rm: '%s': Permission denied.\n", base);
        vfs->current = orig;
        return;
    }

    if (tgt->type == 'd' && recursive) {
        VNode* here = vfs->current;
        vfs->current = tgt;
        while (tgt->child) {
            char child_name[MAXN];
            strncpy(child_name, tgt->child->name, MAXN - 1);
            child_name[MAXN - 1] = '\0';
            do_rm(vfs, users, child_name, recursive, force, verbose);
        }
        vfs->current = here;
    }

    if (verbose) printf("Deleted: %s\n", base);
    RemoveDir(vfs, base);
    vfs->current = orig;
}

static void* rm_thread(void* arg) {
    RmJob* j = (RmJob*)arg;
    pthread_mutex_lock(&rm_lock);
    do_rm(j->vfs, j->users, j->path, j->recursive, j->force, j->verbose);
    pthread_mutex_unlock(&rm_lock);
    return NULL;
}

void command_rm(VFS* vfs, UserDB* users, int argc, char** argv) {
    int recursive = 0, force = 0, verbose = 0;
    int i = 1;

    for (; i < argc && argv[i][0] == '-'; i++) {
        for (int j = 1; argv[i][j]; j++) {
            if      (argv[i][j] == 'r') recursive = 1;
            else if (argv[i][j] == 'f') force     = 1;
            else if (argv[i][j] == 'v') verbose   = 1;
            else {
                printf("rm: unknown option -- '%c'\n", argv[i][j]);
                return;
            }
        }
    }

    int ntargets = argc - i;
    if (ntargets == 0) {
        fprintf(stderr, "rm: missing operand\n");
        return;
    }

    if (ntargets == 1) {
        do_rm(vfs, users, argv[i], recursive, force, verbose);
        return;
    }

    RmJob*     jobs    = (RmJob*)calloc(ntargets, sizeof(RmJob));
    pthread_t* threads = (pthread_t*)malloc(ntargets * sizeof(pthread_t));

    for (int k = 0; k < ntargets; k++) {
        jobs[k].vfs       = vfs;
        jobs[k].users     = users;
        jobs[k].recursive = recursive;
        jobs[k].force     = force;
        jobs[k].verbose   = verbose;
        strncpy(jobs[k].path, argv[i + k], MAXD - 1);
        jobs[k].path[MAXD - 1] = '\0';
        pthread_create(&threads[k], NULL, rm_thread, &jobs[k]);
    }

    for (int k = 0; k < ntargets; k++)
        pthread_join(threads[k], NULL);

    free(jobs);
    free(threads);
}
