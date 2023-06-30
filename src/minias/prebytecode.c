#include <ctype.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <miniisa/extra.h>
#include <miniisa/strnlen.h>
#include "prebytecode.h"

register_arg_t *init_register_arg(register_arg_t *r) {
    __MINIISA_INIT(r, register_arg_t);
    r->id = -1;
    r->type = UNKNOWN_TYPE;
    r->size = UNKNOWN_SIZE;
    return r;
}

register_id_t parse_register_id(char *s) {
    size_t s_len = strlen(s);
    if (s_len < 2) {
        return -1;
    }
    if (!strncmp(s, "rbp", 3)) {
        return 0xc;
    } else if (!strncmp(s, "rsp", 3)) {
        return 0xd;
    } else if (!strncmp(s, "rip", 3)) {
        return 0xe;
    } else if (!strncmp(s, "flg", 3)) {
        return 0xf;
    } else if (s[0] == 'r') {
        if (isdigit(s[1]) && !isdigit(s[2])) {
            return s[1] - '0'; // converts '0' to 0 and so on
        // fails when something like r18 is encountered
        } else if (s[1] == '1' && isdigit(s[2]) && (s[2] == '0' || s[2] == '1')) {
            return (s[2] - '0') + 10;
        } else {
            return -2;
        }
    } else {
        return -1;
    }
}

int parse_register(char *s, register_arg_t *reg) {
    size_t s_len = strlen(s);
    register_arg_t r;
    init_register_arg(&r);
    r.type = 0xff;
    r.size = 0xff;
    r.id = parse_register_id(s);
    if (r.id < 0) {
        __DBG("parse_register: invalid register: %s\n", s);
        return 1;
    }
    size_t index = r.id <= 9 ? 2 : 3; // r1, ..., r9 OR r10, ..., flg
    if (s_len > index) {
        switch (s[index]) {
            case 'u': r.type = 0; break;
            case 'i': r.type = 1; break;
            case 'f': r.type = 2; break;
            case 'b': r.size = 0; break;
            case 'w': r.size = 1; break;
            case 'd': r.size = 2; break;
            case 'q': r.size = 3; break;
            default:
                __DBG("parse_register: invalid type/size: %c\n", s[index]);
                return 1;
        }
    }
    index++;
    if (s_len > index) {
        if (r.size != 0xff) {
            __DBG("parse_register: cannot have any character after size: %s\n", s);
            return 1;
        }
        switch (s[index]) {
            case 'b': r.size = 0; break;
            case 'w': r.size = 1; break;
            case 'd': r.size = 2; break;
            case 'q': r.size = 3; break;
            default:
                __DBG("parse_register: invalid size: %c\n", s[index]);
                return 1;
        }
    }
    index++;
    if (s_len > index) {
        __DBG("parse_register: register identifier too long!: %s\n", s);
        return 1;
    }
    reg->id = r.id;
    if (r.type != 0xff) {
        reg->type = r.type;
    }
    if (r.size != 0xff) {
        reg->size = r.size;
    }
    return 0;
}

symbol_arg_t *init_symbol_arg(symbol_arg_t *s) {
    __MINIISA_INIT(s, symbol_arg_t);
    memset(s->name, '\0', MAX_TOKEN_LEN+1);
    return s;
}

size_t set_symbol_name(symbol_arg_t *s, char *name) {
    size_t name_len = strnlen(name, MAX_TOKEN_LEN+1);
    size_t copy_len = name_len <= MAX_TOKEN_LEN ? name_len : MAX_TOKEN_LEN;
    memcpy(s->name, name, copy_len);
    memset(&s->name[copy_len], '\0', MAX_TOKEN_LEN+1-copy_len);
    return name_len;
}

value_arg_t *init_value_arg(value_arg_t *v) {
    __MINIISA_INIT(v, value_arg_t);
    memset(v->value, 0, 8);
    v->type = UNKNOWN_TYPE;
    v->size = UNKNOWN_SIZE;
    return v;
}

