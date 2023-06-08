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

typedef int32_t miniisa_mnemonic_t;

typedef struct miniisa_instruction {
    miniisa_mnemonic_t mnemonic;
    miniisa_register_t reg_a;
    miniisa_register_t reg_b;
    uint8_t extra[8];
} miniisa_instruction_t;

#   define MINIISA_NOP 0
#   define MINIISA_ADD 1
#   define MINIISA_SUB 2
#   define MINIISA_MUL 3
#   define MINIISA_DIV 4
#   define MINIISA_AND 5
#   define MINIISA_IOR 6
#   define MINIISA_XOR 7
#   define MINIISA_NOT 8
#   define MINIISA_SHL 9
#   define MINIISA_SAL 10
#   define MINIISA_SHR 11
#   define MINIISA_SAR 12
#   define MINIISA_ROL 13
#   define MINIISA_ROR 14
#   define MINIISA_SET 15
#   define MINIISA_MOV 16
#   define MINIISA_LDR 17
#   define MINIISA_STR 18
#   define MINIISA_PSH 19
#   define MINIISA_POP 20
#   define MINIISA_XCG 21
#   define MINIISA_CVT 22
#   define MINIISA_CMP 23
#   define MINIISA_JZR 24
#   define MINIISA_JNZ 25
#   define MINIISA_JEQ 26
#   define MINIISA_JNE 27
#   define MINIISA_JLT 28
#   define MINIISA_JLE 29
#   define MINIISA_JGT 30
#   define MINIISA_JGE 31
#   define MINIISA_INT 32
#   define MINIISA_HLT 33

#endif
