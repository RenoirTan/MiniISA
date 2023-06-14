#ifndef MINIISA_SECTION_H
#   define MINIISA_SECTION_H

#   include <stddef.h>
#   include <miniisa/symbol.h>

#   define MINIISA_SECTION_NAME_MAX_LEN (255)

// a collection of symbols, used to categorise data as well as manage
// permissions
typedef struct miniisa_section {
    // name of the section
    char name[MINIISA_SECTION_NAME_MAX_LEN+1];
    // list of symbols
    miniisa_symbol_t *symbols;
    // number of symbols in this section
    size_t symbols_count;
} miniisa_section_t;

miniisa_section_t *miniisa_section_init(miniisa_section_t *s);

#endif
