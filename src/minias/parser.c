#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <miniisa/bytecode.h>
#include <miniisa/extra.h>
#include <miniisa/instruction.h>
#include <miniisa/permissions.h>
#include <miniisa/section.h>
#include <miniisa/symbol.h>
#include "parser.h"
#include "token.h"

parser_t *init_parser(parser_t *p) {
    if (!p) {
        p = malloc(sizeof(parser_t));
        if (!p) return NULL;
    }
    init_token(&p->prev_token);
    init_token(&p->curr_token);
    p->need_new_token = 1;
    p->state = PARSER_INIT;
    miniisa_instruction_init(&p->instruction);
    memset(p->data, '\0', PARSER_DATA_MAX_LEN);
    p->data_len = 0;
    return p;
}

static void push_token(parser_t *p, token_t *t) {
    copy_token(&p->prev_token, &p->curr_token);
    if (t) {
        copy_token(&p->curr_token, t);
    } else {
        init_token(&p->curr_token);
    }
}

static int getting_initial(parser_t *p, miniisa_bytecode_t *b) {
    int status = 0;
    token_t *t = &p->curr_token;
    switch (t->token_type) {
    case IDENTIFIER_TOKEN:
        p->state = strcmp(t->span, "section") == 0
            ? PARSER_NEEDING_SECTION_NAME
            : PARSER_DETECTING_TYPE;
        p->need_new_token = 1;
        break;
    default:
        __DBG("getting_identifier: not identifier token: %s\n", t->span);
        p->need_new_token = 0;
        status = 1;
    }
    return status;
}

static int detecting_type(parser_t *p, miniisa_bytecode_t *b) {
    int status = 0;
    if (p->curr_token.token_type == COLON_TOKEN) {
        miniisa_bytecode_terminate_last_symbol(b);
        miniisa_symbol_t s;
        miniisa_symbol_init(&s);
        miniisa_symbol_set_name(&s, p->prev_token.span);
        miniisa_bytecode_new_symbol(b, &s);
        p->need_new_token = 1;
        p->state = PARSER_INIT;
    } else if (p->prev_token.token_type == IDENTIFIER_TOKEN) {
        token_t *t = &p->prev_token;
        if (strcmp(t->span, "data") == 0) {
            p->state = PARSER_SETTING_DATA;
        } else {
            miniisa_instruction_init(&p->instruction);
            status = miniisa_get_opcode(t->span, &p->instruction);
            if (status < 0) {
                __DBG("detecting_type: unknown mnemonic: %s\n", t->span);
                return status;
            } else if (status > 0) {
                __DBG("detecting_type: an error occurred parsing instruction: %s\n", t->span);
                return status;
            }
            p->state = PARSER_FINDING_ARGUMENT;
        }
        p->need_new_token = 0;
    } else {
        __DBG("detecting_type: what is this??? %s\n", p->prev_token.span);
        p->need_new_token = 0;
        status = 1;
    }
    return status;
}

static int setting_data(parser_t *p, miniisa_bytecode_t *b) {
    return 0;
}

static int _set_register(char *s, miniisa_register_t *dest, miniisa_register_t *def) {
    int status = 0;
    miniisa_register_t reg = *def;
    status = miniisa_get_register(s, &reg);
    if (status) return status;
    dest->id = reg.id;
    if (dest->type == 0xff || dest->type == reg.type) {
        dest->type = reg.type;
    } else {
        __DBG(
            "_set_register: conflicting register types: %hhu vs %hhu\n",
            dest->type,
            reg.type
        );
        return 1;
    }
    if (dest->size == 0xff || dest->size == reg.size) {
        dest->size = reg.size;
    } else {
        __DBG(
            "_set_register: conflicting register sizes: %hhu vs %hhu\n",
            dest->size,
            reg.size
        );
        return 1;
    }
    return status;
}

