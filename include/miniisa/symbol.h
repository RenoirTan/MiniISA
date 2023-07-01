#ifndef MINIISA_SYMBOL_H
#   define MINIISA_SYMBOL_H

#   include <stddef.h>

#   ifdef __cplusplus
extern "C" {
#   endif

#   define MINIISA_SYMBOL_NAME_MAX_LEN (255)

// unit of data, like a function, constant or "static" variable
typedef struct miniisa_symbol {
    char name[MINIISA_SYMBOL_NAME_MAX_LEN+1];
    size_t start;
    size_t end;
} miniisa_symbol_t;

miniisa_symbol_t *miniisa_symbol_init(miniisa_symbol_t *s);

// truncated to 255 bytes plus null byte
size_t miniisa_symbol_set_name(miniisa_symbol_t *s, char *name);

#   ifdef __cplusplus
}
#   endif

#endif
