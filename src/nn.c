#include "nn.h"
#include <unistd.h>
#include <stdlib.h>

#include "listfiles.h"

#define FLAG_IMPLEMENTATION
#include "flag.h"

void usage(FILE *sink, const char *program)
{
    fprintf(sink, "Usage: %s [OPTIONS] [--] <OUTPUT FILES...>\n", program);
    fprintf(sink, "OPTIONS:\n");
    flag_print_options(sink);
}

char * tmenu_browse(const char *notes, const char *regex) {
    struct popen2 child;
    chdir(notes);

    char temp_in[15], temp_out[15];
    strcpy(temp_in, "nn-in.XXXXXX");
    strcpy(temp_out, "nn-out.XXXXXX");

    int fp_in = mkstemp(temp_in);
    FILE *fd_in = fdopen(fp_in, "w");
    listfiles(fd_in, ".", regex);
    fclose(fd_in);
    
    int fp_out = mkstemp(temp_out);
    FILE *fd_out = fdopen(fp_out, "r");

    char cmd[1024];
    sprintf(cmd, "tmenu %s %s", temp_in, temp_out);
    system(cmd);

    char *buff = (char *) malloc(sizeof(char)*1024);
    memset(buff, 0, 1024);
    fread(buff, sizeof(char), 1024, fd_out);
    fclose(fd_out);

    if (*buff == 0)
        return NULL;

    // Remove newline
    char *c = buff;
    while (*c != '\n' && *c != 0) ++c;
    *c = 0;

    return buff;
}


char * dmenu_browse(const char *notes, const char *regex) {
    struct popen2 child;

    // char cmd[1024];
    // sprintf(cmd, "tmenu %s %s");
    // sprintf(cmd, "dmenu %s", DMENU_ARGS);

    chdir(notes);
    // popen2(cmd, &child);
    

    // char *temp = mktemp("XXXXX");
    char temp_in[15], temp_out[15];
    strcpy(temp_in, "nn-in.XXXXXX");
    strcpy(temp_out, "nn-out.XXXXXX");

    // FILE *fd = fopen(temp_in, "w");
    int fp_in = mkstemp(temp_in);
    FILE *fd_in = fdopen(fp_in, "w");
    listfiles(fd_in, ".", regex);
    fclose(fd_in);
    
    int fp_out = mkstemp(temp_out);
    FILE *fd_out = fdopen(fp_out, "r");

    char cmd[1024];
    sprintf(cmd, "tmenu %s %s", temp_in, temp_out);
    // sprintf(cmd, "dmenu %s", DMENU_ARGS);
    system(cmd);


    char *buff = (char *) malloc(sizeof(char)*1024);
    fread(buff, sizeof(char), 1024, fd_out);
    fclose(fd_out);

    // child.to_child
    // close(child.to_child);

    // char *buff = (char *) malloc(sizeof(char)*1024);
    // memset(buff, 0, 1024);
    // read(child.from_child, buff, 1024);
 
    // Check if wheter there was no output
    // if (*buff == 0)
    //     return NULL;

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
        for(int i=0; i<hist.size; ++i)
            fprintf(fd, "%s\n", hist.index[i]);
    }

    free(hist_path);
}


void open(const char *path) {
    char buff[1024];
    hist_add(path);

    // TODO: Flag and env_var

    sprintf(buff, "echo 'okular.exe \"%s\"' | cmd.exe", path);

    // sprintf(buff, "okular.exe \"$(wslpath -w '%s')\"", path);
    // sprintf(buff, "gio open '%s'", path);
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
        return g_cache_dir;

        // This should be recursive.
        // And check if failed
        mkdir(g_cache_dir, 0755);
    
        // hist_filepath = (char*) malloc(len + 6);
        // sprintf(hist_filepath, "%s/hist", cache_dir);
    } else {
        fprintf(stderr, "%s\n", "Cache directory not found");
        return 0;
    }
}

char *gen_histpath(const char *path) {
    // printf("%s\n", realpath(path, NULL));


    char *cache_dir = get_cachedir();

    // TODO: Make the directory if not exists!
    // printf("%s\n", cache_dir);

    size_t len = strlen(cache_dir) + 22;
    char *hist_path = (char *) malloc(len);

    sprintf(hist_path, "%s/%lx.hist", cache_dir, hash(realpath(path, NULL)));
    return hist_path;
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

    argv = flag_rest_argv();

    const char *path;
    if (*browse || *argv == 0) {
        path = tmenu_browse(*notes, ".pdf$\\|.djvu$");

        // TODO: Flag and env_var
        // path = dmenu_browse(*notes, ".pdf$\\|.djvu$");
        if (path != NULL)
            open(path);
    }

    // while (*argv) {
    //     open(argv++);
    // }
}
