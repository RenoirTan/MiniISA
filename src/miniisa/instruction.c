#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <_private/miniisa/extra.h>
#include <miniisa/instruction.h>

void reset_register(miniisa_register_t *r) {
    r->id = -1;
    r->type = 0xff;
    r->size = 0xff;
}

miniisa_instruction_t *miniisa_instruction_init(miniisa_instruction_t *i) {
    if (!i) {
        i = malloc(sizeof(miniisa_instruction_t));
        if (!i) return NULL;
    }
    i->opcode = MINIISA_OP_NOP;
    reset_register(&i->reg_a);
    reset_register(&i->reg_b);
    memset(i->extra, 0, 8);
    return i;
}

int miniisa_instruction_to_bytes(
    miniisa_instruction_t *instruction,
    char *bytes,
    const char *end
) {
    return 0;
}

int miniisa_bytes_to_instruction(
    char *bytes,
    const char *end,
    miniisa_instruction_t *instruction
) {
    return 0;
}
