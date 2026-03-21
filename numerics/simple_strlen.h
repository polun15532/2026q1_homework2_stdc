#ifndef NUMERICS_SIMPLE_STRLEN_H
#define NUMERICS_SIMPLE_STRLEN_H

#include <stddef.h>

size_t single_byte_strlen(const unsigned char *s);
size_t four_byte_strlen(const unsigned char *s);

#endif
