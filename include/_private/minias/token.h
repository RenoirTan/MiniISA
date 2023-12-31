#ifndef MINIAS_TOKEN_H
#   define MINIAS_TOKEN_H

#   include <stddef.h>
#   include <stdint.h>

#   ifdef __cplusplus
extern "C" {
#   endif

#   define MAX_TOKEN_LEN 255

typedef enum token_type {
    UNKNOWN_TOKEN = 0,
    IDENTIFIER_TOKEN = 1,
    INT_TOKEN = 2,
    FLOAT_TOKEN = 3,
    COMMA_TOKEN = 4,
    NEWLINE_TOKEN = 5,
    COLON_TOKEN = 6,
    EOF_TOKEN = 7
} token_type_t;

typedef struct token {
    char span[MAX_TOKEN_LEN+1];
    size_t length;
    token_type_t token_type;
    size_t line_no;
    size_t col_no;
} token_t;

token_t *init_token(token_t *token);

token_t *copy_token(token_t *dest, token_t *src);

int make_eof_token(token_t *token);

#   ifdef __cplusplus
}
#   endif

#endif
