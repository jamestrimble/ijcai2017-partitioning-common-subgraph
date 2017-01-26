#define _GNU_SOURCE
#define _POSIX_SOURCE

#include <argp.h>
#include <limits.h>
#include <locale.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

static void fail(char* msg) {
    printf("%s\n", msg);
    exit(1);
}

/*******************************************************************************
                             Command-line arguments
*******************************************************************************/

static char doc[] = "Remove vertex or edge labels from a VF-format graph";
static char args_doc[] = "INPUT-FILENAME OUTPUT-FILENAME";
static struct argp_option options[] = {
    {"no-edge-labels", 'e', 0, 0, "Remove edge labels"},
    {"no-vtx-labels", 'v', 0, 0, "Remove vertex labels"},
    { 0 }
};

static struct {
    bool use_edge_labels;
    bool use_vtx_labels;
    char *filename1;
    char *filename2;
    int arg_num;
} arguments;

void set_default_arguments() {
    arguments.use_edge_labels = true;
    arguments.use_vtx_labels = true;
    arguments.filename1 = NULL;
    arguments.filename2 = NULL;
    arguments.arg_num = 0;
}

static error_t parse_opt (int key, char *arg, struct argp_state *state) {
    switch (key) {
        case 'e':
            arguments.use_edge_labels = false;
            break;
        case 'v':
            arguments.use_vtx_labels = false;
            break;
        case ARGP_KEY_ARG:
            if (arguments.arg_num == 0) {
                arguments.filename1 = arg;
            } else if (arguments.arg_num == 1) {
                arguments.filename2 = arg;
            } else {
                argp_usage(state);
            }
            arguments.arg_num++;
            break;
        case ARGP_KEY_END:
            if (arguments.arg_num == 0)
                argp_usage(state);
            break;
        default: return ARGP_ERR_UNKNOWN;
    }
    return 0;
}

static struct argp argp = { options, parse_opt, args_doc, doc };

/*******************************************************************************/

int read_word(FILE *fp) {
    unsigned char a[2];
    if (fread(a, 1, 2, fp) != 2)
        fail("Error reading file.\n");
    return (int)a[0] | (((int)a[1]) << 8);
}

void write_word(FILE *fp, int word) {
    unsigned char a[2];
    a[0] = word & 0xFF;
    a[1] = (word & 0xFF00) >> 8;
    if (fwrite(a, 1, 2, fp) != 2)
        fail("Error writing file.\n");
}

// Precondition: *g is already zeroed out
void convert_graph(char* in_filename, char *out_filename) {
    FILE* f;
    FILE* f_out;
    
    if ((f=fopen(in_filename, "rb"))==NULL)
        fail("Cannot open input file");
    if ((f_out=fopen(out_filename, "wb"))==NULL)
        fail("Cannot open output file");

    int nvertices = read_word(f);
    write_word(f_out, nvertices);
    printf("%d vertices\n", nvertices);

    for (int i=0; i<nvertices; i++) {
        int label = read_word(f);
        write_word(f_out, arguments.use_vtx_labels ? label : 0);
    }

    for (int i=0; i<nvertices; i++) {
        int len = read_word(f);
        write_word(f_out, len);
        for (int j=0; j<len; j++) {
            int target = read_word(f);
            write_word(f_out, target);
            int label = read_word(f);
            write_word(f_out, arguments.use_edge_labels ? label : 1);
        }
    }
    fclose(f_out);
    fclose(f);
}

int main(int argc, char** argv) {
    set_default_arguments();
    argp_parse(&argp, argc, argv, 0, 0, 0);
    if (arguments.use_edge_labels && arguments.use_vtx_labels)
        fail("The -e or -v flag must be used.\n");

    convert_graph(arguments.filename1, arguments.filename2);
}
