#ifndef LISTFILES_H_
#define LISTFILES_H_

#include <stdio.h>
 
#include "strlist.h"
#include "recdir.h"


struct dirent * look_for_file(DIR *dir, const char *filename);

void read_blfile(char *path, StrList *blist);

void read_hist(StrList *files, const char *hist_path);

// void listfiles(StrList *paths, const char *path, const char *regex);

void listfiles(FILE *sink, const char *path, const char *regex);

#endif