argument_t *init_argument(argument_t *a) {
    __MINIISA_INIT(a, argument_t);
    a->kind = REGISTER_ARG;
    init_register_arg(&a->a.reg);
    return a;
}

instruction_t *init_instruction(instruction_t *i) {
    __MINIISA_INIT(i, instruction_t);
    i->mnemonic = INVALID_MNEMONIC;
    init_argument(&i->arg_1);
    init_argument(&i->arg_2);
    return i;
}

static int match_mnemonic(
    char *s,
    char *expectation,
    int accepts_type,
    int accepts_size,
    raw_mnemonic_t mnemonic,
    instruction_t *i
) {
    const size_t m_len = strlen(expectation);
    if (strncmp(s, expectation, m_len)) {
        size_t index = m_len - 1;
        data_type_t rtype = 0;
        data_size_t rsize = 0;
        if (accepts_type) {
            index++;
            char tchar = s[index];
            switch (tchar) {
                case '\0': case 'u':
                    rtype = 0; break;
                case 'i': rtype = 1; break;
                case 'f': rtype = 2; break;
                default:
                    __DBG("invalid type char: %c\n", tchar);
                    return 1;
            }
        }
        if (accepts_size) {
            index++;
            char schar = s[index];
            switch (schar) {
                case 'b': rsize = 0; break;
                case 'w': rsize = 1; break;
                case 'd': rsize = 2; break;
                case '\0': case 'q':
                    rsize = 3; break;
                default:
                    __DBG("invalid size char: %c\n", schar);
                    return 1;
            }
        }
        i->mnemonic = mnemonic;
        if (accepts_type) {
            i->arg_1.a.reg.type = rtype;
            i->arg_2.a.reg.type = rtype;
        }
        if (accepts_size) {
            i->arg_1.a.reg.size = rsize;
            i->arg_1.a.reg.size = rsize;
        }
        return 0;
    } else {
        return -1;
    }
}

#define _GET_OPCODE(s, m, ty, sz, op, i) { \
    status = match_mnemonic(s, m, ty, sz, op, i); \
    if (status >= 0) return status; \
}

int parse_instruction_mnemonic(char *s, instruction_t *i) {
    int status;

    // RESET REGISTER ARGUMENTS
    init_argument(&i->arg_1);
    init_argument(&i->arg_2);
    i->arg_1.kind = REGISTER_ARG;
    i->arg_2.kind = REGISTER_ARG;
    init_register_arg(&i->arg_1.a.reg);
    init_register_arg(&i->arg_2.a.reg);
    // END RESET REGISTER ARGUMENTS

    _GET_OPCODE(s, "nop", 0, 0, NOP_MNEMONIC, i);
    _GET_OPCODE(s, "add", 1, 1, ADD_MNEMONIC, i);
    _GET_OPCODE(s, "sub", 1, 1, SUB_MNEMONIC, i);
    _GET_OPCODE(s, "mul", 1, 1, MUL_MNEMONIC, i);
    _GET_OPCODE(s, "div", 1, 1, DIV_MNEMONIC, i);
    _GET_OPCODE(s, "and", 0, 1, AND_MNEMONIC, i);
    _GET_OPCODE(s, "ior", 0, 1, IOR_MNEMONIC, i);
    _GET_OPCODE(s, "xor", 0, 1, XOR_MNEMONIC, i);
    _GET_OPCODE(s, "not", 0, 1, NOT_MNEMONIC, i);
    _GET_OPCODE(s, "shl", 0, 1, SHL_MNEMONIC, i);
    _GET_OPCODE(s, "shr", 0, 1, SHR_MNEMONIC, i);
    _GET_OPCODE(s, "sal", 0, 1, SAL_MNEMONIC, i);
    _GET_OPCODE(s, "sar", 0, 1, SAR_MNEMONIC, i);
    _GET_OPCODE(s, "rol", 0, 1, ROL_MNEMONIC, i);
    _GET_OPCODE(s, "ror", 0, 1, ROR_MNEMONIC, i);
    _GET_OPCODE(s, "set", 1, 1, SET_MNEMONIC, i);
    _GET_OPCODE(s, "mov", 0, 1, MOV_MNEMONIC, i);
    _GET_OPCODE(s, "ldr", 0, 1, LDR_MNEMONIC, i);
    _GET_OPCODE(s, "str", 0, 1, STR_MNEMONIC, i);
    _GET_OPCODE(s, "psh", 0, 1, PSH_MNEMONIC, i);
    _GET_OPCODE(s, "pop", 0, 1, POP_MNEMONIC, i);
    _GET_OPCODE(s, "xcg", 0, 1, XCG_MNEMONIC, i);
    _GET_OPCODE(s, "cvt", 1, 1, CVT_MNEMONIC, i);
    _GET_OPCODE(s, "cmp", 1, 1, CMP_MNEMONIC, i);
    _GET_OPCODE(s, "jzr", 1, 1, JZR_MNEMONIC, i);
    _GET_OPCODE(s, "jnz", 1, 1, JNZ_MNEMONIC, i);
    _GET_OPCODE(s, "jeq", 0, 0, JEQ_MNEMONIC, i);
    _GET_OPCODE(s, "jne", 0, 0, JNE_MNEMONIC, i);
    _GET_OPCODE(s, "jlt", 0, 0, JLT_MNEMONIC, i);
    _GET_OPCODE(s, "jle", 0, 0, JLE_MNEMONIC, i);
    _GET_OPCODE(s, "jgt", 0, 0, JGT_MNEMONIC, i);
    _GET_OPCODE(s, "jge", 0, 0, JGE_MNEMONIC, i);
    _GET_OPCODE(s, "int", 0, 0, INT_MNEMONIC, i);
    _GET_OPCODE(s, "hlt", 0, 0, HLT_MNEMONIC, i);
    return -1;
}

