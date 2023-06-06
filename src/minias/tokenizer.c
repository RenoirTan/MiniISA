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
    t->last_char = NO_LAST_CHAR;
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

static inline int is_identifier_initial(int codepoint) {
    return (isalpha(codepoint) || codepoint == '_');
}

static inline int is_identifier_medial(int codepoint) {
    return (is_identifier_initial(codepoint) || isdigit(codepoint));
}

#define _GROW_TOKEN_CHECKED(t, next_char) { \
    if (grow_token(t, next_char)) { \
        return TOKENIZER_TOO_LONG; \
    } \
}

#define _RET_TOKEN_IF_NOT_EMPTY_ELSE_CONTINUE(t, token) { \
    if (t->token_len > 0) { \
        return_token(t, token); \
        break; \
    } else { \
        continue; \
    } \
}

int get_next_token(tokenizer_t *t, token_t *token) {
    int go_on = 1;
    while (go_on) {
        // get the latest character
        // if t->last_char is not NO_LAST_CHAR, then use the character in
        // t->last_char first (because it hasn't been parsed yet)
        __DBG("get_next_token: t->last_char = %d\n", t->last_char);
        if (t->last_char == NO_LAST_CHAR) {
            t->last_char = grab_next_char(t);
        }
        int last_char = t->last_char;
        t->last_char = NO_LAST_CHAR;
        __DBG("get_next_token: last_char = %d\n", last_char);

        // return EOF
        if (last_char == EOF) {
            return EOF;
        }
        // skip character if comment
        // if newline, set t->is_comment to false
        if (t->is_comment) {
            __DBG("get_next_token: is_comment\n");
            if (last_char == '\r' || last_char == '\n') {
                t->is_comment = 0;
                t->last_char = last_char;
            }
            continue;
        }

        // newline
        if (last_char == '\r' || last_char == '\n') {
            __DBG("get_next_token: is_crlf\n");
            // terminate current token if not newline
            if (t->token_len > 0 && t->detected_token_type != NEWLINE_TOKEN) {
                t->last_char = last_char;
                return_token(t, token);
                break;
            }
        // space
        } else if (isspace(last_char)) {
            __DBG("get_next_token: is_space\n");
            _RET_TOKEN_IF_NOT_EMPTY_ELSE_CONTINUE(t, token);
        // begin comment
        } else if (last_char == ';') {
            __DBG("get_next_token: is_semicolon\n");
            t->is_comment = 1;
            _RET_TOKEN_IF_NOT_EMPTY_ELSE_CONTINUE(t, token);
        // comma separator
        } else if (last_char == ',') {
            __DBG("get_next_comma: is_comma\n");
            // terminate current token
            if (t->token_len > 0) {
                // store in t->last_char so we can return COMMA_TOKEN later
                t->last_char = last_char;
            // create COMMA_TOKEN
            } else {
                grow_token(t, ',');
                t->detected_token_type = COMMA_TOKEN;
            }
            return_token(t, token);
            break;
        }

        switch (t->detected_token_type) {
        // cell specialization
        case UNKNOWN_TOKEN:
            __DBG("get_next_token: UNKNOWN_TOKEN\n");
            if (t->token_len > 0) {
                return TOKENIZER_UNRECOGNIZED_SEQUENCE;
            }
            if (is_identifier_initial(last_char)) {
                t->detected_token_type = IDENTIFIER_TOKEN;
            } else if (isdigit(last_char)) {
                t->detected_token_type = INT_TOKEN;
            } else if (last_char == '\r' || last_char == '\n') {
                t->detected_token_type = NEWLINE_TOKEN;
            } else {
                return TOKENIZER_INVALID_START_CHAR;
            }
            grow_token(t, last_char);
            break;
        case IDENTIFIER_TOKEN:
            __DBG("get_next_token: IDENTIFIER_TOKEN\n");
            if (is_identifier_medial(last_char)) {
                _GROW_TOKEN_CHECKED(t, last_char);
            } else {
                return TOKENIZER_BAD_CHAR_IN_IDENTIFIER;
            }
            break;
        case INT_TOKEN:
            __DBG("get_next_token: INT_TOKEN\n");
            if (isdigit(last_char)) {
                _GROW_TOKEN_CHECKED(t, last_char);
            } else if (last_char == '.') {
                _GROW_TOKEN_CHECKED(t, last_char);
                t->detected_token_type = FLOAT_TOKEN;
            } else {
                return TOKENIZER_BAD_CHAR_IN_NUM;
            }
            break;
        case FLOAT_TOKEN:
            __DBG("get_next_token: FLOAT_TOKEN\n");
            if (isdigit(last_char)) {
                _GROW_TOKEN_CHECKED(t, last_char);
            } else if (last_char == '.') {
                return TOKENIZER_TOO_MANY_DOTS;
            } else {
                return TOKENIZER_BAD_CHAR_IN_NUM;
            }
            break;
        case NEWLINE_TOKEN:
            __DBG("get_next_token: NEWLINE_TOKEN\n");
            if (t->token_len > 1) {
                return TOKENIZER_BAD_NEWLINE_SEQUENCE;
            }
            if (last_char == '\n' && t->token[0] == '\r') {
                _GROW_TOKEN_CHECKED(t, '\n');
            } else {
                t->last_char = last_char;
                return_token(t, token);
                go_on = 0;
            }
            break;
        default:
            __DBG("get_next_token: fuck -> %i\n", t->detected_token_type);
            return 255;
        }
    }
    return 0;
}

#undef _RET_TOKEN_IF_NOT_EMPTY_ELSE_CONTINUE
#undef _GROW_TOKEN_CHECKED
