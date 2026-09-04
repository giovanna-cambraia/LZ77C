#include "lz77.h"

static int run_case(const char *name, const uint8_t *input, size_t input_len)
{
    uint8_t *compressed = NULL;
    size_t compressed_len = 0;
    uint8_t *restored = NULL;
    size_t restored_len = 0;
    int ok = 0;

    if (lz77_compress(input, input_len, &compressed, &compressed_len) != 0)
    {
        printf("[FAIL] %s: lz77_compress returned error\n", name);
        return 0;
    }

    if (lz77_decompress(compressed, compressed_len, &restored, &restored_len) != 0)
    {
        printf("[FAIL] %s: lz77_decompress returned error\n", name);
        free(compressed);
        return 0;
    }

    if (restored_len != input_len)
    {
        printf("[FAIL] %s: length mismatch (input %zu, restored %zu)\n", name, input_len, restored_len);
    }
    else if (input_len > 0 && memcmp(input, restored, input_len) != 0)
    {
        printf("[FAIL] %s: content mismatch after roundrip\n", name);
    }
    else
    {
        double ratio = (input_len > 0) ? (double)compressed_len / (double)input_len : 0.0;
        printf("[PASS] %s: input %zu bytes -> compressed &zu bytes (ration %.3f)\n", name, input_len, compressed_len, ratio);
        ok = 1;
    }

    free(compressed);
    free(restored);
    return ok;
}

int main(void)
{
    int total = 0;
    int passed = 0;

    {
        total++;
        passed += run_case("empty", NULL, 0);
    }

    {
        uint8_t data[] = {0x42};
        total++;
        passed += run_case("repetitive_byte", data, sizeof(data));
    }

    {
        const char *text = "abababababababababababababababababababab"
                           "abababababababababababababababababababab";
        total++;
        passed += run_case("repetitive_text", (const uint8_t *)text, strlen(text));
    }

    {
        uint8_t data [256];
        for (int i = 0; i < 256; i++) data [i] = (uint8_t)i;
        total++;
        passed += run_case("no_repeats_incrementing", data, sizeof(data));
    }

     {
        const char *text =
            "The quick brown fox jumps over the lazy dog. "
            "The quick brown fox jumps over the lazy dog again. "
            "Pack my box with five dozen liquor jugs. "
            "Pack my box with five dozen liquor jugs, again and again. "
            "Sphinx of black quartz, judge my vow. Sphinx of black quartz, judge my vow.";
        total++;
        passed += run_case("mixed_text", (const uint8_t *)text, strlen(text));
    }
 
     // match at max window/length boundary: distance near WINDOW_SIZE isn't
     // tested here (needs a huge buffer), but a long single repeated run
     // exercises MAX_MATCH clamping/splitting across multiple match tokens.
    {
        size_t len = 1000;
        uint8_t *data = (uint8_t *)malloc(len);
        for (size_t i = 0; i < len; i++) data[i] = 'z';
        total++;
        passed += run_case("long_single_run", data, len);
        free(data);
    }
 
    printf("\n%d/%d cases passed\n", passed, total);
    return (passed == total) ? 0 : 1;
}