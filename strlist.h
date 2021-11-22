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
void strlist_add(StrList slst, char *str);

void strlist_del(StrList slst);

#endif

//////////////////////////////

// Could alternativly have a chunk of memory that is pre-alocated

#ifdef STRLIST_IMPLEMENTATION

#include <stdio.h>
#include <assert.h>

StrList strlist_new(size_t cap) {
    StrList slst;
    slst.index = (char **) malloc(sizeof(char *)*cap);
    slst.size = 0;
    slst.capacity = cap;
    return slst;
}

void strlist_add(StrList *slst, char *str) {
    // assert(slst.size < slst.capacity);
    size_t len = strlen(str);
    char *buff = (char *) malloc(sizeof(char)*len);
    strcpy(buff, str);
    slst->index[slst->size++] = buff;
}

void strlist_del(StrList *slst) {
    while (slst->size-- > 0)
        free(slst->index[slst->size]);
    free(slst->index);
}


#endif


