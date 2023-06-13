#include <stdlib.h>
#include <string.h>
#include "token.h"

token_t *init_token(token_t *token) {
    if (!token) {
        token = malloc(sizeof(token));
        if (!token) return NULL;
    }
    memset(&token->span, '\0', MAX_TOKEN_LEN+1);
    token->length = 0;
    token->token_type = UNKNOWN_TOKEN;
    token->line_no = 0;
    token->col_no = 0;
    return token;
}
