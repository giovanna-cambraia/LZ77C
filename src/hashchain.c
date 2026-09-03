#include "hashchain.h"
#include <stdlib.h>
 
static inline uint32_t hash3(const uint8_t *p) {
    uint32_t h = ((uint32_t)p[0] << 16) | ((uint32_t)p[1] << 8) | (uint32_t)p[2];
    h = (h * 2654435761u) >> (32 - HASH_BITS);
    return h & (HASH_SIZE - 1);
}
 
int hashchain_init(hashchain_t *hc, const uint8_t *input, size_t input_len) {
    hc->input = input;
    hc->input_len = input_len;
 
    hc->head = (int32_t *)malloc(HASH_SIZE * sizeof(int32_t));
    if (!hc->head) return -1;
    for (size_t i = 0; i < HASH_SIZE; i++) hc->head[i] = -1;
 
    if (input_len == 0) {
        hc->prev = NULL;
        return 0;
    }
 
    hc->prev = (int32_t *)malloc(input_len * sizeof(int32_t));
    if (!hc->prev) {
        free(hc->head);
        hc->head = NULL;
        return -1;
    }
    for (size_t i = 0; i < input_len; i++) hc->prev[i] = -1;
 
    return 0;
}
 
void hashchain_free(hashchain_t *hc) {
    free(hc->head);
    free(hc->prev);
    hc->head = NULL;
    hc->prev = NULL;
    hc->input = NULL;
    hc->input_len = 0;
}
 
void hashchain_insert(hashchain_t *hc, size_t pos) {
    uint32_t h = hash3(hc->input + pos);
    hc->prev[pos] = hc->head[h];
    hc->head[h] = (int32_t)pos;
}
 
match_t hashchain_find_match(hashchain_t *hc, size_t pos) {
    match_t best = {0, 0};
 
    if (pos + MIN_MATCH > hc->input_len) return best;
 
    uint32_t h = hash3(hc->input + pos);
    int32_t cand = hc->head[h];
 
    size_t window_start = (pos > WINDOWS_SIZE) ? pos - WINDOWS_SIZE : 0;
    size_t max_len = hc->input_len - pos;
    if (max_len > MAX_MATCH) max_len = MAX_MATCH;
 
    int chain_steps = 0;
    while (cand >= 0 && (size_t)cand >= window_start && chain_steps < MAX_CHAIN_LENGTH) {
    
        if (best.length == 0 ||
            (best.length < max_len &&
             hc->input[(size_t)cand + best.length] == hc->input[pos + best.length])) {
 
            size_t len = 0;
            while (len < max_len && hc->input[(size_t)cand + len] == hc->input[pos + len]) {
                len++;
            }
 
            if (len > best.length) {
                best.length = (uint32_t)len;
                best.distance = (uint32_t)(pos - (size_t)cand);
                if (len >= max_len) break; 
            }
        }
 
        cand = hc->prev[cand];
        chain_steps++;
    }
 
    if (best.length < MIN_MATCH) {
        best.length = 0;
        best.distance = 0;
    }
 
    return best;
}