static int finding_argument(parser_t *p, miniisa_bytecode_t *b) {
    int status = 0;
    miniisa_ops_t opcode = p->instruction.opcode;
    miniisa_register_t def; // default register
    def.id = -1;
    def.type = 0; // unsigned integer
    def.size = 3; // qword (64-bit)

    switch (opcode) {
    // 0 arguments
    case MINIISA_OP_NOP: case MINIISA_OP_HLT: {
        token_t *t = &p->curr_token;
        // TODO: other terminating tokens like EOF
        if (t->token_type == NEWLINE_TOKEN) {
            p->need_new_token = 1;
            // TODO: add instruction to bytecode
            p->state = PARSER_INIT;
        } else {
            __DBG("finding_argument: invalid argument after nop/hlt: %s\n", t->span);
            return 1;
        }
        break;
    }
    // 1 register argument
    case MINIISA_OP_NOT:
    case MINIISA_OP_PSH: case MINIISA_OP_POP:
    case MINIISA_OP_JEQ: case MINIISA_OP_JNE:
    case MINIISA_OP_JLT: case MINIISA_OP_JLE:
    case MINIISA_OP_JGT: case MINIISA_OP_JGE: {
        token_t *t = &p->curr_token;
        if (t->token_type == IDENTIFIER_TOKEN) {
            status = _set_register(t->span, &p->instruction.reg_a, &def);
            if (status) return status;
            p->need_new_token = 1;
            p->state = PARSER_INIT; // TODO: EXPECT NEWLINE/TERMINATOR INSTEAD
        } else {
            __DBG("finding_argument: expected register, instead got this: %s\n", t->span);
            p->need_new_token = 0;
            return 1;
        }
        break;
    }
    // 2 register arguments
    case MINIISA_OP_ADD: case MINIISA_OP_SUB: case MINIISA_OP_MUL: case MINIISA_OP_DIV:
    case MINIISA_OP_AND: case MINIISA_OP_IOR: case MINIISA_OP_XOR:
    case MINIISA_OP_SHL: case MINIISA_OP_SHR: case MINIISA_OP_SAL: case MINIISA_OP_SAR:
    case MINIISA_OP_ROL: case MINIISA_OP_ROR:
    case MINIISA_OP_MOV: case MINIISA_OP_LDR: case MINIISA_OP_STR: case MINIISA_OP_XCG:
    case MINIISA_OP_CMP: case MINIISA_OP_JZR: case MINIISA_OP_JNZ: {
        size_t curr_reg = p->instruction.reg_a.id == -1 ? 0 : 1;
        token_t *t = &p->curr_token;
        if (t->token_type == IDENTIFIER_TOKEN) {
            miniisa_register_t *dest = curr_reg ? &p->instruction.reg_a : &p->instruction.reg_b;
            status = _set_register(t->span, dest, &def);
            if (status) return status;
            p->need_new_token = 1;
            if (curr_reg == 0) {
                p->state = PARSER_AWAITING_ARG_COMMA;
            } else {
                p->state = PARSER_INIT; // TODO: EXPECT NEWLINE/TERMINATOR INSTEAD
            }
        } else {
            __DBG("finding_argument: expected register, instead got this: %s\n", t->span);
            p->need_new_token = 0;
            return 1;
        }
        break;
    }
    // set mnemonic
    case MINIISA_OP_SET:
        break;
    // cvt mnemonic
    case MINIISA_OP_CVT:
        break;
    // int mnemonic
    case MINIISA_OP_INT:
        break;
    case MINIISA_OP_INVALID: default:
        __DBG("finding_argument: invalid opcode %d\n", opcode);
    }
    return status;
}

static int awaiting_arg_comma(parser_t *p, miniisa_bytecode_t *b) {
    return 0;
}

static int needing_section_name(parser_t *p, miniisa_bytecode_t *b) {
    int status = 0;
    token_t *t = &p->curr_token;
    if (t->token_type == IDENTIFIER_TOKEN) {
        miniisa_bytecode_terminate_last_section(b);
        miniisa_section_t s;
        miniisa_section_init(&s);
        miniisa_section_set_name(&s, t->span);
        miniisa_bytecode_new_section(b, &s);
        p->need_new_token = 1;
    } else {
        __DBG("needing_section_name: not identifier token: %s\n", t->span);
        p->need_new_token = 0;
    }
    return status;
}

static int expecting_colon(parser_t *p, miniisa_bytecode_t *b) {
    return 0;
}

static int demanding_size_comma(parser_t *p, miniisa_bytecode_t *b) {
    return 0;
}

static int requiring_size(parser_t *p, miniisa_bytecode_t *b) {
    return 0;
}

#define _RUN_PARSER_FN_SHORT_CIRCUIT(f, s, p, b) { \
    if (!(status = f(p, b))) { \
        return status; \
    } \
}

// TODO: FIND THE NEXT FUNCTION TO RUN IF p->need_new_token REMAINS FALSE
int parse_one_token(parser_t *p, token_t *t, miniisa_bytecode_t *b) {
    int status = 0;
    if (p->state == PARSER_INIT || p->state == PARSER_DONE) {
        p->state = PARSER_GETTING_INITIAL;
    }
    if (p->need_new_token) {
        push_token(p, t);
        p->need_new_token = 0;
    }
    while (!(p->need_new_token)) {
        switch (p->state) {
            case PARSER_GETTING_INITIAL:
                _RUN_PARSER_FN_SHORT_CIRCUIT(getting_initial, status, p, b);
                break;
            case PARSER_DETECTING_TYPE:
                _RUN_PARSER_FN_SHORT_CIRCUIT(detecting_type, status, p, b);
                break;
            case PARSER_SETTING_DATA:
                _RUN_PARSER_FN_SHORT_CIRCUIT(setting_data, status, p, b);
                break;
            case PARSER_FINDING_ARGUMENT:
                _RUN_PARSER_FN_SHORT_CIRCUIT(finding_argument, status, p, b);
                break;
            case PARSER_AWAITING_ARG_COMMA:
                _RUN_PARSER_FN_SHORT_CIRCUIT(awaiting_arg_comma, status, p, b);
                break;
            case PARSER_NEEDING_SECTION_NAME:
                _RUN_PARSER_FN_SHORT_CIRCUIT(needing_section_name, status, p, b);
                break;
            case PARSER_EXPECTING_COLON:
                _RUN_PARSER_FN_SHORT_CIRCUIT(expecting_colon, status, p, b);
                break;
            case PARSER_DEMANDING_SIZE_COMMA:
                _RUN_PARSER_FN_SHORT_CIRCUIT(demanding_size_comma, status, p, b);
                break;
            case PARSER_REQUIRING_SIZE:
                _RUN_PARSER_FN_SHORT_CIRCUIT(requiring_size, status, p, b);
                break;
            default:
                __DBG("what the fuck is this parser state: %d\n", p->state);
                return 1;
        }
    }
    return 0;
}

#undef _RUN_PARSER_FN_SHORT_CIRCUIT
