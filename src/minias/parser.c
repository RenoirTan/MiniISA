#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <miniisa/data.h>
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

    // reset statement
    init_statement(&p->stmt);

    switch (t->token_type) {
    case IDENTIFIER_TOKEN:
        // determine if this line is a section derivative or something else
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

    // if there is a colon ':' after the previous token, then this is a symbol
    if (p->curr_token.token_type == COLON_TOKEN) {
        // set statement to symbol
        p->stmt.kind = SYMBOL_STMT;
        init_symbol_stmt(&p->stmt.s.symbol);

        // set name of symbol
        strncpy(p->stmt.s.symbol.name, p->prev_token.span, MAX_TOKEN_LEN+1);
        p->stmt.s.symbol.name[MAX_TOKEN_LEN] = '\0'; // null-character
        
        // add this statement to prebytecode_t *b
        push_prebytecode_stmt(b, &p->stmt);

        // go back to start
        p->need_new_token = 1;
        p->state = PARSER_INIT;
    } else if (p->prev_token.token_type == IDENTIFIER_TOKEN) {
        // either an instruction or data statement
        token_t *t = &p->prev_token;
        if (strcmp(t->span, "data") == 0) {
            // set DATA_STMT
            p->stmt.kind = DATA_STMT;
            init_data_stmt(&p->stmt.s.data);
            p->state = PARSER_SETTING_DATA;
        } else {
            // set INSTRUCTION_STMT, and detect mnemonic
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

        // set this to false because the p->curr_token hasn't been parsed yet
        p->need_new_token = 0;
    } else {
        __DBG("detecting_type: what is this??? %s\n", p->prev_token.span);
        p->need_new_token = 0;
        status = 1;
    }
    return status;
}

static int setting_data(parser_t *p, prebytecode_t *b) {
    int status = 0;
    token_t *t = &p->curr_token;
    if (t->token_type == INT_TOKEN) {
        uint8_t *data = malloc(8);
        if (!data) {
            __DBG("setting_data: could not malloc(8) for int\n");
            return 1;
        }
        memset(data, '\0', 8);
        size_t width = 0;
        status = miniisa_str_to_le_int(t->span, data, &width);
        if (status) return status;
        data_stmt_t *ds = &p->stmt.s.data;
        ds->data = data;
        ds->length = width;
        ds->type = UNSIGNED_INT_TYPE; // TODO: signed int
    } else if (t->token_type == FLOAT_TOKEN) {
        // TODO: accept single-precision f32 floats as well
        uint8_t *data = malloc(8);
        if (!data) {
            __DBG("setting_data: could not malloc(8) for float\n");
            return 1;
        }
        memset(data, '\0', 8);
        status = miniisa_str_to_float_bytes(t->span, data);
        if (status) return status;
        data_stmt_t *ds = &p->stmt.s.data;
        ds->data = data;
        ds->length = 8;
        ds->type = FLOAT_TYPE;
    } else {
        __DBG("setting_data: invalid data type: %s\n", t->span);
        p->need_new_token = 0;
        return 1;
    }
    p->need_new_token = 1;
    p->state = PARSER_DEMANDING_SIZE_COMMA;
    return status;
}

/**
 * \brief Parse `s`, detecting which register `s` is referring to and store it
 *        in `dest`, with `def` as the default register, data type or size.
 *
 * If the register `s` has a data type or size that conflicts with that in
 * `dest`, a non-zero error code is returned.
 */
static int _set_register(char *s, register_arg_t *dest, register_arg_t *def) {
    int status = 0;
    register_arg_t reg = *def;
    status = parse_register(s, &reg);
    if (status) return status;
    dest->id = reg.id;
    if (dest->type == UNKNOWN_TYPE || dest->type == reg.type) {
        dest->type = reg.type;
    } else {
        __DBG(
            "_set_register: conflicting register types: %u vs %u\n",
            dest->type,
            reg.type
        );
        return 1;
    }
    if (dest->size == UNKNOWN_SIZE || dest->size == reg.size) {
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
    raw_mnemonic_t mnemonic = p->stmt.s.instruction.mnemonic;

    // default register is a uint64_t
    register_arg_t def; // default register
    def.id = UNKNOWN_REG;
    def.type = 0; // unsigned integer
    def.size = 3; // qword (64-bit)

    switch (mnemonic) {
    // 0 arguments
    // these mnemonics expect no arguments after it,
    // so it expects terminating tokens like newline or eof after the mnemonic
    case NOP_MNEMONIC: case HLT_MNEMONIC: {
        p->need_new_token = 0;
        p->state = PARSER_ANTICIPATING_TERMINATING;
        break;
    }
    // 1 register argument
    // expect one register argument, then expect newline/eof
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
            p->state = PARSER_ANTICIPATING_TERMINATING;
        } else {
            __DBG("finding_argument: expected register, instead got this: %s\n", t->span);
            p->need_new_token = 0;
            return 1;
        }
        break;
    }
    // 2 register arguments
    // check if arg_1 has been set, if not, parse for arg_1, then expect comma
    // otherwise parse for arg_2, then expect terminator
    case ADD_MNEMONIC: case SUB_MNEMONIC: case MUL_MNEMONIC: case DIV_MNEMONIC:
    case AND_MNEMONIC: case IOR_MNEMONIC: case XOR_MNEMONIC:
    case SHL_MNEMONIC: case SHR_MNEMONIC: case SAL_MNEMONIC: case SAR_MNEMONIC:
    case ROL_MNEMONIC: case ROR_MNEMONIC:
    case MOV_MNEMONIC: case LDR_MNEMONIC: case STR_MNEMONIC: case XCG_MNEMONIC:
    case CMP_MNEMONIC: case JZR_MNEMONIC: case JNZ_MNEMONIC: {
        // check if arg_1 has been set
        size_t curr_reg = p->stmt.s.instruction.arg_1.a.reg.id == UNKNOWN_REG ? 0 : 1;
        token_t *t = &p->curr_token;
        if (t->token_type == IDENTIFIER_TOKEN) {
            register_arg_t *dest = curr_reg
                ? &p->stmt.s.instruction.arg_1.a.reg  // arg_1 not set yet
                : &p->stmt.s.instruction.arg_2.a.reg; // arg_2 not set yet
            status = _set_register(t->span, dest, &def); // parse register
            if (status) return status;
            p->need_new_token = 1;
            if (curr_reg == 0) {
                p->state = PARSER_AWAITING_ARG_COMMA; // need arg_2 now
            } else {
                p->state = PARSER_ANTICIPATING_TERMINATING; // arg_2 done
            }
        } else {
            __DBG("finding_argument: expected register, instead got this: %s\n", t->span);
            p->need_new_token = 0;
            return 1;
        }
        break;
    }
    // set mnemonic
    case SET_MNEMONIC: {
        token_t *t = &p->curr_token;
        register_arg_t *arg_1 = &p->stmt.s.instruction.arg_1.a.reg;
        if (arg_1->id == UNKNOWN_REG) {
            if (t->token_type != IDENTIFIER_TOKEN) {
                __DBG(
                    "finding_argument: expected register after set, but got: %s\n",
                    t->span
                );
                return 1;
            }
            status = _set_register(t->span, arg_1, &def);
            if (status) {
                __DBG("finding_argument: bad register: %s\n", t->span);
                return 1;
            }
            p->state = PARSER_AWAITING_ARG_COMMA;
        } else {
            if (t->token_type == INT_TOKEN) {
                if (arg_1->type == UNSIGNED_INT_TYPE || arg_1->type == SIGNED_INT_TYPE) {
                    // skip
                } else if (arg_1->type == UNKNOWN_TYPE) {
                    arg_1->type = SIGNED_INT_TYPE; // TODO: unsigned int
                } else {
                    __DBG("finding_argument: value of set instruction is int!\n");
                    return 1;
                }
                value_arg_t *arg_2 = &p->stmt.s.instruction.arg_2.a.val;
                init_value_arg(arg_2);
                p->stmt.s.instruction.arg_2.kind = VALUE_ARG;
                size_t w = 0;
                status = miniisa_str_to_le_int(t->span, arg_2->value, &w);
                if (status) {
                    __DBG("finding_argument: could not parse integer: %s\n", t->span);
                    return 1;
                }
                if (arg_1->size == BYTE_SIZE && w <= 1) {
                    arg_2->size = 1;
                } else if (arg_1->size == WORD_SIZE && w <= 2) {
                    arg_2->size = 2;
                } else if (arg_1->size == DWORD_SIZE && w <= 4) {
                    arg_2->size = 4;
                } else if (arg_1->size == QWORD_SIZE && w <= 8) {
                    arg_2->size = 8;
                } else if (arg_1->size == UNKNOWN_SIZE) {
                    arg_1->size = QWORD_SIZE;
                    arg_2->size = 8;
                } else {
                    // TODO: show number of bytes arg_1 accepts
                    __DBG("finding_argument: parsed int (%s) has %lu bytes\n", t->span, w);
                    return 1;
                }
                arg_2->type = arg_1->type;
            } else if (t->token_type == FLOAT_TOKEN) {
                if (arg_1->type == FLOAT_TOKEN) {
                    // skip
                } else if (arg_1->type == UNKNOWN_TOKEN) {
                    arg_1->type = FLOAT_TYPE;
                } else {
                    __DBG("finding_argument: value of set instruction is float!\n");
                    return 1;
                }
                value_arg_t *arg_2 = &p->stmt.s.instruction.arg_2.a.val;
                init_value_arg(arg_2);
                p->stmt.s.instruction.arg_2.kind = VALUE_ARG;
                status = miniisa_str_to_float_bytes(t->span, arg_2->value);
                if (status) {
                    __DBG("finding_argument: could not parse float: %s\n", t->span);
                }
                arg_2->type = FLOAT_TYPE;
                if (arg_1->size == QWORD_SIZE || arg_1->size == UNKNOWN_SIZE) {
                    arg_1->size = QWORD_SIZE;
                    arg_2->size = QWORD_SIZE;
                } else if (arg_1->size == DWORD_SIZE) {
                    miniisa_double_to_float_bytes(arg_2->value);
                    arg_2->size = DWORD_SIZE;
                } else {
                    __DBG("finding_argument: invalid size for float: %d\n", arg_1->size);
                    return 1;
                }
            } else if (t->token_type == IDENTIFIER_TOKEN) {
                symbol_arg_t *arg_2 = &p->stmt.s.instruction.arg_2.a.sym;
                init_symbol_arg(arg_2);
                p->stmt.s.instruction.arg_2.kind = SYMBOL_ARG;
                set_symbol_name(arg_2, t->span);
            } else {
                __DBG("finding_argument: unexpected token in set: %s\n", t->span);
                return 1;
            }
            p->state = PARSER_ANTICIPATING_TERMINATING;
        }
        p->need_new_token = 1;
        break;
    }
    // cvt mnemonic
    case CVT_MNEMONIC: {
        token_t *t = &p->curr_token;
        register_arg_t reg;
        init_register_arg(&reg);
        status = parse_register(t->span, &reg);
        p->stmt.s.instruction.arg_1.a.reg.id = reg.id; // dest
        p->stmt.s.instruction.arg_2.a.reg = reg; // src
        p->need_new_token = 1;
        p->state = PARSER_ANTICIPATING_TERMINATING;
        break;
    }
    // int mnemonic
    case INT_MNEMONIC:
        break;
    case INVALID_MNEMONIC: default:
        __DBG("finding_argument: invalid opcode %d\n", mnemonic);
    }
    return status;
}

