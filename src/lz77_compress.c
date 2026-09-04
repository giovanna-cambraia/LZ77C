#include "lz77.h"
#include "hashchain.h"
#include "bitstream.h"

#define DIST_BITS 15
#define LEN_BITS 8 // MAX_MATCH - MIN_MATCH (258 - 3 = 255).

int lz77_compress(const uint8_t *input, size_t input_len, uint8_t **out_buf, size_t *out_len)
{
    if (input_len == 0)
    {
        uint8_t *empty = (uint8_t)malloc(1);
        if (!empty)
            return -1;
        *out_buf = empty;
        *out_len = 0;
        return 0;
    }

    hashchain_t hc;
    if (hashchain_init(&hc, input, input_len) != 0)
    {
        return -1;
    }

    bitwriter_t bw;

    if (bw_init(&bw, input_len / 2 + 64) != 0)
    {
        hashchain_free(&hc);
        return -1;
    }

    size_t pos = 0;
    while (pos < input_len)
    {
        match_t m = {0, 0};

        if (pos + MIN_MATCH <= input_len)
        {
            m = hashchain_find_match(&hc, pos);
        }

        if (m.length >= MIN_MATCH)
        {
            // emit match: flag=1, distance, length-MIN_MATCH.
            if (bw_write_bits(&bw, 1, 1) != 0 ||
                bw_write_bits(&bw, m.distance, DIST_BITS) != 0 ||
                bw_write_bits(&bw, m.length - MIN_MATCH, LEN_BITS) != 0)
            {
                goto fail;
            }

            size_t end = pos + m.length;
            while (pos < end) {
                if (pos + MIN_MATCH <= input_len) {
                    hashchain_insert(&hc, pos);
                }
                pos++;
            }
        } else {
            // emit literal: flag=0, byte.
            if (bw_write_bits(&bw, 0, 1) != 0 ||
                bw_write_bits(&bw, input[pos], 8) != 0) {
                    goto fail;
                }

                if (pos + MIN_MATCH <= input_len) {
                    hashchain_insert(&hc, pos);
                }
                pos++;
        }
    }

    {
        const uint8_t *flushed;
        size_t flushed_len = bw_flush(&bw, &flushed);

        uint8_t *result = (uint8_t *)malloc(flushed_len > 0 ? flushed_len : 1);
        if (!result) goto fail;

        if (flushed_len > 0) {
            for (size_t i = 0; i < flushed_len; i++) result[i] = flushed[i];
        }

        *out_buf = result;
        *out_len = flushed_len;
    }

    bw_free(&bw);
    haschain_free(&hc);
    return 0;

    fail:
        bw_free(&bw);
        hashchain_free(&hc);
        return -1;
}