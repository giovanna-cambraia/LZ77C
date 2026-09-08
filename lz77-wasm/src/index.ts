// The C side (see wasm_bindings.c in the lz77c repo) exports a
// pointer-in/pointer-out interface:

//   uint8_t *wasm_compress(const uint8_t *input, size_t input_len, size_t *out_len)
//   uint8_t *wasm_decompress(const uint8_t *input, size_t input_len, size_t *out_len)
//   void wasm_free(uint8_t *ptr)


// eslint-disable-next-line @typescript-eslint/no-var-requires
const createLZ77Module = require('../vendor/lz77.js');

interface EmscriptenModule {
  _malloc(size: number): number;
  _free(ptr: number): void;
  _wasm_compress(inputPtr: number, inputLen: number, outLenPtr: number): number;
  _wasm_decompress(inputPtr: number, inputLen: number, outLenPtr: number): number;
  _wasm_free(ptr: number): void;
  HEAPU8: Uint8Array;
  getValue(ptr: number, type: string): number;
}

let modulePromise: Promise<EmscriptenModule> | null = null;

function getModule(): Promise<EmscriptenModule> {
  if (!modulePromise) {
    modulePromise = createLZ77Module() as Promise<EmscriptenModule>;
  }
  return modulePromise;
}

// size_t is 32-bit on the wasm32 target this module is built for, so a
// single 4-byte out-param slot is enough to hold the result length pointer
// wasm_compress/wasm_decompress write into.
const SIZE_T_BYTES = 4;

function callWasmFn(
  mod: EmscriptenModule,
  fn: (inputPtr: number, inputLen: number, outLenPtr: number) => number,
  input: Buffer
): Buffer {
  const inputPtr = mod._malloc(input.length > 0 ? input.length : 1);
  const outLenPtr = mod._malloc(SIZE_T_BYTES);
  let resultPtr = 0;

  try {
    mod.HEAPU8.set(input, inputPtr);

    resultPtr = fn(inputPtr, input.length, outLenPtr);
    if (resultPtr === 0) {
      throw new Error('lz77-wasm: operation failed (see C-side error path)');
    }

    const outLen = mod.getValue(outLenPtr, 'i32');
    const result = Buffer.from(mod.HEAPU8.subarray(resultPtr, resultPtr + outLen));
    return result;
  } finally {
    mod._free(inputPtr);
    mod._free(outLenPtr);
    if (resultPtr !== 0) {
      mod._wasm_free(resultPtr);
    }
  }
}

export async function compress(input: Buffer): Promise<Buffer> {
  const mod = await getModule();
  return callWasmFn(mod, mod._wasm_compress.bind(mod), input);
}

export async function decompress(input: Buffer): Promise<Buffer> {
  const mod = await getModule();
  return callWasmFn(mod, mod._wasm_decompress.bind(mod), input);
}