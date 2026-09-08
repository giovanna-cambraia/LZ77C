// Benchmarks the WASM LZ77 core against Node's built-in zlib and a naive
// pure-JS LZ77 implementation, across a few representative input types.
// Run after `npm run build` and `bash scripts/sync-wasm.sh`: node benchmark.js

const zlib = require("zlib");
const {
  compress: wasmCompress,
  decompress: wasmDecompress,
} = require("./dist/index.js");
const { naiveCompress, naiveDecompress } = require("./naive-lz77.js");

function makeRepetitiveText(size) {
  const unit = "The quick brown fox jumps over the lazy dog. ";
  return Buffer.from(unit.repeat(Math.ceil(size / unit.length))).subarray(
    0,
    size,
  );
}

function makeNaturalText(size) {
  // pseudo-natural text: varied word lengths, some repetition, not as
  // artificially uniform as makeRepetitiveText but still real prose
  // structure (LZ77 does reasonably on this, just not spectacularly).
  const words = [
    "system",
    "compress",
    "buffer",
    "stream",
    "window",
    "match",
    "token",
    "byte",
    "length",
    "distance",
    "hash",
    "chain",
    "native",
    "module",
    "the",
    "and",
    "of",
    "to",
    "a",
    "in",
    "is",
    "that",
    "for",
    "with",
  ];
  const out = [];
  let len = 0;
  while (len < size) {
    const w = words[Math.floor(Math.random() * words.length)];
    out.push(w);
    len += w.length + 1;
  }
  return Buffer.from(out.join(" ")).subarray(0, size);
}

function makeRandomBytes(size) {
  return require("crypto").randomBytes(size);
}

function timeMs(fn) {
  const start = process.hrtime.bigint();
  const result = fn();
  const end = process.hrtime.bigint();
  return { result, ms: Number(end - start) / 1e6 };
}

async function timeMsAsync(fn) {
  const start = process.hrtime.bigint();
  const result = await fn();
  const end = process.hrtime.bigint();
  return { result, ms: Number(end - start) / 1e6 };
}

async function benchWasm(input) {
  const c = await timeMsAsync(() => wasmCompress(input));
  const d = await timeMsAsync(() => wasmDecompress(c.result));
  const ok = Buffer.compare(input, d.result) === 0;
  return {
    name: "wasm-lz77",
    compressedLen: c.result.length,
    compressMs: c.ms,
    decompressMs: d.ms,
    ok,
  };
}

function benchZlib(input) {
  const c = timeMs(() => zlib.deflateSync(input));
  const d = timeMs(() => zlib.inflateSync(c.result));
  const ok = Buffer.compare(input, d.result) === 0;
  return {
    name: "zlib",
    compressedLen: c.result.length,
    compressMs: c.ms,
    decompressMs: d.ms,
    ok,
  };
}

function benchNaive(input) {
  const c = timeMs(() => naiveCompress(input));
  const d = timeMs(() => naiveDecompress(c.result));
  const ok = Buffer.compare(input, d.result) === 0;
  return {
    name: "naive-js-lz77",
    compressedLen: c.result.length,
    compressMs: c.ms,
    decompressMs: d.ms,
    ok,
  };
}

function printRow(label, r, originalLen) {
  const ratio = (r.compressedLen / originalLen).toFixed(3);
  const status = r.ok ? "OK" : "ROUNDTRIP FAILED";
  console.log(
    `  ${label.padEnd(16)} ratio=${ratio.padStart(6)}  compress=${r.compressMs.toFixed(2).padStart(8)}ms  decompress=${r.decompressMs.toFixed(2).padStart(8)}ms  [${status}]`,
  );
}

async function runDataset(name, input) {
  console.log(`\n=== ${name} (${input.length} bytes) ===`);

  const wasmResult = await benchWasm(input);
  printRow(wasmResult.name, wasmResult, input.length);

  const zlibResult = benchZlib(input);
  printRow(zlibResult.name, zlibResult, input.length);

  // naive JS is O(window * n) with no indexing -- keep it off large/random
  // inputs or it will take an unreasonable amount of time to finish.
  if (input.length <= 20000) {
    const naiveResult = benchNaive(input);
    printRow(naiveResult.name, naiveResult, input.length);
  } else {
    console.log(
      `  ${"naive-js-lz77".padEnd(16)} skipped (input too large for naive O(window*n) scan)`,
    );
  }
}

async function main() {
  const size = 10000;

  await runDataset("repetitive text", makeRepetitiveText(size));
  await runDataset("natural-ish text", makeNaturalText(size));
  await runDataset("random bytes (worst case)", makeRandomBytes(size));
}

main().catch((err) => {
  console.error("benchmark threw:", err);
  process.exit(1);
});
