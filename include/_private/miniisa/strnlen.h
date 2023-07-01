#ifndef MINIISA_STRNLEN_H
#   define MINIISA_STRNLEN_H

#   include <stddef.h>
#   include <string.h>

#   ifdef __cplusplus
extern "C" {
#   endif

#   ifndef strnlen

static size_t strnlen(char *s, size_t n) {
    size_t i;
    for (i = 0; i < n; i++) {
        if (s[i] == '\0') return i;
    }
    return n;
}

#   endif

#   ifdef __cplusplus
}
#   endif

#endif
