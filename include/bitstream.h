#ifndef BITSTREAM_H
#define BITSTREAM_H

#include <stdint.h>
#include <stddef.h>

// growable bit-level writer; bits are packed MSB-first within each byte.
typedef struct {
    uint8_t *buf;
    size_t cap;
    size_t byte_pos;
    uint8_t bit_pos; // 0-7, next free bit within buf[byte_pos]
} bitwriter_t;

// read-only cursor oven an existing buffer by bitwriter_t.
typedef struct {
    const uint8_t *buf;
    size_t size;
    size_t byte_pos;
    uint8_t bit_pos;
} bitreader_t;

int bw_init(bitwriter_t *bw, size_t initial_cap);
void bw_free(bitwriter_t *bw);

// write low 'nbits' bits of vlaue (nbits in [1,32]).
int bw_write_bits(bitwriter_t *bw, uint32_t value, uint8_t nbits);
int bw_write_byte(bitwriter_t *bw, uint8_t byte);

// pads the final byte with zero bits and returns pointer/length; buffer ownership stats with bw until bw_free
size_t bw_flush(bitwriter_t *bw, const uint8_t **out_buf);

void br_init(bitreader_t *br, const uint8_t *buf, size_t size);
int br_read_bits(bitreader_t *br, uint8_t nbits, uint32_t *out_value);
int br_read_byte(bitreader_t *br, uint8_t *out_type);
int br_at_end(const bitreader_t *br);

#endif
