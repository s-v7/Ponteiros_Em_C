/*
 * sha256.c - SHA-256 implementation from scratch (FIPS PUB 180-4).
 *
 * Everything here is pointer + bitwise manipulation: no OpenSSL, no libs.
 * This is the cryptographic primitive that turns a plain linked list into
 * a tamper-evident blockchain (see blockchain.c).
 *
 * Author: Silas Vasconcelos Cruz (s-v7)
 * License: MIT
 */
#include "sha256.h"
#include <string.h>

/* Rotate right: the core bit operation of SHA-256. */
#define ROTR(x, n) (((x) >> (n)) | ((x) << (32 - (n))))

#define CH(x, y, z)  (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define EP0(x)  (ROTR(x, 2)  ^ ROTR(x, 13) ^ ROTR(x, 22))
#define EP1(x)  (ROTR(x, 6)  ^ ROTR(x, 11) ^ ROTR(x, 25))
#define SIG0(x) (ROTR(x, 7)  ^ ROTR(x, 18) ^ ((x) >> 3))
#define SIG1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ ((x) >> 10))

/* First 32 bits of the fractional parts of the cube roots of the first 64 primes. */
static const uint32_t K[64] = {
    0x428a2f98,0x71374491,0xb5c0fbcf,0xe9b5dba5,0x3956c25b,0x59f111f1,0x923f82a4,0xab1c5ed5,
    0xd807aa98,0x12835b01,0x243185be,0x550c7dc3,0x72be5d74,0x80deb1fe,0x9bdc06a7,0xc19bf174,
    0xe49b69c1,0xefbe4786,0x0fc19dc6,0x240ca1cc,0x2de92c6f,0x4a7484aa,0x5cb0a9dc,0x76f988da,
    0x983e5152,0xa831c66d,0xb00327c8,0xbf597fc7,0xc6e00bf3,0xd5a79147,0x06ca6351,0x14292967,
    0x27b70a85,0x2e1b2138,0x4d2c6dfc,0x53380d13,0x650a7354,0x766a0abb,0x81c2c92e,0x92722c85,
    0xa2bfe8a1,0xa81a664b,0xc24b8b70,0xc76c51a3,0xd192e819,0xd6990624,0xf40e3585,0x106aa070,
    0x19a4c116,0x1e376c08,0x2748774c,0x34b0bcb5,0x391c0cb3,0x4ed8aa4a,0x5b9cca4f,0x682e6ff3,
    0x748f82ee,0x78a5636f,0x84c87814,0x8cc70208,0x90befffa,0xa4506ceb,0xbef9a3f7,0xc67178f2
};

/* Compress one 512-bit block held in ctx->buffer into ctx->state. */
static void sha256_transform(SHA256_CTX *ctx) {
    uint32_t w[64];
    uint32_t a, b, c, d, e, f, g, h, t1, t2;
    const uint8_t *p = ctx->buffer;
    int i;

    /* Big-endian load of the 16 message words, then expand to 64. */
    for (i = 0; i < 16; i++) {
        w[i] = ((uint32_t)p[i * 4] << 24) | ((uint32_t)p[i * 4 + 1] << 16) |
               ((uint32_t)p[i * 4 + 2] << 8) | ((uint32_t)p[i * 4 + 3]);
    }
    for (i = 16; i < 64; i++) {
        w[i] = SIG1(w[i - 2]) + w[i - 7] + SIG0(w[i - 15]) + w[i - 16];
    }

    a = ctx->state[0]; b = ctx->state[1]; c = ctx->state[2]; d = ctx->state[3];
    e = ctx->state[4]; f = ctx->state[5]; g = ctx->state[6]; h = ctx->state[7];

    for (i = 0; i < 64; i++) {
        t1 = h + EP1(e) + CH(e, f, g) + K[i] + w[i];
        t2 = EP0(a) + MAJ(a, b, c);
        h = g; g = f; f = e; e = d + t1;
        d = c; c = b; b = a; a = t1 + t2;
    }

    ctx->state[0] += a; ctx->state[1] += b; ctx->state[2] += c; ctx->state[3] += d;
    ctx->state[4] += e; ctx->state[5] += f; ctx->state[6] += g; ctx->state[7] += h;
}

void sha256_init(SHA256_CTX *ctx) {
    ctx->state[0] = 0x6a09e667; ctx->state[1] = 0xbb67ae85;
    ctx->state[2] = 0x3c6ef372; ctx->state[3] = 0xa54ff53a;
    ctx->state[4] = 0x510e527f; ctx->state[5] = 0x9b05688c;
    ctx->state[6] = 0x1f83d9ab; ctx->state[7] = 0x5be0cd19;
    ctx->bitlen = 0;
    ctx->buflen = 0;
}

void sha256_update(SHA256_CTX *ctx, const uint8_t *data, size_t len) {
    size_t i;
    for (i = 0; i < len; i++) {
        ctx->buffer[ctx->buflen++] = data[i];
        if (ctx->buflen == 64) {
            sha256_transform(ctx);
            ctx->bitlen += 512;
            ctx->buflen = 0;
        }
    }
}

void sha256_final(SHA256_CTX *ctx, uint8_t digest[SHA256_DIGEST_SIZE]) {
    size_t i = ctx->buflen;

    /* Append the 0x80 byte, then pad with zeros up to the length field. */
    ctx->bitlen += (uint64_t)ctx->buflen * 8;
    ctx->buffer[i++] = 0x80;
    if (i > 56) {
        while (i < 64) ctx->buffer[i++] = 0x00;
        sha256_transform(ctx);
        i = 0;
    }
    while (i < 56) ctx->buffer[i++] = 0x00;

    /* Append the 64-bit big-endian total length in bits. */
    for (int j = 7; j >= 0; j--) {
        ctx->buffer[56 + (7 - j)] = (uint8_t)(ctx->bitlen >> (j * 8));
    }
    sha256_transform(ctx);

    /* Emit the digest big-endian. */
    for (i = 0; i < 4; i++) {
        for (int s = 0; s < 8; s++) {
            digest[s * 4 + i] = (uint8_t)(ctx->state[s] >> (24 - i * 8));
        }
    }
}

void sha256_bytes(const uint8_t *data, size_t len, uint8_t digest[SHA256_DIGEST_SIZE]) {
    SHA256_CTX ctx;
    sha256_init(&ctx);
    sha256_update(&ctx, data, len);
    sha256_final(&ctx, digest);
}

void sha256_hex(const uint8_t *data, size_t len, char out_hex[SHA256_HEX_SIZE]) {
    static const char hexd[] = "0123456789abcdef";
    uint8_t digest[SHA256_DIGEST_SIZE];
    sha256_bytes(data, len, digest);
    for (int i = 0; i < SHA256_DIGEST_SIZE; i++) {
        out_hex[i * 2]     = hexd[digest[i] >> 4];
        out_hex[i * 2 + 1] = hexd[digest[i] & 0x0f];
    }
    out_hex[SHA256_DIGEST_SIZE * 2] = '\0';
}
