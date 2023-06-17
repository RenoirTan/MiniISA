#include <stdlib.h>
#include <string.h>
#include <miniisa/extra.h>
#include <miniisa/bytecode.h>

miniisa_bytecode_t *miniisa_bytecode_init(miniisa_bytecode_t *b) {
    __MINIISA_INIT(b, miniisa_bytecode_t);
    memset(b->label, '\0', MINIISA_BYTECODE_LABEL_MAX_LEN+1);
    b->bytes = NULL;
    b->bytes_count = 0;
    b->sections = NULL;
    b->sections_count = 0;
    b->symbols = NULL;
    b->symbols_count = 0;
    return b;
}
