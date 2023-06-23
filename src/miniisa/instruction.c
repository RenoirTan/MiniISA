#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <miniisa/extra.h>
#include <miniisa/instruction.h>

void reset_register(miniisa_register_t *r) {
    r->id = -1;
    r->type = 0xff;
    r->size = 0xff;
}

miniisa_register_id_t miniisa_get_register_id(char *s) {
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

int miniisa_get_register(char *s, miniisa_register_t *reg) {
    size_t s_len = strlen(s);
    miniisa_register_t r;
    reset_register(&r);
    r.type = 0xff;
    r.size = 0xff;
    r.id = miniisa_get_register_id(s);
    if (r.id < 0) {
        __DBG("miniisa_get_register: invalid register: %s\n", s);
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
                __DBG("miniisa_get_register: invalid type/size: %c\n", s[index]);
                return 1;
        }
    }
    index++;
    if (s_len > index) {
        if (r.size != 0xff) {
            __DBG("miniisa_get_register: cannot have any character after size: %s\n", s);
            return 1;
        }
        switch (s[index]) {
            case 'b': r.size = 0; break;
            case 'w': r.size = 1; break;
            case 'd': r.size = 2; break;
            case 'q': r.size = 3; break;
            default:
                __DBG("miniisa_get_register: invalid size: %c\n", s[index]);
                return 1;
        }
    }
    index++;
    if (s_len > index) {
        __DBG("miniisa_get_register: register identifier too long!: %s\n", s);
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

static int match_mnemonic(
    char *s,
    char *mnemonic,
    int accepts_type,
    int accepts_size,
    miniisa_ops_t opcode,
    miniisa_instruction_t *i
) {
    const size_t m_len = strlen(mnemonic);
    if (strncmp(s, mnemonic, m_len)) {
        size_t index = m_len - 1;
        miniisa_register_type_t rtype = 0;
        miniisa_register_size_t rsize = 0;
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
        i->opcode = opcode;
        if (accepts_type) {
            i->reg_a.type = rtype;
            i->reg_b.type = rtype;
        }
        if (accepts_size) {
            i->reg_a.size = rsize;
            i->reg_b.size = rsize;
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

int miniisa_get_opcode(char *s, miniisa_instruction_t *i) {
    int status;
    _GET_OPCODE(s, "nop", 0, 0, MINIISA_OP_NOP, i);
    _GET_OPCODE(s, "add", 1, 1, MINIISA_OP_ADD, i);
    _GET_OPCODE(s, "sub", 1, 1, MINIISA_OP_SUB, i);
    _GET_OPCODE(s, "mul", 1, 1, MINIISA_OP_MUL, i);
    _GET_OPCODE(s, "div", 1, 1, MINIISA_OP_DIV, i);
    _GET_OPCODE(s, "and", 0, 1, MINIISA_OP_AND, i);
    _GET_OPCODE(s, "ior", 0, 1, MINIISA_OP_IOR, i);
    _GET_OPCODE(s, "xor", 0, 1, MINIISA_OP_XOR, i);
    _GET_OPCODE(s, "not", 0, 1, MINIISA_OP_NOT, i);
    _GET_OPCODE(s, "shl", 0, 1, MINIISA_OP_SHL, i);
    _GET_OPCODE(s, "shr", 0, 1, MINIISA_OP_SHR, i);
    _GET_OPCODE(s, "sal", 0, 1, MINIISA_OP_SAL, i);
    _GET_OPCODE(s, "sar", 0, 1, MINIISA_OP_SAR, i);
    _GET_OPCODE(s, "rol", 0, 1, MINIISA_OP_ROL, i);
    _GET_OPCODE(s, "ror", 0, 1, MINIISA_OP_ROR, i);
    _GET_OPCODE(s, "set", 1, 1, MINIISA_OP_SET, i);
    _GET_OPCODE(s, "mov", 0, 1, MINIISA_OP_MOV, i);
    _GET_OPCODE(s, "ldr", 0, 1, MINIISA_OP_LDR, i);
    _GET_OPCODE(s, "str", 0, 1, MINIISA_OP_STR, i);
    _GET_OPCODE(s, "psh", 0, 1, MINIISA_OP_PSH, i);
    _GET_OPCODE(s, "pop", 0, 1, MINIISA_OP_POP, i);
    _GET_OPCODE(s, "xcg", 0, 1, MINIISA_OP_XCG, i);
    _GET_OPCODE(s, "cvt", 1, 1, MINIISA_OP_CVT, i);
    _GET_OPCODE(s, "cmp", 1, 1, MINIISA_OP_CMP, i);
    _GET_OPCODE(s, "jzr", 1, 1, MINIISA_OP_JZR, i);
    _GET_OPCODE(s, "jnz", 1, 1, MINIISA_OP_JNZ, i);
    _GET_OPCODE(s, "jeq", 0, 0, MINIISA_OP_JEQ, i);
    _GET_OPCODE(s, "jne", 0, 0, MINIISA_OP_JNE, i);
    _GET_OPCODE(s, "jlt", 0, 0, MINIISA_OP_JLT, i);
    _GET_OPCODE(s, "jle", 0, 0, MINIISA_OP_JLE, i);
    _GET_OPCODE(s, "jgt", 0, 0, MINIISA_OP_JGT, i);
    _GET_OPCODE(s, "jge", 0, 0, MINIISA_OP_JGE, i);
    _GET_OPCODE(s, "int", 0, 0, MINIISA_OP_INT, i);
    _GET_OPCODE(s, "hlt", 0, 0, MINIISA_OP_HLT, i);
    return -1;
}

#undef _GET_OPCODE

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
