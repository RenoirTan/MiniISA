#include <stdlib.h>
#include <string.h>
#include <miniisa/extra.h>
#include <miniisa/section.h>

miniisa_section_t *miniisa_section_init(miniisa_section_t *s) {
    __MINIISA_INIT(s, miniisa_section_t);
    memset(s->name, '\0', MINIISA_SECTION_NAME_MAX_LEN);
    s->symbols = NULL;
    s->symbols_count = 0;
    return s;
}
