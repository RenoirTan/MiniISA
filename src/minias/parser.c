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
            // TODO: create a new instruction
            miniisa_instruction_init(&p->instruction);
            // TODO: figure out which mnemonic this is
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

#define _RUN_PARSER_FN_SHORT_CIRCUIT(f, s, p, b) { \
    if (!(status = getting_initial(p, b))) { \
        return status; \
    } \
}

// TODO: FIND THE NEXT FUNCTION TO RUN IF p->need_new_token REMAINS FALSE
int parse_one_token(parser_t *p, token_t *t, miniisa_bytecode_t *b) {
    int status = 0;
    if (t->token_type == NEWLINE_TOKEN) {
        return 0;
    }
    if (p->state == PARSER_INIT || p->state == PARSER_DONE) {
        p->state = PARSER_GETTING_INITIAL;
    }
    if (p->need_new_token) {
        push_token(p, t);
        p->need_new_token = 0;
    }
    switch (p->state) {
    case PARSER_GETTING_INITIAL:
        _RUN_PARSER_FN_SHORT_CIRCUIT(getting_initial, status, p, b);
        break;
    case PARSER_DETECTING_TYPE:
        _RUN_PARSER_FN_SHORT_CIRCUIT(detecting_type, status, p, b);
        break;
    case PARSER_SETTING_DATA:
        set_prev_token(p, t);
        break;
    case PARSER_FINDING_ARGUMENT:
        set_prev_token(p, t);
        break;
    case PARSER_AWAITING_ARG_COMMA:
        set_prev_token(p, t);
        break;
    case PARSER_NEEDING_SECTION_NAME:
        _RUN_PARSER_FN_SHORT_CIRCUIT(needing_section_name, status, p, b);
        break;
    case PARSER_EXPECTING_COLON:
        if (t->token_type != COLON_TOKEN) {
            __DBG(
                "parse_one_token: PARSER_EXPECTING_COLON: colon not found: %s\n",
                t->span
            );
            return 2;
        }
        set_prev_token(p, t);
        break;
    default:
        __DBG("what the fuck is this parser state: %d\n", p->state);
        return 1;
    }
    return 0;
}

#undef _RUN_PARSER_FN_SHORT_CIRCUIT
