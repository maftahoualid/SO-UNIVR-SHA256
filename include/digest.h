#ifndef DIGEST_H
#define DIGEST_H

#include <stdint.h>  // per uint8_t e tipi interi standard

/**
 * Calcola l'impronta (digest) SHA-256 del buffer di input.
 * 
 * @param data Puntatore ai dati di input su cui calcolare il digest.
 * @param size Dimensione in byte del buffer di input.
 * @param hash Puntatore al buffer di output dove verrà scritto il digest (32 byte).
 *             Deve essere allocato dal chiamante.
 */
void digest_buffer(const uint8_t *data, size_t size, uint8_t *hash);

#endif
