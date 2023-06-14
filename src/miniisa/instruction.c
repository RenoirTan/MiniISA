#include <stdlib.h>
#include <string.h>
#include <miniisa/instruction.h>

static void reset_register(miniisa_register_t *r) {
    r->id = -1;
    r->type = 0;
    r->size = 0;
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