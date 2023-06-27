#ifndef MINIISA_STRNLEN_H
#   define MINIISA_STRNLEN_H

#   include <stddef.h>
#   include <string.h>

#   ifdef strnlen
#       define miniisa_strnlen strnlen
#   else

static size_t strnlen(char *s, size_t n) {
    size_t i;
    for (i = 0; i < n; i++) {
        if (s[i] == '\0') return i;
    }
    return n;
}

#   endif

#endif
