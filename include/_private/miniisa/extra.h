#ifndef MINIISA_EXTRA_H
#   define MINIISA_EXTRA_H

#   include <miniisa/config.h>

#   ifdef __cplusplus
extern "C" {
#   endif

#   ifdef MINIISA_DEBUG
#       define __DBG(...) (printf(__VA_ARGS__))
#   else
#       define __DBG(...) (0)
#   endif

#   define __MINIISA_INIT(ptr, type) { \
    if (!ptr) { \
        ptr = malloc(sizeof(type)); \
        if (!ptr) return NULL; \
    } \
}

#   define __ERR(...) (fprintf(stderr, __VA_ARGS__))

#   ifdef __cplusplus
}
#   endif

#endif
