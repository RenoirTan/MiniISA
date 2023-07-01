#ifndef MINIISA_SECTION_H
#   define MINIISA_SECTION_H

#   include <stddef.h>
#   include <stdint.h>
#   include <miniisa/symbol.h>

#   ifdef __cplusplus
extern "C" {
#   endif

#   define MINIISA_SECTION_NAME_MAX_LEN (255)

// a collection of symbols, used to categorise data as well as manage
// permissions
typedef struct miniisa_section {
    // name of the section
    char name[MINIISA_SECTION_NAME_MAX_LEN+1];
    // span range
    size_t start;
    size_t end;
    // permissions
    int32_t permissions;
} miniisa_section_t;

miniisa_section_t *miniisa_section_init(miniisa_section_t *s);

size_t miniisa_section_set_name(miniisa_section_t *s, char *name);

#   ifdef __cplusplus
}
#   endif

#endif
