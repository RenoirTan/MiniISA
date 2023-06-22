#include <stdlib.h>
#include <string.h>
#include <miniisa/extra.h>
#include <miniisa/permissions.h>
#include <miniisa/section.h>

miniisa_section_t *miniisa_section_init(miniisa_section_t *s) {
    __MINIISA_INIT(s, miniisa_section_t);
    memset(s->name, '\0', MINIISA_SECTION_NAME_MAX_LEN);
    s->start = 0;
    s->end = 0;
    s->permissions = MINIISA_NO_PERMS;
    return s;
}

// TODO: return number of bytes copied, 256 if truncated
size_t miniisa_section_set_name(miniisa_section_t *s, char *name) {
    strncpy(s->name, name, MINIISA_SYMBOL_NAME_MAX_LEN);
    return 0;
}
