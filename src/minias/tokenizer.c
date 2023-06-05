#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <extra.h>
#include "tokenizer.h"

tokenizer_t *init_tokenizer(tokenizer_t *t) {
    if (!t) {
        t = malloc(sizeof(tokenizer_t));
        if (!t) return NULL;
    }

    t->infile = NULL;
    t->token_len = 0;
    memset(t->token, '\0', MAX_TOKEN_LEN+1);
    t->line_no = 1;
    t->col_no = 1;
    memset(t->buffer, '\0', MAX_BUFFER_LEN+1);
    t->buf_len = 0;
    t->buf_idx = MAX_BUFFER_LEN; // must trigger new buffer
    t->last_char = 0;
    t->detected_token_type = UNKNOWN_TOKEN;
    t->is_comment = 0;

    return t;
}

FILE *set_tokenizer_file(tokenizer_t *t, FILE *f) {
    FILE *old = t->infile;
    t->infile = f;
    t->line_no = 1;
    t->col_no = 1;
    return old;
}

static size_t grab_buffer(tokenizer_t *t) {
    t->buf_len = fread(t->buffer, sizeof(char), MAX_BUFFER_LEN, t->infile);
    __DBG("grab_buffer: t->buf_len = %zu\n", t->buf_len);
    t->buf_idx = 0;
    return t->buf_len;
}

// TODO: Add unicode support in the future?
static inline int grab_next_char(tokenizer_t *t) {
    if (t->buf_idx >= t->buf_len) {
        grab_buffer(t);
        if (t->buf_len == 0) {
            return EOF;
        }
    }
    char cur_char = t->buffer[t->buf_idx];
    t->buf_idx++;
    __DBG("cur_char: '%c' %hhu\n", cur_char, cur_char);
    return (int) cur_char;
}

// set token to t->token and reset t->token
static int return_token(tokenizer_t *t, token_t *token) {
    // transfer to *token
    token->length = t->token_len;
    strncpy(token->span, t->token, t->token_len);
    token->span[token->length] = '\0';
    token->token_type = t->detected_token_type;
    token->line_no = t->line_no;
    token->col_no = t->col_no;

    // reset
    t->token_len = 0;
    t->detected_token_type = UNKNOWN_TOKEN;
    return 0;
}

static int grow_token(tokenizer_t *t, char last_char) {
    if (t->token_len >= MAX_TOKEN_LEN) {
        return 1;
    } else {
        t->token[t->token_len] = last_char;
        t->token_len++;
        return 0;
    }
}

#define _GROW_TOKEN_CHECKED(t, next_char) { \
    if (grow_token(t, next_char)) { \
        return TOKENIZER_TOO_LONG; \
    } \
}

int get_next_token(tokenizer_t *t, token_t *token) {
    int go_on = 1;
    while (go_on) {
        int next_char = grab_next_char(t);
        if (next_char == EOF) {
            return EOF;
        }
        if (t->is_comment) {
            __DBG("is_comment\n");
            t->last_char = next_char;
            if (next_char == '\r' || next_char == '\n') {
                t->is_comment = 0;
            }
            continue;
        }
        if ((isspace(next_char) || next_char == ';') && t->token_len > 0) { // space or ;
            __DBG("is_space\n");
            return_token(t, token);
            go_on = 0;
        } else if (isalpha(next_char) || next_char == '_') { // char is [A-Za-z_]
            __DBG("is_alpha\n");
            if (t->detected_token_type == IDENTIFIER_TOKEN) {
                _GROW_TOKEN_CHECKED(t, next_char);
            } else if (t->detected_token_type == UNKNOWN_TOKEN) {
                if (t->token_len > 0) {
                    return TOKENIZER_UNKNOWN_TOKEN;
                }
                grow_token(t, next_char);
                t->detected_token_type = IDENTIFIER_TOKEN;
            } else {
                return TOKENIZER_BAD_CHAR_IN_NUM;
            }
        } else if (isdigit(next_char)) { // char is [0-9]
            __DBG("is_digit\n");
            if (
                t->detected_token_type == INT_TOKEN ||
                t->detected_token_type == FLOAT_TOKEN ||
                t->detected_token_type == IDENTIFIER_TOKEN
            ) {
                _GROW_TOKEN_CHECKED(t, next_char);
            } else if (t->detected_token_type == UNKNOWN_TOKEN) {
                if (t->token_len > 0) {
                    return TOKENIZER_UNKNOWN_TOKEN;
                }
                grow_token(t, next_char);
                t->detected_token_type = INT_TOKEN;
            }
        } else if (next_char == '#') { // char is #
            __DBG("is_pound\n");
            return_token(t, token);
            go_on = 0;
            t->is_comment = 1;
        } else if (next_char == '.') { // char is .
            __DBG("is_do\n");
            if (t->detected_token_type == INT_TOKEN) {
                t->detected_token_type = FLOAT_TOKEN;
            } else {
                return TOKENIZER_TOO_MANY_DOTS;
            }
            _GROW_TOKEN_CHECKED(t, next_char);
        } else {
            __DBG("is_other\n");
            return TOKENIZER_UNRECOGNIZED_CHAR;
        }
        t->last_char = next_char;
    }
    return 0;
}

#undef _GROW_TOKEN_CHECKED