#undef _GET_OPCODE


data_stmt_t *init_data_stmt(data_stmt_t *d) {
    __MINIISA_INIT(d, data_stmt_t);
    d->data = NULL;
    d->length = 0;
    d->size = 0;
    d->type = UNKNOWN_TYPE;
    return d;
}

int set_data(data_stmt_t *d, uint8_t *value, size_t length) {
    uint8_t *new = realloc(d->data, length);
    if (!new) return 1;
    d->data = new;
    memcpy(d->data, value, length);
    return 0;
}

symbol_stmt_t *init_symbol_stmt(symbol_stmt_t *s) {
    __MINIISA_INIT(s, symbol_stmt_t);
    memset(s->name, '\0', MAX_TOKEN_LEN+1);
    return s;
}

section_stmt_t *init_section_stmt(section_stmt_t *s) {
    __MINIISA_INIT(s, section_stmt_t);
    memset(s->name, '\0', MAX_TOKEN_LEN+1);
    return s;
}

statement_t *init_statement(statement_t *s) {
    __MINIISA_INIT(s, statement_t);
    s->kind = INSTRUCTION_STMT;
    init_instruction(&s->s.instruction);
    return s;
}

prebytecode_t *init_prebytecode(prebytecode_t *p) {
    __MINIISA_INIT(p, prebytecode_t);
    p->statements = NULL;
    p->stmts_count = 0;
    p->stmts_capacity = 0;
    return p;
}

int set_prebytecode_capacity(prebytecode_t *p, size_t new_cap) {
    if (new_cap < p->stmts_count) {
        new_cap = p->stmts_count;
    }
    if (new_cap == 0 && p->statements != NULL) {
        free(p->statements);
        p->statements = NULL;
        p->stmts_count = 0;
        p->stmts_capacity = 0;
    } else {
        statement_t *new = realloc(p->statements, new_cap);
        if (!new) return 1;
        p->statements = new;
        p->stmts_capacity = new_cap;
    }
    return 0;
}

int push_prebytecode_stmt(prebytecode_t *p, statement_t *s) {
    int status = 0;
    if (p->stmts_count >= p->stmts_capacity) {
        status = set_prebytecode_capacity(p, p->stmts_capacity+8);
        if (status) return status;
    }
    p->statements[p->stmts_count] = *s;
    p->stmts_count++;
    return status;
}
