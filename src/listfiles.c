#include "listfiles.h"

#include "recdir.h"
#include "nn.h"

struct dirent * look_for_file(DIR *dir, const char *filename){
    long loc = telldir(dir);
    struct dirent *ent = NULL;
    while ((ent = readdir(dir))) {
        if (strcmp(ent->d_name, filename) == 0) 
            break;
    }
    seekdir(dir, loc);
    return ent;
}

void read_blfile(char *path, StrList *blist) {
    FILE *fd = fopen(path, "r");
    if (fd == NULL) {
        fprintf(stderr, "ERROR: Failed to open blacklist file `%s`\n", path);
        exit(1);
    }
    
    size_t len;
    char *line = NULL, *c;
    while (getline(&line, &len, fd) != -1) {
        // remove newline
        for (c = line; *c != '\n' && *c != 0; ++c);
        *c = 0;
        // TODO: Might want to look for duplicates. A bitree would fix this
        strlist_add(blist, line);
    }
    
    fclose(fd);
}

void read_hist(StrList *files, const char *hist_path) {
    FILE *fd = fopen(hist_path, "r");

    if (fd) {
        size_t len;
        char *line = NULL, *c;
        while (getline(&line, &len, fd) != -1) {
            // remove newline
            for (c = line; *c != '\n' && *c != 0; ++c);
            *c = 0;
            strlist_add(files, line);
        }
        
        fclose(fd);
    }

}

// void listfiles(StrList *paths, const char *path, const char *regex) {

void listfiles(FILE *sink, const char *path, const char *regex) {
    RECDIR *recdir = recdir_open(path);
    

    regex_t rgx;
    regcomp(&rgx, regex, 0);

    StrList files = strlist_new(2048);

    char *hist_path = gen_histpath(path);
    read_hist(&files, hist_path);
    free(hist_path);

    char buff[1024];
    StrList blist = strlist_new(1024);
    bool blacklisted;
    int i;

    struct dirent *ent, *blfile;
    RECDIR_Frame *current = recdir_top(recdir);

    blfile = look_for_file(current->dir, ".blacklist");
    if (blfile) {
        /// TODO: The blacklist should be a bitree.
        sprintf(buff, "%s/.blacklist", current->path);
        read_blfile(buff, &blist);
    }

    int count=0;

    errno = 0;
    while (ent = recdir_read(recdir, true)) {
        if (ent->d_type == DT_DIR) {
            current = recdir_top(recdir);
            blfile = look_for_file(current->dir, ".blacklist");
            if (blfile) {
                /// TODO: The blacklist should be a bitree.
                sprintf(buff, "%s/.blacklist", current->path);
                read_blfile(buff, &blist);
            }
            
            blacklisted = false;
            for (i=0; !blacklisted && i<blist.size; ++i){
                if (strcmp(ent->d_name, blist.index[i]) == 0) {
                    blacklisted = true;
                }
            }
            if (blacklisted){
                recdir_pop(recdir);
                continue;
            }
        }
    
        blacklisted = false;
        for (i=0; !blacklisted && i<blist.size; ++i) {
            if (strcmp(ent->d_name, blist.index[i]) == 0) {
                blacklisted = true;
            }
        }
        if (blacklisted){
            continue;
        }
        if (regexec(&rgx, ent->d_name, 0, NULL, 0)) 
            continue;

        sprintf(buff, "%s/%s", recdir_top(recdir)->path, ent->d_name);
        strlist_add(&files, buff);

        count++;
        // fprintf(sink, "%s/%s\n", recdir_top(recdir)->path, ent->d_name);
    }

    for (i=0; i < files.size; ++i) {
        fprintf(sink, "%s\n", files.index[i]);
    }

    
    if (errno != 0) {
        fprintf(stderr,
                "ERROR: could not read the directory: %s\n",
                recdir_top(recdir)->path);
        exit(1);
    }

    recdir_close(recdir);
}
