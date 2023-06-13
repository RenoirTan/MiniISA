#ifndef MINIISA_SYMBOL_H
#   define MINIISA_SYMBOL_H

#   include <stddef.h>

#   define MINIISA_SYMBOL_NAME_MAX_LEN (255)

// unit of data, like a function, constant or "static" variable
typedef struct miniisa_symbol {
    char name[MINIISA_SYMBOL_NAME_MAX_LEN+1];
    size_t bytes_count;
} miniisa_symbol_t;

#endif
