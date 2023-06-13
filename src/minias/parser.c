#include <stdlib.h>
#include <miniisa/instruction.h>
#include "parser.h"
#include "token.h"

parser_t *init_parser(parser_t *p) {
    if (!p) {
        p = malloc(sizeof(parser_t));
        if (!p) return NULL;
    }
    init_token(&p->prev_token);
    miniisa_instruction_init(&p->instruction);
    return p;
}
