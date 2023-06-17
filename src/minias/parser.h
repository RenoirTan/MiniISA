#ifndef MINIAS_PARSER_H
#   define MINIAS_PARSER_H

#   include <stddef.h>
#   include <miniisa/instruction.h>
#   include "token.h"

#   define PARSER_DATA_MAX_LEN (1024)

typedef enum parser_state {
    PARSER_INIT = 0,
    PARSER_DONE = 1,
    PARSER_GETTING_NAME = 2,
    PARSER_DETECTING_TYPE = 3,
    PARSER_SETTING_DATA = 4,
    PARSER_FINDING_ARGUMENT = 5,
    PARSER_WAITING_COMMA = 6
} parser_state_t;

typedef struct parser {
    token_t prev_token;
    parser_state_t state;
    miniisa_instruction_t instruction;
    uint8_t data[PARSER_DATA_MAX_LEN];
    size_t data_len;
} parser_t;

parser_t *init_parser(parser_t *p);

#endif
