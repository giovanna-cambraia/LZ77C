const WINDOW_SIZE = 32768;
const MIN_MATCH = 3;
const MAX_MATCH = 258;
 
function naiveCompress(input) {
  const tokens = [];
  let pos = 0;
 
  while (pos < input.length) {
    let bestLen = 0;
    let bestDist = 0;
 
    const windowStart = Math.max(0, pos - WINDOW_SIZE);
    const maxLen = Math.min(MAX_MATCH, input.length - pos);
 
    // linear backward scan -- O(window) per position, no hashing/indexing
    for (let cand = pos - 1; cand >= windowStart; cand--) {
      let len = 0;
      while (len < maxLen && input[cand + len] === input[pos + len]) {
        len++;
      }
      if (len > bestLen) {
        bestLen = len;
        bestDist = pos - cand;
        if (len >= maxLen) break;
      }
    }
 
    if (bestLen >= MIN_MATCH) {
      tokens.push({ type: 'match', distance: bestDist, length: bestLen });
      pos += bestLen;
    } else {
      tokens.push({ type: 'literal', byte: input[pos] });
      pos += 1;
    }
  }
 
  // serialize tokens into a flat byte buffer: 1 tag byte + payload.
  // tag 0 = literal (1 byte follows), tag 1 = match (4 bytes: distance hi/lo, length hi/lo)
  const out = [];
  for (const t of tokens) {
    if (t.type === 'literal') {
      out.push(0, t.byte);
    } else {
      out.push(1, (t.distance >> 8) & 0xff, t.distance & 0xff, (t.length >> 8) & 0xff, t.length & 0xff);
    }
  }
  return Buffer.from(out);
}
 
function naiveDecompress(compressed) {
  const out = [];
  let i = 0;
  while (i < compressed.length) {
    const tag = compressed[i++];
    if (tag === 0) {
      out.push(compressed[i++]);
    } else {
      const distance = (compressed[i] << 8) | compressed[i + 1];
      const length = (compressed[i + 2] << 8) | compressed[i + 3];
      i += 4;
      const start = out.length - distance;
      for (let k = 0; k < length; k++) {
        out.push(out[start + k]);
      }
    }
  }
  return Buffer.from(out);
}
 
module.exports = { naiveCompress, naiveDecompress };
 