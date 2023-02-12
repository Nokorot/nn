#ifndef NN_H_
#define NN_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <regex.h>
#include <errno.h>

#include <sys/stat.h>

#include "flag.h"
#include "popen2.h"
#include "strlist.h"

static const char *DMENU_ARGS = "-l 20";

// typedef struct {
// } options_t;

char *get_cachedir();

char *gen_histpath(const char *path);

int main(int argc, char **argv);

void usage(FILE *sink, const char *program);

void open(const char *path);

char *dmenu_browse(const char *notes, const char *dmenu_args, const char *regex);

#endif
