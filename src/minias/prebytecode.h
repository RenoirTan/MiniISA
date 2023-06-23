#ifndef MINIAS_PREBYTECODE_H
#   define MINIAS_PREBYTECODE_H

#   include <stddef.h>
#   include <stdint.h>
#   include "token.h"

typedef enum data_type {
    UNSIGNED_INT_TYPE = 0,
    SIGNED_INT_TYPE = 1,
    FLOAT_TYPE = 2,
    UNKNOWN_TYPE = 0xFF
} data_type_t;

typedef enum data_size {
    BYTE_SIZE = 0, // 8-bit
    WORD_SIZE = 1, // 16-bit
    DWORD_SIZE = 2, // 32-bit
    QWORD_SIZE = 3, // 64-bit
    UNKNOWN_SIZE = 0xFF
} data_size_t;

typedef enum raw_mnemonic {
    INVALID_MNEMONIC,
    NOP_MNEMONIC,
    ADD_MNEMONIC,
    SUB_MNEMONIC,
    MUL_MNEMONIC,
    DIV_MNEMONIC,
    AND_MNEMONIC,
    IOR_MNEMONIC,
    XOR_MNEMONIC,
    NOT_MNEMONIC,
    SHL_MNEMONIC,
    SAL_MNEMONIC,
    SHR_MNEMONIC,
    SAR_MNEMONIC,
    ROL_MNEMONIC,
    ROR_MNEMONIC,
    SET_MNEMONIC,
    MOV_MNEMONIC,
    LDR_MNEMONIC,
    STR_MNEMONIC,
    PSH_MNEMONIC,
    POP_MNEMONIC,
    XCG_MNEMONIC,
    CVT_MNEMONIC,
    CMP_MNEMONIC,
    JZR_MNEMONIC,
    JNZ_MNEMONIC,
    JEQ_MNEMONIC,
    JNE_MNEMONIC,
    JLT_MNEMONIC,
    JLE_MNEMONIC,
    JGT_MNEMONIC,
    JGE_MNEMONIC,
    INT_MNEMONIC,
    HLT_MNEMONIC
} raw_mnemonic_t;

typedef enum register_id {
    R0_REG = 0,
    R1_REG = 1,
    R2_REG = 2,
    R3_REG = 3,
    R4_REG = 4,
    R5_REG = 5,
    R6_REG = 6,
    R7_REG = 7,
    R8_REG = 8,
    R9_REG = 9,
    R10_REG = 10,
    R11_REG = 11,
    RBP_REG = 12,
    RSP_REG = 13,
    RIP_REG = 14,
    FLG_REG = 15
} register_id_t;

typedef enum arg_kind {
    REGISTER_ARG = 0,
    SYMBOL_ARG = 1,
    VALUE_ARG = 2,
} arg_kind_t;

typedef struct register_arg {
    register_id_t id;
    data_type_t type;
    data_size_t size;
} register_arg_t;

typedef struct symbol_arg {
    char symbol[MAX_TOKEN_LEN+1];
} symbol_arg_t;

typedef struct value_arg {
    uint8_t value[8];
    data_type_t type;
    data_size_t size;
} value_arg_t;

typedef struct argument {
    arg_kind_t kind;
    union {
        register_arg_t reg;
        symbol_arg_t sym;
        value_arg_t val;
    } a;
} argument_t;

typedef enum stmt_kind {
    INSTRUCTION_STMT = 0,
    DATA_STMT = 1,
    SYMBOL_STMT = 2,
    SECTION_STMT = 3
} stmt_kind_t;

typedef struct instruction {
    raw_mnemonic_t mnemonic;
    argument_t arg_1;
    argument_t arg_2;
} instruction_t;

typedef struct data_stmt {
    uint8_t *data;
    size_t length;
} data_stmt_t;

typedef struct symbol_stmt {
    char name[MAX_TOKEN_LEN+1];
} symbol_stmt_t;

typedef struct section_stmt {
    char name[MAX_TOKEN_LEN+1];
} section_stmt_t;

typedef struct statement {
    stmt_kind_t kind;
    union {
        instruction_t instruction;
        data_stmt_t data;
        symbol_stmt_t symbol;
        section_stmt_t section;
    } s;
} statement_t;

typedef struct prebytecode {
    statement_t *statements;
    size_t stmts_count;
} prebytecode_t;

#endif
