#include <openssl/sha.h>  // libreria OpenSSL per SHA-256
#include <stdint.h>       // tipi interi fissi (uint8_t)
#include <stddef.h>       // size_t

/**
 * Calcola l'hash SHA-256 di un buffer di dati.
 * 
 * @param data Puntatore al buffer dati in input.
 * @param size Dimensione in byte del buffer dati.
 * @param hash Puntatore al buffer di output (almeno 32 byte) dove sarà scritto il digest.
 */
void digest_buffer(const uint8_t *data, size_t size, uint8_t *hash) {
    SHA256_CTX ctx;          // struttura di contesto per SHA-256
    SHA256_Init(&ctx);       // inizializza il contesto SHA-256
    SHA256_Update(&ctx, data, size);  // aggiorna il contesto con i dati
    SHA256_Final(hash, &ctx);          // calcola il digest finale e lo scrive in 'hash'
}
