/*
 * blockchain.c - A minimal blockchain in C.
 *
 * The whole point, for a "Ponteiros_Em_C" showcase: a blockchain is just a
 * linked list whose link is *two* things at once -
 *
 *     struct Block *prev   -> an ordinary C pointer (lets us walk the chain)
 *     char prev_hash[65]   -> a cryptographic "pointer" (makes it tamper-evident)
 *
 * Change one byte in any past block and every subsequent hash stops matching,
 * so the chain refuses to validate. That single idea is what separates a
 * blockchain from a plain linked list.
 *
 * Builds on the from-scratch SHA-256 in sha256.c.
 *
 * Compile: gcc -O2 -Wall -Wextra sha256.c blockchain.c -o blockchain_demo
 * Run:     ./blockchain_demo
 *
 * Author: Silas Vasconcelos Cruz (s-v7)
 * License: MIT
 */
#include "sha256.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define HASH_HEX_LEN 64
#define DATA_MAX     256

/* Proof-of-work difficulty: number of leading '0' hex digits required. */
#define POW_DIFFICULTY 3

typedef struct Block {
    uint32_t      index;                       		/* position in the chain        */
    long          timestamp;                    	/* unix time                    */
    char          data[DATA_MAX];               	/* payload (e.g. an ART record) */
    uint64_t      nonce;                         	/* proof-of-work counter        */
    char          prev_hash[HASH_HEX_LEN + 1];   	/* cryptographic pointer        */
    char          hash[HASH_HEX_LEN + 1];        	/* this block's own hash        */
    struct Block *prev;                          	/* C pointer to previous block  */
} Block;

typedef struct {
    Block *head;   /* genesis block            */
    Block *tail;   /* most recently added block */
    size_t length;
} Blockchain;

/* Serialize the hashable fields and compute this block's SHA-256 hex digest. */
static void compute_hash(const Block *b, char out_hex[HASH_HEX_LEN + 1]) {
    char buf[DATA_MAX + 256];
    int n = snprintf(buf, sizeof(buf), "%u|%ld|%s|%llu|%s",
                     b->index, b->timestamp, b->data,
                     (unsigned long long)b->nonce, b->prev_hash);
    sha256_hex((const uint8_t *)buf, (size_t)n, out_hex);
}

/* A hash satisfies PoW if it starts with `difficulty` zero hex digits. */
static int meets_difficulty(const char *hash, int difficulty) {
    for (int i = 0; i < difficulty; i++) {
        if (hash[i] != '0') return 0;
    }
    return 1;
}

/* Increment nonce until the hash meets the difficulty target. */
static void mine_block(Block *b, int difficulty) {
    b->nonce = 0;
    do {
        compute_hash(b, b->hash);
        b->nonce++;
    } while (!meets_difficulty(b->hash, difficulty));
    b->nonce--; /* compensate the final post-increment */
}

static Blockchain *chain_create(void) {
    Blockchain *chain = malloc(sizeof(*chain));
    if (!chain) { perror("malloc"); exit(EXIT_FAILURE); }
    chain->head = chain->tail = NULL;
    chain->length = 0;
    return chain;
}

static void chain_add(Blockchain *chain, const char *data) {
    Block *b = malloc(sizeof(*b));
    if (!b) { perror("malloc"); exit(EXIT_FAILURE); }

    b->index     = (uint32_t)chain->length;
    b->timestamp = (long)time(NULL);
    b->prev      = chain->tail;
    snprintf(b->data, sizeof(b->data), "%s", data);

    if (chain->tail) {
        memcpy(b->prev_hash, chain->tail->hash, HASH_HEX_LEN + 1);
    } else {
        /* Genesis block: the "previous hash" is all zeros. */
        memset(b->prev_hash, '0', HASH_HEX_LEN);
        b->prev_hash[HASH_HEX_LEN] = '\0';
    }

    mine_block(b, POW_DIFFICULTY);

    if (!chain->head) chain->head = b;
    chain->tail = b;
    chain->length++;
}

/*
 * Validate integrity by walking the C pointers and checking, for each block,
 * that (a) its stored hash still matches a recomputation, and (b) its
 * prev_hash equals the actual hash of the previous block.
 */
static int chain_is_valid(const Blockchain *chain) {
    char recomputed[HASH_HEX_LEN + 1];
    /* We keep backward pointers, so validate from tail to head. */
    for (Block *b = chain->tail; b != NULL; b = b->prev) {
        compute_hash(b, recomputed);
        if (strcmp(recomputed, b->hash) != 0) {
            printf("  ! block #%u: stored hash does not match its contents\n", b->index);
            return 0;
        }
        if (b->prev) {
            if (strcmp(b->prev_hash, b->prev->hash) != 0) {
                printf("  ! block #%u: prev_hash does not point to block #%u\n",
                       b->index, b->prev->index);
                return 0;
            }
        }
    }
    return 1;
}

static void chain_print(const Blockchain *chain) {
    for (Block *b = chain->head; b != NULL; ) {
        /* Walk forward by finding the block whose prev == b. O(n^2) but fine for a demo. */
        printf("  Block #%u  nonce=%llu\n", b->index, (unsigned long long)b->nonce);
        printf("    data : %s\n", b->data);
        printf("    prev : %.16s...\n", b->prev_hash);
        printf("    hash : %.16s...\n", b->hash);
        Block *next = NULL;
        for (Block *c = chain->tail; c != NULL; c = c->prev)
            if (c->prev == b) next = c;
        b = next;
    }
}

static void chain_free(Blockchain *chain) {
    Block *b = chain->tail;
    while (b) {
        Block *p = b->prev;
        free(b);
        b = p;
    }
    free(chain);
}

int main(void) {
    Blockchain *chain = chain_create();

    printf("Mining blocks (PoW difficulty = %d leading zeros)...\n\n", POW_DIFFICULTY);
    chain_add(chain, "GENESIS - CREA-PI ART ledger");
    chain_add(chain, "ART #1149437 ingested");
    chain_add(chain, "ART #1149438 ingested");
    chain_add(chain, "ART #1149439 ingested");

    printf("Chain (%zu blocks):\n", chain->length);
    chain_print(chain);

    printf("\nValidating original chain... %s\n",
           chain_is_valid(chain) ? "VALID" : "INVALID");

    /* Tamper with a past block to show the hash-pointer guarantee. */
    printf("\nTampering with block #2 (rewriting its data)...\n");
    for (Block *b = chain->tail; b != NULL; b = b->prev) {
        if (b->index == 2) {
            snprintf(b->data, sizeof(b->data), "ART #9999999 FORGED");
            break;
        }
    }

    printf("Re-validating tampered chain... %s\n",
           chain_is_valid(chain) ? "VALID" : "INVALID");

    chain_free(chain);
    return 0;
}
