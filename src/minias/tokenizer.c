#include <ctype.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>

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
    size_t bytes_read = fread(t->buffer, sizeof(char), MAX_BUFFER_LEN, t->infile);
    t->buf_idx = 0;
    return bytes_read;
}

static inline char grab_next_char(tokenizer_t *t) {
    if (t->buf_idx >= MAX_BUFFER_LEN) {
        grab_buffer(t);
        t->buf_idx = 0;
    }
    char cur_char = t->buffer[t->buf_idx];
    if (cur_char != EOF) {
        t->buf_idx++;
    }
    printf("cur_char: '%c' %hhu\n", cur_char, cur_char);
    return cur_char;
}

// set token to t->token and reset t->token
static int return_token(tokenizer_t *t, char *token) {
    strncpy(token, t->token, t->token_len);
    token[t->token_len] = '\0';
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

int get_next_token(tokenizer_t *t, char *token) {
    if (t->last_char == EOF) {
        return EOF;
    }
    int go_on = 1;
    while (go_on) {
        char next_char = grab_next_char(t);
        if (t->is_comment) {
            puts("is_comment");
            t->last_char = next_char;
            if (next_char == '\r' || next_char == '\n') {
                t->is_comment = 0;
            }
            continue;
        }
        if ((isspace(next_char) || next_char == ';') && t->token_len > 0) { // space or ;
            puts("is_space");
            return_token(t, token);
            go_on = 0;
        } else if (isalpha(next_char) || next_char == '_') { // char is [A-Za-z_]
            puts("is_alpha");
            if (t->detected_token_type == IDENTIFIER_TOKEN) {
                if (grow_token(t, next_char)) {
                    return 1;
                }
            } else if (t->detected_token_type == UNKNOWN_TOKEN) {
                if (t->token_len > 0) {
                    return 1;
                }
                grow_token(t, next_char);
                t->detected_token_type = IDENTIFIER_TOKEN;
            } else {
                return 1;
            }
        } else if (isdigit(next_char)) { // char is [0-9]
            puts("is_digit");
            if (t->detected_token_type == INT_TOKEN || t->detected_token_type == FLOAT_TOKEN) {
                if (grow_token(t, next_char)) {
                    return 1;
                }
            } else if (t->detected_token_type == UNKNOWN_TOKEN) {
                if (t->token_len > 0) {
                    return 1;
                }
                grow_token(t, next_char);
                t->detected_token_type = INT_TOKEN;
            }
        } else if (next_char == '#') { // char is #
            puts("is_pound");
            return_token(t, token);
            go_on = 0;
            t->is_comment = 1;
        } else if (next_char == '.') { // char is .
            puts("is_dot");
            if (t->detected_token_type == INT_TOKEN) {
                t->detected_token_type = FLOAT_TOKEN;
            } else {
                return 1;
            }
        } else {
            puts("is_other");
            return 1;
        }
        t->last_char = next_char;
    }
    return 0;
}
