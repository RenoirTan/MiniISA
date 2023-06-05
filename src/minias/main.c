#include <stdio.h>
#include <stdlib.h>

#include <config.h>
#include "tokenizer.h"

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

    tokenizer_t* tokenizer = init_tokenizer(NULL);
    FILE *asm_file = fopen(args->asm_fp, "r");
    set_tokenizer_file(tokenizer, asm_file);
    token_t token;
    int status = 0;
    while (status != EOF) {
        status = get_next_token(tokenizer, &token);
        if (status == 1) {
            return 1;
        } else {
            printf("%s\n", token.span);
        }
    }
    return 0;
}