static int awaiting_arg_comma(parser_t *p, prebytecode_t *b) {
    int status = 0;
    token_t *t = &p->curr_token;
    if (t->token_type == COMMA_TOKEN) {
        p->need_new_token = 1;
        p->state = PARSER_FINDING_ARGUMENT;
    } else {
        __DBG("awaiting_arg_comma: got: %s\n", t->span);
        return 1;
    }
    return status;
}

static int needing_section_name(parser_t *p, prebytecode_t *b) {
    int status = 0;
    token_t *t = &p->curr_token;
    if (t->token_type == IDENTIFIER_TOKEN) {
        // init section statement
        init_section_stmt(&p->stmt.s.section);
        // set section name
        strncpy(p->stmt.s.section.name, t->span, MAX_TOKEN_LEN+1);
        p->stmt.s.section.name[MAX_TOKEN_LEN] = '\0';
        // add statement to prebytecode_t *b
        push_prebytecode_stmt(b, &p->stmt);
        p->need_new_token = 1;
        p->state = PARSER_ANTICIPATING_TERMINATING;
    } else {
        __DBG("needing_section_name: not identifier token: %s\n", t->span);
        p->need_new_token = 0;
    }
    return status;
}

static int expecting_colon(parser_t *p, prebytecode_t *b) {
    int status = 0;
    token_t *t = &p->curr_token;
    if (t->token_type == COLON_TOKEN) {
        p->need_new_token = 1;
        p->state = PARSER_FINDING_ARGUMENT;
    } else {
        __DBG("expecting_colon: got: %s\n", t->span);
        return 1;
    }
    return status;
}

