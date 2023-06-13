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
    // TODO: Spin this off to a separate file for initing an instruction
    miniisa_register_t reg_a = { .id = -1, .type = 0, .size = 0 };
    miniisa_register_t reg_b = { .id = -1, .type = 0, .size = 0 };
    miniisa_instruction_t instruction = {
        .opcode = MINIISA_OP_NOP,
        .reg_a = reg_a,
        .reg_b = reg_b,
        .extra = {0, 0, 0, 0, 0, 0, 0, 0}
    };
    p->instruction = instruction;
    return p;
}
