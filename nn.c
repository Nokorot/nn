#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h> 
#include <regex.h>
#include <errno.h>

#include <sys/stat.h>

#define RECDIR_IMPLEMENTATION
#include "thirdparty/recdir.h"

#define FLAG_IMPLEMENTATION
#include "thirdparty/flag.h"

#define POPEN2_IMPLEMENTATION
#include "thirdparty/popen2.h"

#define STRLIST_IMPLEMENTATION
#include "strlist.h"

static const char *DMENU_ARGS = "-l 20";

static char *cache_dir;
static char *hist_filepath;

// typedef struct {
// } options_t;


void usage(FILE *sink, const char *program)
{
    fprintf(sink, "Usage: %s [OPTIONS] [--] <OUTPUT FILES...>\n", program);
    fprintf(sink, "OPTIONS:\n");
    flag_print_options(sink);
}

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

void read_hist(const char *filename) {
}

void listfiles(FILE *sink, const char *path, const char *regex) {
    RECDIR *recdir = recdir_open(path);
    
    read_hist("~/testA");

    regex_t rgx;
    regcomp(&rgx, regex, 0);

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
        printf("%s:\n", buff);
        read_blfile(buff, &blist);
    }

    errno = 0;
    while (ent = recdir_read(recdir, true)) {
        if (ent->d_type == DT_DIR) {
            current = recdir_top(recdir);
            blfile = look_for_file(current->dir, ".blacklist");
            if (blfile) {
                /// TODO: The blacklist should be a bitree.
                sprintf(buff, "%s/.blacklist", current->path);
                printf("%s:\n", buff);
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

        fprintf(sink, "%s/%s\n", recdir_top(recdir)->path, ent->d_name);
    }

    if (errno != 0) {
        fprintf(stderr,
                "ERROR: could not read the directory: %s\n",
                recdir_top(recdir)->path);
        exit(1);
    }

    recdir_close(recdir);
}

char * dmenu_browse(const char *notes, const char *regex) {
    struct popen2 child;
    
    char cmd[1024];
    sprintf(cmd, "dmenu %s", DMENU_ARGS);

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

void open(const char *path) {
    char buff[1024];
    // TODO: The program should be an option

    sprintf(buff, "gio open '%s'", path);
    // sprintf(buff, "zathura '%s'", path);
    system(buff);
}

void init(){
 // Find cache directiory
    int len;
    const char *homedir, *dsuffix = "";
	if ((homedir = getenv("XDG_CACHE_HOME")) == NULL || homedir[0] == '\0') {
		homedir = getenv("HOME");
		dsuffix = "/.cache";
	}
	if (homedir != NULL) {
		len = strlen(homedir) + strlen(dsuffix) + 4;
		cache_dir = (char*) malloc(len);
		snprintf(cache_dir, len, "%s%s/nn", homedir, dsuffix);
    
		cache_dir = (char*) malloc(len + 5);
		snprintf(hist_filepath, len, "%s/hist", cache_dir);
	} else {
		fprintf(stderr, "%s\n", "Cache directory not found");
    }
	
}

int main(int argc, char **argv) {
    const char *program = *argv;

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
 
    // Parse the chach path as an argument, when an option.
    // init();

    argv = flag_rest_argv();


    const char *path;
    if (*browse) {
        path = dmenu_browse(*notes, ".pdf$\\|.djvu$");
        if (path != NULL)
            open(path);
    }

    // while (*argv) {
    //     open(argv++);
    // }
}
