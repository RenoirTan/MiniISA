#ifndef MINIAS_TOKEN_H
#   define MINIAS_TOKEN_H

#   include <stddef.h>
#   include <stdint.h>

#   define MAX_TOKEN_LEN 255

typedef enum token_type {
    UNKNOWN_TOKEN = 0,
    IDENTIFIER_TOKEN = 1,
    INT_TOKEN = 2,
    FLOAT_TOKEN = 3,
    COMMA_TOKEN = 4,
    NEWLINE_TOKEN = 5,
    COLON_TOKEN = 6
} token_type_t;

typedef struct token {
    char span[MAX_TOKEN_LEN+1];
    size_t length;
    token_type_t token_type;
    size_t line_no;
    size_t col_no;
} token_t;

#endif
