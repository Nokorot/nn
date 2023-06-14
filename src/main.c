#include <stdio.h>

#define FLAG_IMPLEMENTATION
#include "flag.h"

#include "nn.h"

// This is dumb
static const char * FILETYPES = ".pdf$\\|.djvu$";
//  static const char * FILETYPES = "pdf,djvu";


int main(int argc, char **argv);
void usage(FILE *sink, const char *program);


void usage(FILE *sink, const char *program)
{
    fprintf(sink, "Usage: %s [OPTIONS]\n", program);
    fprintf(sink, "OPTIONS:\n");
    flag_print_options(sink);
}

int main(int argc, char **argv) {
    const char *program = *argv;

    // TODO:  Ignore hidden option
    bool *help = flag_bool("help", false, "Print this help to stdout and exit with 0");
    bool *browse = flag_bool("browse", false, "Line to output to the file");
    bool *get_cache = flag_bool("get-cache", false, "Prints the path of the cache directory to standard output");

    char *notes_env = getenv("NOTES");
    char **notes = flag_str("notes-dir", notes_env, "Amount of lines to generate");
    char **regexString = flag_str("regex", FILETYPES, "Regex String to filter the files");

    char **dmenu_args = flag_str("dmargs", DMENU_ARGS, "Arguments that are pased to dmenu");


    if (!flag_parse(argc, argv)) {
        usage(stderr, program);
        flag_print_error(stderr);
        exit(1);
    }

    if (*help) {
        usage(stdout, program);
        exit(0);
    }

    if (*get_cache) {
        printf("%s\n", get_cachedir());
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
    if (*browse) {
        path = dmenu_browse(*notes, *dmenu_args, *regexString);
        if (path != NULL)
            open(path);
    }

    // while (*argv) {
    //     open(argv++);
    // }
}