static int demanding_size_comma(parser_t *p, prebytecode_t *b) {
    int status = 0;
    token_t *t = &p->curr_token;
    if (t->token_type == COMMA_TOKEN) {
        p->need_new_token = 1;
        p->state = PARSER_REQUIRING_SIZE;
    } else {
        __DBG("demanding_size_comma: got: %s\n", t->span);
        return 1;
    }
    return status;
}

static int requiring_size(parser_t *p, prebytecode_t *b) {
    int status = 0;
    token_t *t = &p->curr_token;
    if (t->token_type == INT_TOKEN) {
        uint64_t size = 0;
        status = miniisa_str_to_uint64(t->span, &size);
        if (status) {
            __DBG("requiring_size: could not parse uint64: %s\n", t->span);
            return 1;
        }
        data_stmt_t *ds = &p->stmt.s.data;
        if (ds->type == UNSIGNED_INT_TYPE || ds->type == SIGNED_INT_TYPE) {
            if (size != 1 && size != 2 && size != 4 && size != 8) {
                __DBG("requiring_size: invalid size for integer: %lu\n", size);
                return 1;
            }
            if (size < ds->length) {
                __DBG(
                    "requiring_size: size explicitly given is smaller than "
                    "width of integer: %lu vs %lu\n",
                    size,
                    ds->length
                );
                return 1;
            }
            ds->size = size;
        } else if (ds->type == FLOAT_TYPE) {
            if (size == 4) {
                status = miniisa_double_to_float_bytes(ds->data);
                if (status) return status;
            } else if (size != 8) {
                __DBG("requiring_size: invalid size for float: %lu\n", size);
                return 1;
            }
            ds->size = size;
        }
        p->need_new_token = 1;
        p->state = PARSER_ANTICIPATING_TERMINATING;
    } else {
        __DBG("requiring_size: got %s\n", t->span);
        return 1;
    }
    return status;
}

