#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <miniisa/extra.h>
#include "parser.h"
#include "prebytecode.h"
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
    init_statement(&p->stmt);
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

static int getting_initial(parser_t *p, prebytecode_t *b) {
    int status = 0;
    token_t *t = &p->curr_token;
    init_statement(&p->stmt);
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

static int detecting_type(parser_t *p, prebytecode_t *b) {
    int status = 0;
    if (p->curr_token.token_type == COLON_TOKEN) {
        p->stmt.kind = SYMBOL_STMT;
        init_symbol_stmt(&p->stmt.s.symbol);
        strncpy(p->stmt.s.symbol.name, p->prev_token.span, MAX_TOKEN_LEN+1);
        p->stmt.s.symbol.name[MAX_TOKEN_LEN] = '\0';
        push_prebytecode_stmt(b, &p->stmt);
        p->need_new_token = 1;
        p->state = PARSER_INIT;
    } else if (p->prev_token.token_type == IDENTIFIER_TOKEN) {
        token_t *t = &p->prev_token;
        if (strcmp(t->span, "data") == 0) {
            p->stmt.kind = DATA_STMT;
            init_data_stmt(&p->stmt.s.data);
            p->state = PARSER_SETTING_DATA;
        } else {
            p->stmt.kind = INSTRUCTION_STMT;
            init_instruction(&p->stmt.s.instruction);
            status = parse_instruction_mnemonic(t->span, &p->stmt.s.instruction);
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

static int setting_data(parser_t *p, prebytecode_t *b) {
    return 0;
}

static int _set_register(char *s, register_arg_t *dest, register_arg_t *def) {
    int status = 0;
    register_arg_t reg = *def;
    status = parse_register(s, &reg);
    if (status) return status;
    dest->id = reg.id;
    if (dest->type == 0xff || dest->type == reg.type) {
        dest->type = reg.type;
    } else {
        __DBG(
            "_set_register: conflicting register types: %u vs %u\n",
            dest->type,
            reg.type
        );
        return 1;
    }
    if (dest->size == 0xff || dest->size == reg.size) {
        dest->size = reg.size;
    } else {
        __DBG(
            "_set_register: conflicting register sizes: %u vs %u\n",
            dest->size,
            reg.size
        );
        return 1;
    }
    return status;
}

static int finding_argument(parser_t *p, prebytecode_t *b) {
    int status = 0;
    raw_mnemonic_t opcode = p->stmt.s.instruction.mnemonic;
    register_arg_t def; // default register
    def.id = UNKNOWN_REG;
    def.type = 0; // unsigned integer
    def.size = 3; // qword (64-bit)

    switch (opcode) {
    // 0 arguments
    case NOP_MNEMONIC: case HLT_MNEMONIC: {
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
    case NOT_MNEMONIC:
    case PSH_MNEMONIC: case POP_MNEMONIC:
    case JEQ_MNEMONIC: case JNE_MNEMONIC:
    case JLT_MNEMONIC: case JLE_MNEMONIC:
    case JGT_MNEMONIC: case JGE_MNEMONIC: {
        token_t *t = &p->curr_token;
        if (t->token_type == IDENTIFIER_TOKEN) {
            status = _set_register(t->span, &p->stmt.s.instruction.arg_1.a.reg, &def);
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
    case ADD_MNEMONIC: case SUB_MNEMONIC: case MUL_MNEMONIC: case DIV_MNEMONIC:
    case AND_MNEMONIC: case IOR_MNEMONIC: case XOR_MNEMONIC:
    case SHL_MNEMONIC: case SHR_MNEMONIC: case SAL_MNEMONIC: case SAR_MNEMONIC:
    case ROL_MNEMONIC: case ROR_MNEMONIC:
    case MOV_MNEMONIC: case LDR_MNEMONIC: case STR_MNEMONIC: case XCG_MNEMONIC:
    case CMP_MNEMONIC: case JZR_MNEMONIC: case JNZ_MNEMONIC: {
        size_t curr_reg = p->stmt.s.instruction.arg_1.a.reg.id == -1 ? 0 : 1;
        token_t *t = &p->curr_token;
        if (t->token_type == IDENTIFIER_TOKEN) {
            register_arg_t *dest = curr_reg
                ? &p->stmt.s.instruction.arg_1.a.reg
                : &p->stmt.s.instruction.arg_2.a.reg;
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
    case SET_MNEMONIC:
        break;
    // cvt mnemonic
    case CVT_MNEMONIC:
        break;
    // int mnemonic
    case INT_MNEMONIC:
        break;
    case INVALID_MNEMONIC: default:
        __DBG("finding_argument: invalid opcode %d\n", opcode);
    }
    return status;
}

static int awaiting_arg_comma(parser_t *p, prebytecode_t *b) {
    return 0;
}

static int needing_section_name(parser_t *p, prebytecode_t *b) {
    int status = 0;
    token_t *t = &p->curr_token;
    if (t->token_type == IDENTIFIER_TOKEN) {
        init_section_stmt(&p->stmt.s.section);
        strncpy(p->stmt.s.section.name, t->span, MAX_TOKEN_LEN+1);
        p->stmt.s.section.name[MAX_TOKEN_LEN] = '\0';
        push_prebytecode_stmt(b, &p->stmt);
        p->need_new_token = 1;
        p->state = PARSER_INIT;
    } else {
        __DBG("needing_section_name: not identifier token: %s\n", t->span);
        p->need_new_token = 0;
    }
    return status;
}

static int expecting_colon(parser_t *p, prebytecode_t *b) {
    return 0;
}

static int demanding_size_comma(parser_t *p, prebytecode_t *b) {
    return 0;
}

static int requiring_size(parser_t *p, prebytecode_t *b) {
    return 0;
}

#define _RUN_PARSER_FN_SHORT_CIRCUIT(f, s, p, b) { \
    if (!(status = f(p, b))) { \
        return status; \
    } \
}

// TODO: FIND THE NEXT FUNCTION TO RUN IF p->need_new_token REMAINS FALSE
int parse_one_token(parser_t *p, token_t *t, prebytecode_t *b) {
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
