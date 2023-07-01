#ifndef MINIISA_BYTECODE_H
#   define MINIISA_BYTECODE_H

#   include <stddef.h>
#   include <stdint.h>
#   include <miniisa/section.h>
#   include <miniisa/symbol.h>

#   ifdef __cplusplus
extern "C" {
#   endif

#   define MINIISA_BYTECODE_LABEL_MAX_LEN (255)

// a bytecode object file, stores sections and symbols
typedef struct miniisa_bytecode {
    // name of the bytecode library
    char label[MINIISA_BYTECODE_LABEL_MAX_LEN+1];
    // actual data
    uint8_t *bytes;
    // number of bytes
    size_t bytes_count;
    // sections
    miniisa_section_t *sections;
    // number of sections
    size_t sections_count;
    // symbols
    miniisa_symbol_t *symbols;
    // number of symbols
    size_t symbols_count;
} miniisa_bytecode_t;

miniisa_bytecode_t *miniisa_bytecode_init(miniisa_bytecode_t *b);

int miniisa_bytecode_new_section(miniisa_bytecode_t *b, miniisa_section_t *s);

int miniisa_bytecode_terminate_last_section(miniisa_bytecode_t *b);

int miniisa_bytecode_new_symbol(miniisa_bytecode_t *b, miniisa_symbol_t *s);

int miniisa_bytecode_terminate_last_symbol(miniisa_bytecode_t *b);

#   ifdef __cplusplus
}
#   endif

#endif