static int anticipating_terminating(parser_t *p, prebytecode_t *b) {
    int status = 0;
    token_t *t = &p->curr_token;
    switch (t->token_type) {
    case NEWLINE_TOKEN: case EOF_TOKEN: { // include EOFs as well
        p->need_new_token = 1;
        p->state = PARSER_DONE;
        break;
    }
    default:
        __DBG("anticipating_terminating: got '%s'\n", t->span);
        return 1;
    }
    return status;
}

#define _RUN_PARSER_FN_SHORT_CIRCUIT(f, s, p, b) { \
    if (!(status = f(p, b))) { \
        return status; \
    } \
}

// TODO: FIND THE NEXT FUNCTION TO RUN IF p->need_new_token REMAINS FALSE
int parse_one_token(parser_t *p, token_t *t, prebytecode_t *b) {
    int status = 0;
    // go to start of flowchart
    if (p->state == PARSER_INIT || p->state == PARSER_DONE) {
        p->state = PARSER_GETTING_INITIAL;
    }

    // set p->curr_token to *t
    push_token(p, t);
    p->need_new_token = 0;

    // traverse the flowchart until a new token is needed
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
            case PARSER_ANTICIPATING_TERMINATING:
                _RUN_PARSER_FN_SHORT_CIRCUIT(anticipating_terminating, status, p, b);
                break;
            default:
                __DBG("what the fuck is this parser state: %d\n", p->state);
                return 1;
        }
    }
    return 0;
}

#undef _RUN_PARSER_FN_SHORT_CIRCUIT
