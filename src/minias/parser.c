#include <stdlib.h>
#include <string.h>
#include <miniisa/instruction.h>
#include "parser.h"
#include "token.h"

parser_t *init_parser(parser_t *p) {
    if (!p) {
        p = malloc(sizeof(parser_t));
        if (!p) return NULL;
    }
    init_token(&p->prev_token);
    p->state = PARSER_INIT;
    miniisa_instruction_init(&p->instruction);
    memset(p->data, '\0', PARSER_DATA_MAX_LEN);
    p->data_len = 0;
    return p;
}
