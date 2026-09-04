#ifndef LZ77_H
#define LZ77_H

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

// token stream format written by the compress.
// flag bit: 0 = literal, 1 = match.
// literal: 8 bits (raw byte).
// match: 15 bits distance, 8 bits (length - MIN_MATCH).

// compresses input[0..input_len] into a newly malloc'd buffer.
// on success returns 0, sets *out_buf/*out_len and caller owns *out_buf (free()).
// on failure returns -1 and leaves *out_buf/*out_len untouched.

int lz77_compress(const uint8_t *input, size_t input_len, uint8_t **out_buf, size_t * out_len);

// decompresses input [0..input_len] into a newly malloc'd buffer.
// on success returns 0, sets *out_buf/*out_len and caller owns *out_buf (free()).
// on failure (e.g. corrupt stream) returns -1 and leaves *out_buf/*out_len untouched.

int lz77_decompress(const uint8_t *input, size_t input_len, uint8_t **out_buf, size_t *out_len);


#endif