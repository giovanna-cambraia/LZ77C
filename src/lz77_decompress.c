#include "lz77.h"
#include "bitstream.h"
#include "hashchain.h"

#define DIST_BITS 15
#define LEN_BITS 8

typedef struct
{
    uint8_t *buf;
    size_t len;
    size_t cap;
} outbuf_t;

static int outbuf_init(outbuf_t *ob, size_t initial_cap)
{
    if (initial_cap == 0)
        initial_cap = 64;
    ob->buf = (uint8_t *)malloc(initial_cap);
    if (!ob->buf)
        return -1;
    ob->len = 0;
    ob->cap = initial_cap;
    return 0;
}

static int outbuf_ensure(outbuf_t *ob, size_t needed)
{
    if (needed <= ob->cap)
        return 0;
    size_t new_cap = ob->cap;
    while (new_cap < needed)
        new_cap *= 2;
    uint8_t *new_buf = (uint8_t *)realloc(ob->buf, new_cap);
    if (!new_buf)
        return -1;
    ob->buf = new_buf;
    ob->cap = new_cap;
    return 0;
}

static int outbuf_push(outbuf_t *ob, uint8_t byte)
{
    if (outbuf_ensure(ob, ob->len + 1) != 0)
        return -1;
    ob->buf[ob->len++] = byte;
    return 0;
}

int lz77_decompress(const uint8_t *input, size_t input_len, uint8_t **out_buf, size_t *out_len)
{
    if (input_len == 0)
    {
        uint8_t *empty = (uint8_t *)malloc(1);
        if (!empty)
            return -1;
        *out_buf = empty;
        *out_len = 0;
        return 0;
    }

    bitreader_t br;
    br_init(&br, input, input_len);

    uint32_t len_hi, len_lo;
    if (br_read_bits(&br, 32, &len_hi) != 0 || br_read_bits(&br, 32, &len_lo) != 0)
    {
        return -1;
    }

    // Reconstruct in 64-bit first, then narrow to size_t
    uint64_t target_len_64 = ((uint64_t)len_hi << 32) | (uint64_t)len_lo;
    size_t target_len = (size_t)target_len_64;

    outbuf_t ob;
    if (outbuf_init(&ob, target_len + 64) != 0)
    {
        return -1;
    }

    while (ob.len < target_len)
    {
        uint32_t flag;
        if (br_read_bits(&br, 1, &flag) != 0)
        {
            goto fail;
        }

        if (flag == 0)
        {
            uint32_t byte_val;
            if (br_read_bits(&br, 8, &byte_val) != 0)
                goto fail;
            if (outbuf_push(&ob, (uint8_t)byte_val) != 0)
                goto fail;
        }
        else
        {
            // match: distance + (length - MIN_MATCH)
            uint32_t distance, len_field;
            if (br_read_bits(&br, DIST_BITS, &distance) != 0 || br_read_bits(&br, LEN_BITS, &len_field) != 0)
            {
                goto fail;
            }

            uint32_t length = len_field + MIN_MATCH;

            if (distance == 0 || distance > ob.len)
            {
                // corrupt stream: distance must point within already decoded output.
                goto fail;
            }

            if (length > target_len - ob.len)
            {
                length = (uint32_t)(target_len - ob.len);
            }

            if (outbuf_ensure(&ob, ob.len + length) != 0)
                goto fail;

            // byte-by-byte copy: source region may overlap the region being written so no memcpy use.
            size_t src = ob.len - distance;
            for (uint32_t i = 0; i < length; i++)
            {
                ob.buf[ob.len] = ob.buf[src];
                ob.len++;
                src++;
            }
        }
    }

    *out_buf = ob.buf;
    *out_len = ob.len;
    return 0;

fail:
    free(ob.buf);
    return -1;
}