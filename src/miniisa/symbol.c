#include <stdlib.h>
#include <string.h>
#include <_private/miniisa/extra.h>
#include <miniisa/symbol.h>

miniisa_symbol_t *miniisa_symbol_init(miniisa_symbol_t *s) {
    __MINIISA_INIT(s, miniisa_symbol_t);
    memset(s->name, '\0', MINIISA_SYMBOL_NAME_MAX_LEN+1);
    s->start = 0;
    s->end = 0;
    return s;
}

// TODO: return number of bytes copied, 256 if truncated
size_t miniisa_symbol_set_name(miniisa_symbol_t *s, char *name) {
    strncpy(s->name, name, MINIISA_SYMBOL_NAME_MAX_LEN);
    return 0;
}
