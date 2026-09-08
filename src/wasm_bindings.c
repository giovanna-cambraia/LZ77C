// WASM export shim.

// lz77_compress/lz77_decompress, use double-pointer out-params (uint8_t **out_buf, size_t *out_len), which are akward to call from JS
// across the WASM boundary. These wrappers flatten that into a simpler pointer-in/pointer-out interface:

// uint8_t *was_compress(const uint8_t *input, size_t input_len, size_t *out_len)

// JS side calls this, reads *out_len, copies out_len bytes starting at the returned pointer, then MUST call wasm_free() on that pointer
// release the WASM heap allocation. Returns NULL on failure.

#include "lz77.h"
#include <stdlib.h>

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

EMSCRIPTEN_KEEPALIVE
uint8_t *wasm_compress(const uint8_t *input, size_t input_len, size_t *out_len) {
    uint8_t *result = NULL;
    size_t result_len = 0;

    if (lz77_compress(input, input_len, &result, &result_len) != 0) {
        return NULL;
    }

    *out_len = result_len;
    return result;
}

EMSCRIPTEN_KEEPALIVE
uint8_t *wasm_decompress(const uint8_t *input, size_t input_len, size_t *out_len) {
    uint8_t *result = NULL;
    size_t result_len = 0;

    if (lz77_decompress(input, input_len, &result, &result_len) != 0) {
        return NULL;
    }

    *out_len = result_len;
    return result;
}

EMSCRIPTEN_KEEPALIVE
void wasm_free(uint8_t *ptr) {
    free(ptr);
}
