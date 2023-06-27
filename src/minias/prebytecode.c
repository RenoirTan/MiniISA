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

data_stmt_t *init_data_stmt(data_stmt_t *d) {
    __MINIISA_INIT(d, data_stmt_t);
    d->data = NULL;
    d->length = 0;
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
