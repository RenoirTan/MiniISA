#ifndef MINIAS_PARSER_H
#   define MINIAS_PARSER_H

#   include <miniisa/instruction.h>
#   include "token.h"

typedef struct parser {
    token_t prev_token;
    miniisa_instruction_t instruction;
} parser_t;

parser_t *init_parser(parser_t *p);

#endif
