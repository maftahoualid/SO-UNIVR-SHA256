#ifndef DIGEST_H
#define DIGEST_H

#include <stdint.h>

void digest_buffer(const uint8_t *data, size_t size, uint8_t *hash);

#endif
