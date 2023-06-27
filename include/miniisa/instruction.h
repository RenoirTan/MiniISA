#ifndef MINIISA_INSTRUCTION_H
#   define MINIISA_INSTRUCTION_H

#   include <stdint.h>

typedef int16_t miniisa_register_id_t;
typedef uint8_t miniisa_register_type_t;
typedef uint8_t miniisa_register_size_t;

typedef struct miniisa_register {
    miniisa_register_id_t id;
    miniisa_register_type_t type;
    miniisa_register_size_t size;
} miniisa_register_t;

void reset_register(miniisa_register_t *r);

typedef enum miniisa_ops {
    MINIISA_OP_INVALID = -1,
    MINIISA_OP_NOP = 0,
    MINIISA_OP_ADD = 1,
    MINIISA_OP_SUB = 2,
    MINIISA_OP_MUL = 3,
    MINIISA_OP_DIV = 4,
    MINIISA_OP_AND = 5,
    MINIISA_OP_IOR = 6,
    MINIISA_OP_XOR = 7,
    MINIISA_OP_NOT = 8,
    MINIISA_OP_SHL = 9,
    MINIISA_OP_SAL = 10,
    MINIISA_OP_SHR = 11,
    MINIISA_OP_SAR = 12,
    MINIISA_OP_ROL = 13,
    MINIISA_OP_ROR = 14,
    MINIISA_OP_SET = 15,
    MINIISA_OP_MOV = 16,
    MINIISA_OP_LDR = 17,
    MINIISA_OP_STR = 18,
    MINIISA_OP_PSH = 19,
    MINIISA_OP_POP = 20,
    MINIISA_OP_XCG = 21,
    MINIISA_OP_CVT = 22,
    MINIISA_OP_CMP = 23,
    MINIISA_OP_JZR = 24,
    MINIISA_OP_JNZ = 25,
    MINIISA_OP_JEQ = 26,
    MINIISA_OP_JNE = 27,
    MINIISA_OP_JLT = 28,
    MINIISA_OP_JLE = 29,
    MINIISA_OP_JGT = 30,
    MINIISA_OP_JGE = 31,
    MINIISA_OP_INT = 32,
    MINIISA_OP_HLT = 33
} miniisa_ops_t;

typedef struct miniisa_instruction {
    miniisa_ops_t opcode;
    miniisa_register_t reg_a;
    miniisa_register_t reg_b;
    uint8_t extra[8];
} miniisa_instruction_t;

miniisa_instruction_t *miniisa_instruction_init(miniisa_instruction_t *i);

int miniisa_instruction_to_bytes(
    miniisa_instruction_t *instruction,
    char *bytes,
    const char *end
);

int miniisa_bytes_to_instruction(
    char *bytes,
    const char *end,
    miniisa_instruction_t *instruction
);

#endif
