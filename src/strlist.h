#ifndef STRLIST_H_
#define STRLIST_H_

#include <stdlib.h>
#include <string.h> 

typedef struct {
    char **index;
    size_t size;
    size_t capacity;
} StrList;

StrList strlist_new(size_t cap);
void strlist_add(StrList *slst, const char *str);

void strlist_del(StrList *slst);

#endif
