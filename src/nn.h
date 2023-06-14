#ifndef NN_H_
#define NN_H_

static const char *DMENU_ARGS = "-l 20";

// typedef struct {
// } options_t;

char *get_cachedir();

char *gen_histpath(const char *path);

void open(const char *path);

char *dmenu_browse(const char *notes, const char *dmenu_args, const char *regex);

#endif
