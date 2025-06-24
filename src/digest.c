#include <openssl/sha.h>
#include <stdint.h>
#include <stddef.h>

void digest_buffer(const uint8_t *data, size_t size, uint8_t *hash) {
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    SHA256_Update(&ctx, data, size);
    SHA256_Final(hash, &ctx);
}
