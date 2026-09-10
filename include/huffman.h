#ifndef HUFFMAN_H
#define HUFFMAN_H

#include <stdint.h>
#include <stddef.h>
#include "bitstream.h"

// canonical huffman over a fixed-size symbol alphabet [0, num_symbols]; 
// "canonical" means codes are derived purely from a code length array in a fixed, reproducible way (shortest codes firts, 
// lexicographic among equal lengths) so the decoder only needs the code-length table, not the full tree shape, 
// to reconstruct identical codes.

#define HUFFMAX_MAX_CODE_LEN 15

typedef struct {
    uint32_t num_symbols;
    uint8_t *code_lengths; // num_symbols entries, 0 = symbol unused 
    uint16_t *codes; // num_symbols entries, canonical codes bits (only valid wher code_lengths[i] > 0)

    uint16_t *sorted_codes; // num_symbol entries actually used, ascending by (len, code)
    uint32_t *sorted_symbols; // paralley array: which symbol each sorted_code entry maps to
    uint32_t num_used_symbols;
} huffman_table_t;

// builds a canonical huffman table from symbol frequency counts.
// freqs must have num_symbols entries; freqs[i] == 0 means symbol i does
// not appear and will not be assigned a code. Returns 0 on success, -1 on allocation failure. 

int huffman_build_table(huffman_table_t *table, const uint32_t *freqs, uint32_t num_symbols);

void huffman_table_free(huffman_table_t *table);

// writes the code-length table, not data, to the bitstream, so a decoder can rebuild identical canonical code.
// format: num_symbols is assumed known by both (a compile-time constant per alphabet, not serialized).
// each of num_symbols entries is written as 4 bits (0-15, HUFFMAN_MAX_CODE_LEN)

int huffman_write_table(bitwriter_t *bw, const huffman_table_t *table);

// reads a code-length table previously written by huffman_write_table and rebuilts the same canonical table
int huffman_read_table(bitreader_t *br, huffman_table_t *table, uint32_t num_symbols);
 
// encodes a single symbol's canonical code into the bitstream
int huffman_encode_symbol(bitwriter_t *bw, const huffman_table_t *table, uint32_t symbol);
 
// decodes a single symbol from the bitstream by walking bit-by-bit against the sorted canonical code list. 
// returns 0 on success and sets out_symbol, -1 on a malformed/corrupt stream (no matching code found or ran out of bits). 
int huffman_decode_symbol(bitreader_t *br, const huffman_table_t *table, uint32_t *out_symbol);
 
#endif 