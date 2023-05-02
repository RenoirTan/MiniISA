#include <stdio.h>
#include <stdlib.h>

#include <config.h>

typedef struct args {
    char *asm_fp;
    char *obj_fp;
} args_t;

static void help(void) {
    puts("minias " MINIISA_VERSION);
    puts("");
    puts("Assemble your MiniISA assembly files.");
    puts("");
    puts("Usage: minias <asmfile> <objfile>");
    puts("");
    puts("Arguments:");
    puts("    asmfile  Path to assembly file.");
    puts("    objfile  Path to output object file.");
}

int main(int argc, char **argv) {
    if (argc != 3) {
        help();
        exit(1);
    }
    args_t *args = malloc(sizeof(args_t));
    if (!args) {
        fputs("Could not allocate memory for args.", stderr);
    }
    args->asm_fp = argv[1];
    args->obj_fp = argv[2];
    return 0;
}