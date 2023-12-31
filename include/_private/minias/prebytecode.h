#ifndef MINIAS_PREBYTECODE_H
#   define MINIAS_PREBYTECODE_H

#   include <stddef.h>
#   include <stdint.h>
#   include "token.h"

#   ifdef __cplusplus
extern "C" {
#   endif

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
    FLG_REG = 15,
    UNKNOWN_REG = 0xFF
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

register_arg_t *init_register_arg(register_arg_t *r);

register_id_t parse_register_id(char *s);

int parse_register(char *s, register_arg_t *reg);

typedef struct symbol_arg {
    char name[MAX_TOKEN_LEN+1];
} symbol_arg_t;

symbol_arg_t *init_symbol_arg(symbol_arg_t *s);

size_t set_symbol_name(symbol_arg_t *s, char *name);

typedef struct value_arg {
    uint8_t value[8];
    data_type_t type;
    data_size_t size;
} value_arg_t;

value_arg_t *init_value_arg(value_arg_t *v);

typedef struct argument {
    arg_kind_t kind;
    union {
        register_arg_t reg;
        symbol_arg_t sym;
        value_arg_t val;
    } a;
} argument_t;

argument_t *init_argument(argument_t *a);

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

instruction_t *init_instruction(instruction_t *i);

int parse_instruction_mnemonic(char *s, instruction_t *i);

typedef struct data_stmt {
    uint8_t *data;
    size_t length; // inferred
    size_t size; // forced by programmer
    data_type_t type; // for conversion purposes
} data_stmt_t;

data_stmt_t *init_data_stmt(data_stmt_t *d);

int set_data(data_stmt_t *d, uint8_t *value, size_t length);

typedef struct symbol_stmt {
    char name[MAX_TOKEN_LEN+1];
} symbol_stmt_t;

symbol_stmt_t *init_symbol_stmt(symbol_stmt_t *s);

typedef struct section_stmt {
    char name[MAX_TOKEN_LEN+1];
} section_stmt_t;

section_stmt_t *init_section_stmt(section_stmt_t *s);

typedef struct statement {
    stmt_kind_t kind;
    union {
        instruction_t instruction;
        data_stmt_t data;
        symbol_stmt_t symbol;
        section_stmt_t section;
    } s;
} statement_t;

statement_t *init_statement(statement_t *s);

typedef struct prebytecode {
    statement_t *statements;
    size_t stmts_count;
    size_t stmts_capacity;
} prebytecode_t;

prebytecode_t *init_prebytecode(prebytecode_t *p);

int set_prebytecode_capacity(prebytecode_t *p, size_t new_cap);

int push_prebytecode_stmt(prebytecode_t *p, statement_t *s);

#   ifdef __cplusplus
}
#   endif

#endif
