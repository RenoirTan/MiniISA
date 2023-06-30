#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <miniisa/data.h>

int miniisa_str_to_uint64(char *s, uint64_t *out) {
    char *end = s;
    uint64_t result = strtoul(s, &end, 10); // TODO: allow other bases
    if (*end != '\0') { // assert nothing else after number
        return 1;
    }
    *out = result;
    return 0;
}

int miniisa_str_to_le_int(char *s, uint8_t *buf, size_t *w) {
    int status = 0;
    uint64_t integer;
    status = miniisa_str_to_uint64(s, &integer);
    if (status) return status;
    for (size_t i = 1; i <= 8; i++) {
        uint8_t byte = integer & 0xff;
        buf[i] = byte;
        if (byte > 0) {
            *w = i;
        }
        integer >>= 8; // next byte
    }
    return status;
}

int miniisa_str_to_double(char *s, double *out) {
    char *end = s;
    double result = strtod(s, &end);
    if (*end != '\0') {
        return 1;
    }
    *out = result;
    return 0;
}

int miniisa_str_to_float_bytes(char *s, uint8_t *buf) {
    int status = 0;
    double flotilla;
    status = miniisa_str_to_double(s, &flotilla);
    if (status) return status;
    memcpy(buf, &flotilla, 8);
    return status;
}
