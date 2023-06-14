#include "nn.h"
#include <unistd.h>

// #include <stdio.h>
// #include <stdlib.h>
// #include <stdbool.h>
// #include <string.h>
// #include <regex.h>
// #include <errno.h>

#include <sys/stat.h>

#include "popen2.h"
#include "strlist.h"

#include "listfiles.h"

char * dmenu_browse(const char *notes, const char *dmenu_args, const char *regex) {
    struct popen2 child;

    char cmd[1024];
    sprintf(cmd, "dmenu %s", dmenu_args);

    chdir(notes);
    popen2(cmd, &child);

    FILE *fd = fdopen(child.to_child, "w");
    listfiles(fd, ".", regex);
    fflush(fd);
    close(child.to_child);

    char *buff = (char *) malloc(sizeof(char)*1024);
    memset(buff, 0, 1024);
    read(child.from_child, buff, 1024);

    // Check if wheter there was no output
    if (*buff == 0)
        return NULL;

    // Remove newline
    char *c = buff;
    while (*c != '\n' && *c != 0) ++c;
    *c = 0;

    return buff;
}

void hist_add(const char *path) {
    char *hist_path = gen_histpath(".");
    StrList hist = strlist_new(1024);

    strlist_add(&hist, path);
    read_hist(&hist, hist_path);

    FILE *fd = fopen(hist_path, "w");
    if (fd) {
        // TODO: Max history length
        for(int i=0; i<hist.size; ++i)
            fprintf(fd, "%s\n", hist.index[i]);
    }

    free(hist_path);
}


void open(const char *path) {
    char buff[1024];
    // TODO: The program should be an option

    hist_add(path);

    sprintf(buff, "gio open '%s'", path);
    // sprintf(buff, "zathura '%s'", path);
    system(buff);
}


unsigned long
hash(const char *str)
{
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

static char *g_cache_dir = 0;
char *get_cachedir() {
    if (g_cache_dir)
        return g_cache_dir;

    int len;
    const char *homedir, *dsuffix = "";
    if ((homedir = getenv("XDG_CACHE_HOME")) == NULL || homedir[0] == '\0') {
        homedir = getenv("HOME");
        dsuffix = "/.cache";
    }
    if (homedir != NULL) {
        len = strlen(homedir) + strlen(dsuffix) + 5;
        g_cache_dir = (char*) malloc(len);
        snprintf(g_cache_dir, len, "%s%s/nn", homedir, dsuffix);

        // TODO: This should be recursive.
        // And check if failed
        mkdir(g_cache_dir, 0755);
        return g_cache_dir;
    } else {
        fprintf(stderr, "%s\n", "Cache directory not found");
        return 0;
    }
}

char *gen_histpath(const char *path) {
    printf("%s\n", realpath(path, NULL));

    char *cache_dir = get_cachedir();

    size_t len = strlen(cache_dir) + 22;
    char *hist_path = (char *) malloc(len);

    sprintf(hist_path, "%s/%lx.hist", cache_dir, hash(realpath(path, NULL)));
    return hist_path;
}
