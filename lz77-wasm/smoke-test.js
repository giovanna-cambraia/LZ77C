const { compress, decompress } = require('./dist/index.js');

async function main() {
  const original = Buffer.from(
    'The quick brown fox jumps over the lazy dog. '.repeat(20),
    'utf-8'
  );

  console.log(`original: ${original.length} bytes`);

  const compressed = await compress(original);
  console.log(`compressed: ${compressed.length} bytes (ratio ${(compressed.length / original.length).toFixed(3)})`);

  const restored = await decompress(compressed);
  console.log(`restored: ${restored.length} bytes`);

  if (Buffer.compare(original, restored) === 0) {
    console.log('PASS: roundtrip matches original');
  } else {
    console.error('FAIL: roundtrip does NOT match original');
    process.exit(1);
  }
}

main().catch((err) => {
  console.error('smoke test threw:', err);
  process.exit(1);
});