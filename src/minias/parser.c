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
    p->state = PARSER_INIT;
    miniisa_instruction_init(&p->instruction);
    memset(p->data, '\0', PARSER_DATA_MAX_LEN);
    p->data_len = 0;
    return p;
}

static void set_prev_token(parser_t *p, token_t *t) {
    copy_token(&p->prev_token, t);
}

static int getting_initial(parser_t *p, token_t *t, miniisa_bytecode_t *b) {
    int status = 0;
    switch (t->token_type) {
    case IDENTIFIER_TOKEN:
        p->state = strcmp(t->span, "section") == 0
            ? PARSER_NEEDING_SECTION_NAME
            : PARSER_DETECTING_TYPE;
        break;
    default:
        __DBG("getting_identifier: not identifier token: %s\n", t->span);
        status = 1;
    }
    set_prev_token(p, t);
    return status;
}

static int detecting_type(parser_t *p, token_t *t, miniisa_bytecode_t *b) {
    int status = 0;
    switch (t->token_type) {
    case COLON_TOKEN: { // symbol
        miniisa_bytecode_terminate_last_symbol(b);
        miniisa_symbol_t s;
        miniisa_symbol_init(&s);
        miniisa_symbol_set_name(&s, p->prev_token.span);
        miniisa_bytecode_new_symbol(b, &s);
        break;
    }
    default:
        __DBG("detecting_type: unrecognized token: %s\n", t->span);
        status = 1;
    }
    set_prev_token(p, t);
    return status;
}

int parse_one_token(parser_t *p, token_t *t, miniisa_bytecode_t *b) {
    int status = 0;
    if (t->token_type == NEWLINE_TOKEN) {
        return 0;
    }
    if (p->state == PARSER_INIT || p->state == PARSER_DONE) {
        p->state = PARSER_GETTING_INITIAL;
    }
    switch (p->state) {
    case PARSER_GETTING_INITIAL:
        if (!(status = getting_initial(p, t, b))) {
            return status;
        }
        break;
    case PARSER_DETECTING_TYPE:
        if (t->token_type == COLON_TOKEN) {
            // TODO: new symbol
            miniisa_symbol_t *s = miniisa_symbol_init(NULL);
            if (!s) {
                __DBG(
                    "parse_one_token: PARSER_DETECTING_TYPE:"
                    "could not allocate memory space for symbol\n"
                );
                return 2;
            }
            if (!miniisa_bytecode_terminate_last_symbol(b)) return 2;
            s->start = b->bytes_count;
            if (!miniisa_bytecode_new_symbol(b, s)) return 2;
            p->state = PARSER_DONE;
        } else if (t->token_type == IDENTIFIER_TOKEN) {
            // TODO: instruction
            p->state = PARSER_FINDING_ARGUMENT;
        } else if (t->token_type == FLOAT_TOKEN || t->token_type == INT_TOKEN) {
            // TODO: data
            p->state = PARSER_SETTING_DATA;
        }
        set_prev_token(p, t);
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
        if (t->token_type != IDENTIFIER_TOKEN) {
            __DBG(
                "parse_one_token: PARSER_NEEDING_SECTION_NAME: "
                "invalid token after 'section' keyword: %s\n",
                t->span
            );
            return 2;
        }
        // TODO: create a new section
        set_prev_token(p, t);
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
