#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <_private/miniisa/extra.h>
#include <miniisa/bytecode.h>

static int expand_vector(void **v, size_t *length, size_t data_type_size, size_t new_length) {
    if (new_length <= *length) {
        __ERR("expand_vector: new_length (%zu) <= *length (%zu)\n", new_length, *length);
        return 1;
    }
    void *n = realloc(*v, data_type_size * new_length);
    if (!n) {
        __ERR("expand_vector: could not realloc more memory\n");
        return 2;
    }
    *v = n;
    *length = new_length;
    return 0;
}

static int expand_sections_vector_by(miniisa_bytecode_t *b, size_t extra_length) {
    size_t new_length = b->sections_count + extra_length;
    return expand_vector(
        (void **) &b->sections,
        &b->sections_count,
        sizeof(miniisa_section_t),
        new_length
    );
}

static int expand_symbols_vector_by(miniisa_bytecode_t *b, size_t extra_length) {
    size_t new_length = b->symbols_count + extra_length;
    return expand_vector(
        (void **) &b->symbols,
        &b->symbols_count,
        sizeof(miniisa_symbol_t),
        new_length
    );
}

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

int miniisa_bytecode_new_section(miniisa_bytecode_t *b, miniisa_section_t *s) {
    int status;
    status = expand_sections_vector_by(b, 1);
    if (status) return status;
    b->sections[b->sections_count-1] = *s;
    return 0;
}

int miniisa_bytecode_terminate_last_section(miniisa_bytecode_t *b) {
    if (b->sections_count >= 1) {
        b->sections[b->sections_count-1].end = b->bytes_count;
    }
    return 0;
}

int miniisa_bytecode_new_symbol(miniisa_bytecode_t *b, miniisa_symbol_t *s) {
    int status;
    status = expand_symbols_vector_by(b, 1);
    if (status) return status;
    b->symbols[b->symbols_count-1] = *s;
    return 0;
}

int miniisa_bytecode_terminate_last_symbol(miniisa_bytecode_t *b) {
    if (b->symbols_count >= 1) {
        b->symbols[b->symbols_count-1].end = b->bytes_count;
    }
    return 0;
}
