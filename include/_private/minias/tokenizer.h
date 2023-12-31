#ifndef MINIAS_TOKENIZER_H
#   define MINIAS_TOKENIZER_H

#   include <stddef.h>
#   include <stdint.h>
#   include <stdio.h>

#   include "token.h"

#   ifdef __cplusplus
extern "C" {
#   endif

#   define MAX_BUFFER_LEN 255

#   define NO_LAST_CHAR (-255)

typedef enum tokenizer_err {
    TOKENIZER_ERR_TOO_LONG = 1,
    TOKENIZER_ERR_UNKNOWN_TOKEN = 2,
    TOKENIZER_ERR_BAD_CHAR_IN_NUM = 3,
    TOKENIZER_ERR_BAD_CHAR_IN_IDENTIFIER = 4,
    TOKENIZER_ERR_TOO_MANY_DOTS = 5,
    TOKENIZER_ERR_UNRECOGNIZED_CHAR = 6,
    TOKENIZER_ERR_UNRECOGNIZED_SEQUENCE = 7,
    TOKENIZER_ERR_INVALID_START_CHAR = 8,
    TOKENIZER_ERR_BAD_NEWLINE_SEQUENCE = 9
} tokenizer_err_t;

typedef struct tokenizer {
    FILE *infile; // input stream of characters
    char token[MAX_TOKEN_LEN+1]; // token as a string
    size_t token_len; // length of token
    size_t line_no; // current line number
    size_t col_no; // column of current character on this line
    char buffer[MAX_BUFFER_LEN+1]; // buffer of new characters from infile
    size_t buf_len; // current length of contents in buffer
    size_t buf_idx; // which character in the buffer we are reading
    int last_char; // previous character
    token_type_t detected_token_type; // type of token
    int is_comment; // whether we are currently in a comment
} tokenizer_t;

tokenizer_t *init_tokenizer(tokenizer_t *t);

FILE *set_tokenizer_file(tokenizer_t *t, FILE *f);

int get_next_token(tokenizer_t *t, token_t *token);

#   ifdef __cplusplus
}
#   endif

#endif
