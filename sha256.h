/*
 * sha256.h - SHA-256 implementation from scratch (no external libraries).
 *
 * Part of the Ponteiros_Em_C showcase: pure pointer + bitwise work.
 * Reference: FIPS PUB 180-4 (Secure Hash Standard).
 *
 * Author: Silas Vasconcelos (s-v7)
 * License: MIT
 */

#ifndef SHA256_H
#define SHA256_H

#include <stddef.h>
#include <stdint.h>

#define SHA256_DIGEST_SIZE 32          /* raw bytes  */
#define SHA256_HEX_SIZE    (SHA256_DIGEST_SIZE * 2 + 1) /* hex + '\0' */

/* Streaming context: feed data with sha256_update(), close with sha256_final(). */
typedef struct {
    uint32_t state[8];     /* intermediate hash values H0..H7 */
    uint64_t bitlen;       /* total message length, in bits   */
    uint8_t  buffer[64];   /* 512-bit working block           */
    size_t   buflen;       /* bytes currently held in buffer  */
} SHA256_CTX;

void sha256_init(SHA256_CTX *ctx);
void sha256_update(SHA256_CTX *ctx, const uint8_t *data, size_t len);
void sha256_final(SHA256_CTX *ctx, uint8_t digest[SHA256_DIGEST_SIZE]);

/* One-shot helpers. out_hex must hold SHA256_HEX_SIZE bytes. */
void sha256_bytes(const uint8_t *data, size_t len, uint8_t digest[SHA256_DIGEST_SIZE]);
void sha256_hex(const uint8_t *data, size_t len, char out_hex[SHA256_HEX_SIZE]);

#endif /* SHA256_H */

