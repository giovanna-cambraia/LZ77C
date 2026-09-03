#include "bitstream.h"

int bw_init(bitwriter_t *bw, size_t initial_cap)
{
    if (initial_cap == 0)
        initial_cap = 64;
    bw->buf = (uint8_t *)calloc(initial_cap, 1);
    if (!bw->buf)
        return -1;
    bw->cap = initial_cap;
    bw->byte_pos = 0;
    bw->bit_pos = 0;
    return 0;
}

void bw_free(bitwriter_t *bw)
{
    free(bw->buf);
    bw->buf = NULL;
    bw->cap = 0;
    bw->byte_pos = 0;
    bw->bit_pos = 0;
}

// ensure at least one more byte is writable at buf[byte_pos].

static int bw_ensure_capacity(bitwriter_t *bw, size_t needed_bytes)
{
    if (needed_bytes <= bw->cap)
        return 0;
    size_t new_cap = bw->cap;
    while (new_cap < needed_bytes)
        new_cap *= 2;
    uint8_t *new_buf = (uint8_t *)realloc(bw->buf, new_cap);
    if (!new_buf)
        return -1;
    memset(new_buf + bw->cap, 0, new_cap - bw->cap);
    bw->buf = new_buf;
    bw->cap = new_cap;
    return 0;
}

int bw_write_bits(bitwriter_t *bw, uint32_t value, uint8_t nbits)
{
    if (nbits == 0 || nbits > 32)
        return -1;

    // walks bits MSB-first from requested value.
    for (int8_t i = (int8_t)nbits - 1; i >= 0; i--)
    {
        if (bw_ensure_capacity(bw, bw->byte_pos + 1) != 0)
            return -1;

        uint8_t bit = (uint8_t)((value >> i) & 1u);
        bw->buf[bw->byte_pos] |= (uint8_t)(bit << (7 - bw->bit_pos));

        bw->bit_pos++;
        if (bw->bit_pos == 8)
        {
            bw->bit_pos = 0;
            bw->byte_pos++;
        }
    }
    return 0;
}

int bw_write_byte(bitwriter_t *bw, uint8_t byte)
{
    return bw_write_bits(bw, byte, 8);
}

size_t bw_flush(bitwriter_t *bw, const uint8_t **out_buf)
{
    size_t total_bytes = bw->byte_pos + (bw->bit_pos > 0 ? 1 : 0);
    *out_buf = bw->buf;
    return total_bytes;
}

void br_init(bitreader_t *br, const uint8_t *buf, size_t size)
{
    br->buf = buf;
    br->size = size;
    br->byte_pos = 0;
    br->bit_pos = 0;
}

int br_read_bits(bitreader_t *br, uint8_t nbits, uint32_t *out_value)
{
    if (nbits == 0 || nbits > 32)
        return -1;

    uint32_t value = 0;
    for (uint8_t i = 0; i < nbits; i++)
    {
        if (br->byte_pos >= br->size)
            return -1;

        uint8_t bit = (uint8_t)((br->buf[br->byte_pos] >> (7 - br->bit_pos)) & 1u);
        value = (value << 1) | bit;

        br->bit_pos++;
        if (br->bit_pos == 8)
        {
            br->bit_pos = 0;
            br->byte_pos++;
        }
    }

    *out_value = value;
    return 0;
}

int br_read_byte(bitreader_t *br, uint8_t *out_byte)
{
    uint32_t value;
    if (br_read_bits(br, 8, &value) != 0)
        return -1;
    *out_byte = (uint8_t)value;
    return 0;
}

int br_at_end(const bitreader_t *br)
{
    return br->byte_pos >= br->size ||
           (br->byte_pos == br->size - 1 && br->bit_pos == 8);
}