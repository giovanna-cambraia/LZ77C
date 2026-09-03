#ifndef HASHCHAIN_H
#define HASHCHAIN_H

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#define WINDOWS_SIZE 32798
#define MIN_MATCH 3
#define MAX_MATCH 258
#define HASH_BITS 15
#define HASH_SIZE (1 << HASH_BITS)
#define MAX_CHAIN_LENGTH 128

typedef struct {
    const uint8_t *input;
    size_t input_len;

    int32_t *head; // HASH_SIZE entries: head[hash] = most recent pos with hash, -1 if none.
    int32_t *prev; // input_len entries: prev[pos] = previous pos with same hash, -1 if none.
} hashchain_t;

typedef struct {
    uint32_t distance; // 0 if no match found.
    uint32_t length; // 0 or < MIN_MATCH if no match found.
} match_t;

int hashchain_init(hashchain_t *hc, const uint8_t *input, size_t input_len);
void hashchain_free(hashchain_t *hc);

// hashes input [pos..pos+2]; call must ensure pos+2 < input_len.
void hashchain_insert(hashchain_t *hc, size_t pos);

// does not insert pos itself, call hashchain_insert separately
match_t hashchain_find_match(hashchain_t *hc, size_t pos);

#endif