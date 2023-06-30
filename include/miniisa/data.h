#ifndef MINIISA_DATA_H
#   define MINIISA_DATA_H

#   include <stddef.h>
#   include <stdint.h>

int miniisa_str_to_uint64(char *s, uint64_t *out);

/**
 * Parse `s` into an integer and convert it into a little-endian sequence of
 * bytes stored in `buf` of size at least 8. The number of bytes written is
 * then stored in `w`.
 */
int miniisa_str_to_le_int(char *s, uint8_t *buf, size_t *w);

int miniisa_str_to_double(char *s, double *out);

int miniisa_str_to_float_bytes(char *s, uint8_t *buf);

#endif
