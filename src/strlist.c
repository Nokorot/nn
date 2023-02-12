#include "strlist.h"

#include <stdio.h>
#include <assert.h>

StrList strlist_new(size_t cap) {
    StrList slst;
    slst.index = (char **) malloc(sizeof(char *)*cap);
    slst.size = 0;
    slst.capacity = cap;
    return slst;
}

// TODO: This data structure is terible
void strlist_add(StrList *slst, const char *str) {
    for(int i=0; i<slst->size; i++) {
        if (strcmp(slst->index[i], str) == 0)
            return;
    }


    // assert(slst.size < slst.capacity);
    size_t len = strlen(str);
    char *buff = (char *) malloc(sizeof(char)*(len+1));
    strcpy(buff, str);
    slst->index[slst->size++] = buff;
}

void strlist_del(StrList *slst) {
    while (slst->size-- > 0)
        free(slst->index[slst->size]);
    free(slst->index);
}

