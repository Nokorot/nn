#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h> 
#include <regex.h>
#include <errno.h>

#define RECDIR_IMPLEMENTATION
#include "thirdparty/recdir.h"

#define FLAG_IMPLEMENTATION
#include "thirdparty/flag.h"

#define POPEN2_IMPLEMENTATION
#include "thirdparty/popen2.h"

#define STRLIST_IMPLEMENTATION
#include "strlist.h"

const char *DMENU_ARGS = "-l 20";

void usage(FILE *sink, char *program)
{
    fprintf(sink, "Usage: %s [OPTIONS] [--] <OUTPUT FILES...>\n", program);
    fprintf(sink, "OPTIONS:\n");
    flag_print_options(sink);
}

struct dirent * look_for_file(DIR *dir, char *filename){
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
    
    char *line = NULL;
    char *c;
    size_t len;
    // ssize_t read;

    while (getline(&line, &len, fd) != -1) {
        // remove newline
        c = line;
        while (*c != '\n' && *c != 0) ++c;
        *c = 0;
        // printf("%s",line);
        // *line = 0;
        // Might want to look for duplicates
        strlist_add(blist, line);

    }
    
    fclose(fd);
}

void listfiles(FILE *sink, char *path, char *regex) {
    RECDIR *recdir = recdir_open(path);
    
    regex_t rgx;
    regcomp(&rgx, regex, 0);

    char buff[1024];
    StrList blist = strlist_new(1024);
    bool blacklisted;
    int i;

    errno = 0;
    struct dirent *ent, *blfile;
    while (ent = recdir_read(recdir, true)) {
        if (ent->d_type == DT_DIR) {
            RECDIR_Frame *current = recdir_top(recdir);
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

        // printf("%s/%s\n", recdir_top(recdir)->path, ent->d_name);
        fprintf(sink, "%s/%s\n", recdir_top(recdir)->path, ent->d_name);
        // char *path = join_path(recdir_top(recdir)->path, ent->d_name);
    }

    if (errno != 0) {
        fprintf(stderr,
                "ERROR: could not read the directory: %s\n",
                recdir_top(recdir)->path);
        exit(1);
    }

    recdir_close(recdir);
}

char * dmenu_browse(char *notes, char *regex) {
    struct popen2 child;
    
    chdir(notes);
    popen2("dmenu -l 20", &child);

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

void open(char *path) {
    char buff[1024];
    sprintf(buff, "gio open '%s'", path);
    system(buff);
    // FILE *fd = popen(buff);
    // pclose(fd);
}


int main(int argc, char **argv) {
    char *program = *argv;
    
    // TODO:  Ignore hidden option

        
    bool *help = flag_bool("help", false, "Print this help to stdout and exit with 0");
    bool *browse = flag_bool("browse", false, "Line to output to the file");

    char *notes_env = getenv("NOTES");
    char **notes = flag_str("notes-dir", notes_env, "Amount of lines to generate");

    if (!flag_parse(argc, argv)) {
        usage(stderr, program);
        flag_print_error(stderr);
        exit(1);
    }

    if (*help) {
        usage(stdout, program);
        exit(0);
    }

    if (*notes == NULL) {
        usage(stderr, program);
        fprintf(stderr, "ERROR: No 'notes directory' was provided. Set the environment \
                variable `NOTES` or provide a directory with --note-dir option");
        exit(1);
    }
    
    argv = flag_rest_argv();

    char *regex = ".pdf$\\|.djvu$";
    char *path;
    if (*browse) {
        path = dmenu_browse(*notes, regex);
        if (path != NULL)
            open(path);
    }
    // while (*argv) {
    //     open(argv++);
    // }
}

