#ifndef MINIISA_INSTRUCTION_H
#   define MINIISA_INSTRUCTION_H

#   include <stdint.h>

typedef int16_t register_id_t;
typedef uint8_t register_type_t;
typedef uint8_t register_size_t;

typedef struct cpu_register {
    register_id_t id;
    register_type_t type;
    register_size_t size;
} cpu_register_t;

typedef int32_t mnemonic_t;

typedef struct instruction {
    mnemonic_t mnemonic;
    cpu_register_t reg_a;
    cpu_register_t reg_b;
    uint8_t extra[8];
} instruction_t;

#   define NOP_MNEMONIC 0
#   define ADD_MNEMONIC 1
#   define SUB_MNEMONIC 2
#   define MUL_MNEMONIC 3
#   define DIV_MNEMONIC 4
#   define AND_MNEMONIC 5
#   define IOR_MNEMONIC 6
#   define XOR_MNEMONIC 7
#   define NOT_MNEMONIC 8
#   define SHL_MNEMONIC 9
#   define SAL_MNEMONIC 10
#   define SHR_MNEMONIC 11
#   define SAR_MNEMONIC 12
#   define ROL_MNEMONIC 13
#   define ROR_MNEMONIC 14
#   define SET_MNEMONIC 15
#   define MOV_MNEMONIC 16
#   define LDR_MNEMONIC 17
#   define STR_MNEMONIC 18
#   define PSH_MNEMONIC 19
#   define POP_MNEMONIC 20
#   define XCG_MNEMONIC 21
#   define CVT_MNEMONIC 22
#   define CMP_MNEMONIC 23
#   define JZR_MNEMONIC 24
#   define JNZ_MNEMONIC 25
#   define JEQ_MNEMONIC 26
#   define JNE_MNEMONIC 27
#   define JLT_MNEMONIC 28
#   define JLE_MNEMONIC 29
#   define JGT_MNEMONIC 30
#   define JGE_MNEMONIC 31
#   define INT_MNEMONIC 32
#   define HLT_MNEMONIC 33

#endif
