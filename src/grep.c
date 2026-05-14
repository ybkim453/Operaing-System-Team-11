#include <stdio.h>
#include <string.h>
#include <strings.h>
#include <ctype.h>
#include <stdlib.h>
#include <pthread.h>
#include "command.h"
#include "vfs.h"

extern char* file_content_store[256];

static pthread_mutex_t grep_lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct {
    VFS*        vfs;
    const char* word;
    const char* filename;
    int         line_num;
    int         icase;
    int         invert;
    int         only_match;
    int         multi;
} GrepJob;

static int str_icontains(const char* hay, const char* needle) {
    char lh[1024], ln[256];
    size_t i;
    for (i = 0; hay[i] && i < sizeof(lh) - 1; i++)
        lh[i] = tolower((unsigned char)hay[i]);
    lh[i] = '\0';
    for (i = 0; needle[i] && i < sizeof(ln) - 1; i++)
        ln[i] = tolower((unsigned char)needle[i]);
    ln[i] = '\0';
    return strstr(lh, ln) != NULL;
}

static void print_match_parts(const char* line, const char* word, int icase) {
    const char* ptr = line;
    size_t len = strlen(word);
    while (*ptr) {
        int hit = icase ? strncasecmp(ptr, word, len) == 0
                       : strncmp(ptr, word, len) == 0;
        if (hit) {
            printf("%.*s\n", (int)len, ptr);
            ptr += len;
        } else {
            ptr++;
        }
    }
}

static void do_grep(VFS* vfs, const char* filename, const char* word,
                    int line_num, int icase, int invert, int only_match, int multi) {
    VNode* file = ExistDir(vfs, filename, 'f');
    if (!file || file->contentIndex < 0 || file->contentIndex >= 256
            || !file_content_store[file->contentIndex]) {
        printf("grep: cannot open file: %s\n", filename);
        return;
    }

    char buf[4096];
    strncpy(buf, file_content_store[file->contentIndex], sizeof(buf) - 1);
    buf[sizeof(buf) - 1] = '\0';

    char* sp;
    char* ln  = strtok_r(buf, "\n", &sp);
    int   num = 1;

    while (ln) {
        int match = icase ? str_icontains(ln, word) : strstr(ln, word) != NULL;
        if (invert) match = !match;

        if (match) {
            if (only_match) {
                if (multi) printf("%s:", filename);
                print_match_parts(ln, word, icase);
            } else if (line_num) {
                if (multi) printf("%s:%d: %s\n", filename, num, ln);
                else       printf("%d: %s\n", num, ln);
            } else {
                if (multi) printf("%s: %s\n", filename, ln);
                else       printf("%s\n", ln);
            }
        }
        ln = strtok_r(NULL, "\n", &sp);
        num++;
    }
}

static void* grep_thread(void* arg) {
    GrepJob* j = (GrepJob*)arg;
    pthread_mutex_lock(&grep_lock);
    do_grep(j->vfs, j->filename, j->word,
            j->line_num, j->icase, j->invert, j->only_match, j->multi);
    pthread_mutex_unlock(&grep_lock);
    return NULL;
}

void command_grep(VFS* vfs, int argc, char** argv) {
    int line_num   = 0;
    int icase      = 0;
    int invert     = 0;
    int only_match = 0;
    const char* word = NULL;
    const char* files[16];
    int nf = 0;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            for (int j = 1; argv[i][j]; j++) {
                if      (argv[i][j] == 'n') line_num   = 1;
                else if (argv[i][j] == 'i') icase      = 1;
                else if (argv[i][j] == 'v') invert     = 1;
                else if (argv[i][j] == 'o') only_match = 1;
                else {
                    printf("grep: unknown option -- '%c'\n", argv[i][j]);
                    return;
                }
            }
        } else if (!word) {
            word = argv[i];
        } else if (nf < 16) {
            files[nf++] = argv[i];
        }
    }

    if (!word || nf == 0) {
        printf("Usage: grep [-n] [-i] [-v] [-o] <pattern> <file> [file...]\n");
        return;
    }

    if (invert && only_match) {
        printf("-o cannot be combined with -v.\n");
        return;
    }

    if (nf == 1) {
        do_grep(vfs, files[0], word, line_num, icase, invert, only_match, 0);
        return;
    }

    GrepJob*   jobs    = (GrepJob*)calloc(nf, sizeof(GrepJob));
    pthread_t* threads = (pthread_t*)malloc(nf * sizeof(pthread_t));

    for (int k = 0; k < nf; k++) {
        jobs[k].vfs        = vfs;
        jobs[k].word       = word;
        jobs[k].filename   = files[k];
        jobs[k].line_num   = line_num;
        jobs[k].icase      = icase;
        jobs[k].invert     = invert;
        jobs[k].only_match = only_match;
        jobs[k].multi      = 1;
        pthread_create(&threads[k], NULL, grep_thread, &jobs[k]);
    }

    for (int k = 0; k < nf; k++)
        pthread_join(threads[k], NULL);

    free(jobs);
    free(threads);
}
