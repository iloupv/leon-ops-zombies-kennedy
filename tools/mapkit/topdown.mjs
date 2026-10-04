// mapkit validator: top-down PNG of a mapkit dump (bo1_mapkit_dump -> mods/mapkit/mapkit_dump.json), for agents to read.
// usage: node tools/mapkit/topdown.mjs <dump.json> <out.png> [--scale 1.5] [--zlo z] [--zhi z]
// Draws: grid (64u, labels every 256u), retail brushes in the z band (grey), mapkit boxes (blue, B<i>), pathnodes (yellow,
// number), links (green walking, cyan traverse arrows; red = a walking link crosses a mapkit box), marks (perk magenta, trigger cyan ring, zone/room/
// spawner/other orange + name). Prints a text summary (counts, links crossing boxes) to stdout. No dependencies.
import fs from 'node:fs';
import zlib from 'node:zlib';

const args = process.argv.slice(2);
if (args.length < 2) { console.error('usage: topdown.mjs <dump.json> <out.png> [--scale s] [--zlo z] [--zhi z]'); process.exit(2); }
const opt = (k, d) => { const i = args.indexOf(k); return i >= 0 ? Number(args[i + 1]) : d; };
const dump = JSON.parse(fs.readFileSync(args[0], 'utf8'));
const S = opt('--scale', 1.5);
const [lo, hi] = dump.bounds;
const floorZ = dump.boxes.length ? Math.min(...dump.boxes.map(b => b.maxs[2])) : lo[2] + 64;
const zlo = opt('--zlo', floorZ + 4), zhi = opt('--zhi', floorZ + 120);
const W = Math.ceil((hi[0] - lo[0]) * S), H = Math.ceil((hi[1] - lo[1]) * S);
const px = new Uint8Array(W * H * 3).fill(18);
const X = x => (x - lo[0]) * S, Y = y => (hi[1] - y) * S; // +y up

function put(x, y, c, a = 1) {
  x |= 0; y |= 0;
  if (x < 0 || y < 0 || x >= W || y >= H) return;
  const o = (y * W + x) * 3;
  for (let k = 0; k < 3; k++) px[o + k] = Math.round(px[o + k] * (1 - a) + c[k] * a);
}
function rect(x0, y0, x1, y1, c, a, outline) {
  const ax = Math.min(X(x0), X(x1)), bx = Math.max(X(x0), X(x1)), ay = Math.min(Y(y0), Y(y1)), by = Math.max(Y(y0), Y(y1));
  for (let y = Math.floor(ay); y <= Math.ceil(by); y++)
    for (let x = Math.floor(ax); x <= Math.ceil(bx); x++) {
      const edge = x <= ax + 1 || x >= bx - 1 || y <= ay + 1 || y >= by - 1;
      if (edge && outline) put(x, y, outline, 1); else if (a) put(x, y, c, a);
    }
}
function line(x0, y0, x1, y1, c, a = 1) {
  const n = Math.max(1, Math.ceil(Math.hypot(x1 - x0, y1 - y0)));
  for (let i = 0; i <= n; i++) put(x0 + (x1 - x0) * i / n, y0 + (y1 - y0) * i / n, c, a);
}
function disc(cx, cy, r, c, ring) {
  for (let y = -r - 1; y <= r + 1; y++) for (let x = -r - 1; x <= r + 1; x++) {
    const d = Math.hypot(x, y);
    if (ring ? Math.abs(d - r) < 0.8 : d <= r) put(cx + x, cy + y, c);
  }
}
// 3x5 font, rows top->bottom, bits 4|2|1 = left|mid|right
const F = {
  '0': [7,5,5,5,7], '1': [2,6,2,2,7], '2': [7,1,7,4,7], '3': [7,1,7,1,7], '4': [5,5,7,1,1], '5': [7,4,7,1,7],
  '6': [7,4,7,5,7], '7': [7,1,1,1,1], '8': [7,5,7,5,7], '9': [7,5,7,1,7], '-': [0,0,7,0,0], '.': [0,0,0,0,2],
  '_': [0,0,0,0,7], ':': [0,2,0,2,0], ',': [0,0,0,2,4], '/': [1,1,2,4,4], ' ': [0,0,0,0,0],
  A: [2,5,7,5,5], B: [6,5,6,5,6], C: [7,4,4,4,7], D: [6,5,5,5,6], E: [7,4,6,4,7], F: [7,4,6,4,4], G: [7,4,5,5,7],
  H: [5,5,7,5,5], I: [7,2,2,2,7], J: [1,1,1,5,7], K: [5,5,6,5,5], L: [4,4,4,4,7], M: [5,7,7,5,5], N: [6,5,5,5,5],
  O: [7,5,5,5,7], P: [7,5,7,4,4], Q: [7,5,5,7,1], R: [7,5,6,5,5], S: [7,4,7,1,7], T: [7,2,2,2,2], U: [5,5,5,5,7],
  V: [5,5,5,5,2], W: [5,5,7,7,5], X: [5,5,2,5,5], Y: [5,5,2,2,2], Z: [7,1,2,4,7],
};
function text(x, y, s, c, k = 2) {
  for (const ch of String(s).toUpperCase()) {
    const g = F[ch] || F[' '];
    for (let r = 0; r < 5; r++) for (let b = 0; b < 3; b++)
      if (g[r] & (4 >> b)) for (let dy = 0; dy < k; dy++) for (let dx = 0; dx < k; dx++) put(x + b * k + dx, y + r * k + dy, c);
    x += 4 * k;
  }
}

// grid
for (let g = Math.ceil(lo[0] / 64) * 64; g <= hi[0]; g += 64) line(X(g), 0, X(g), H, g % 256 ? [34, 34, 34] : [55, 55, 55]);
for (let g = Math.ceil(lo[1] / 64) * 64; g <= hi[1]; g += 64) line(0, Y(g), W, Y(g), g % 256 ? [34, 34, 34] : [55, 55, 55]);
for (let g = Math.ceil(lo[0] / 256) * 256; g <= hi[0]; g += 256) text(X(g) + 2, 2, 'X' + g, [120, 120, 120]);
for (let g = Math.ceil(lo[1] / 256) * 256; g <= hi[1]; g += 256) text(2, Y(g) + 2, 'Y' + g, [120, 120, 120]);

// retail brushes in the band
const inBand = b => b.maxs[2] > zlo && b.mins[2] < zhi;
const retail = dump.brushes.filter(inBand);
for (const b of retail) rect(b.mins[0], b.mins[1], b.maxs[0], b.maxs[1], [120, 120, 120], 0.55);
// mapkit boxes
const boxes = dump.boxes.filter(inBand);
dump.boxes.forEach((b, i) => {
  if (!inBand(b)) { rect(b.mins[0], b.mins[1], b.maxs[0], b.maxs[1], null, 0, [40, 70, 140]); return; }
  rect(b.mins[0], b.mins[1], b.maxs[0], b.maxs[1], [60, 120, 255], 0.75, [120, 170, 255]);
});
dump.boxes.forEach((b, i) => { if (inBand(b)) text(X(b.mins[0]) + 2, Y(b.maxs[1]) + 2, 'B' + i, [255, 255, 255]); });

// links + nodes
const byIdx = new Map(dump.nodes.map(n => [n.i, n]));
function segHitsBox(a, c, b) { // 2D Liang-Barsky, with the box inside the node's walking band
  const z = Math.min(a[2], c[2]);
  if (b.maxs[2] <= z + 8 || b.mins[2] >= z + 64) return false;
  let t0 = 0, t1 = 1;
  const d = [c[0] - a[0], c[1] - a[1]];
  for (let k = 0; k < 2; k++) {
    if (Math.abs(d[k]) < 1e-6) { if (a[k] < b.mins[k] || a[k] > b.maxs[k]) return false; continue; }
    let u = (b.mins[k] - a[k]) / d[k], v = (b.maxs[k] - a[k]) / d[k];
    if (u > v) [u, v] = [v, u];
    t0 = Math.max(t0, u); t1 = Math.min(t1, v);
    if (t0 > t1) return false;
  }
  return true;
}
const crossing = [];
const traverses = [];
let inferredTraverses = 0;
const isTraverse = (n, m) => Array.isArray(n.traverses) ? n.traverses.includes(m.i) : n.type === 17 && m.type === 18;
for (const n of dump.nodes) for (const l of n.links) {
  const m = byIdx.get(l);
  if (!m) continue;
  if (isTraverse(n, m)) {
    traverses.push([n, m]);
    if (!Array.isArray(n.traverses)) inferredTraverses++;
    continue;
  }
  if (m.links.includes(n.i) && (isTraverse(m, n) || l < n.i)) continue; // draw each ordinary pair once
  const bad = dump.boxes.some(b => segHitsBox(n.origin, m.origin, b));
  if (bad) crossing.push(`${n.i}-${l}`);
  line(X(n.origin[0]), Y(n.origin[1]), X(m.origin[0]), Y(m.origin[1]), bad ? [255, 60, 60] : [60, 190, 90], bad ? 1 : 0.8);
}
// Traverses intentionally cross the sill. Draw their actual directed negotiation edges above walking links.
for (const [n, m] of traverses) {
  const ax = X(n.origin[0]), ay = Y(n.origin[1]), bx = X(m.origin[0]), by = Y(m.origin[1]);
  const dx = bx - ax, dy = by - ay, length = Math.max(1, Math.hypot(dx, dy));
  const colour = [70, 230, 240];
  for (let t = 0; t < length; t += 10) {
    const end = Math.min(t + 6, length);
    line(ax + dx * t / length, ay + dy * t / length, ax + dx * end / length, ay + dy * end / length, colour);
  }
  for (const side of [-1, 1])
    line(bx, by, bx - dx * 10 / length + side * dy * 5 / length, by - dy * 10 / length - side * dx * 5 / length, colour);
}
for (const n of dump.nodes) {
  disc(X(n.origin[0]), Y(n.origin[1]), 3, n.added ? [255, 150, 40] : [240, 220, 60]);
  text(X(n.origin[0]) + 5, Y(n.origin[1]) - 4, n.i, [230, 210, 90], 1);
}
// marks
const markColor = { perk: [240, 70, 240], trigger: [70, 230, 240], zone: [255, 160, 60], room: [255, 160, 60],
  door: [255, 230, 80], debris: [210, 150, 80], mysterybox: [150, 100, 255] };
for (const m of dump.marks) {
  const c = markColor[m.kind] || [255, 160, 60];
  const x = X(m.origin[0]), y = Y(m.origin[1]);
  if (m.kind === 'trigger') disc(x, y, Math.max(3, m.radius * S), c, true);
  else { rect(m.origin[0] - 6, m.origin[1] - 6, m.origin[0] + 6, m.origin[1] + 6, c, 1); }
  text(x + 8, y + 6, `${m.kind}:${m.name}`, c);
}
// Doctor findings are keyed by rank in report.md; circles identify the actual problem position.
const findingsArg = args.indexOf('--findings');
if (findingsArg >= 0) {
  const findings = JSON.parse(fs.readFileSync(args[findingsArg + 1], 'utf8'));
  for (const f of findings) if (Array.isArray(f.position)) {
    const colour = f.severity === 'FAIL' ? [255, 70, 70] : [255, 200, 40];
    disc(X(f.position[0]), Y(f.position[1]), 18, colour, true);
    text(X(f.position[0]) + 19, Y(f.position[1]) - 8, f.rank, colour, 2);
  }
}
const legend = [
  `${dump.layout} Z ${zlo}..${zhi}: FILLED BRUSH SLICE ONLY - NODES AND LINKS SHOW ALL HEIGHTS`,
  'GREY RETAIL BLUE MAPKIT GREEN WALK CYAN ARROW TRAVERSE RED WALK CROSSING',
  'RED TESTS NODE FOOT HEIGHT PLUS 8..64 - INDEPENDENT OF DISPLAY Z SLICE',
  'YELLOW DOOR BROWN DEBRIS PURPLE MYSTERY BOX'
];
const legendScale = W - 8 >= Math.max(...legend.map(s => s.length)) * 8 ? 2 : 1;
legend.forEach((s, i) => text(4, H - 56 + i * 14, s, [200, 200, 200], legendScale));

// PNG
const crcT = new Uint32Array(256).map((_, n) => { let c = n; for (let k = 0; k < 8; k++) c = c & 1 ? 0xedb88320 ^ (c >>> 1) : c >>> 1; return c >>> 0; });
const crc = buf => { let c = 0xffffffff; for (const b of buf) c = crcT[(c ^ b) & 255] ^ (c >>> 8); return (c ^ 0xffffffff) >>> 0; };
const chunk = (type, data) => {
  const len = Buffer.alloc(4); len.writeUInt32BE(data.length);
  const td = Buffer.concat([Buffer.from(type), data]);
  const c = Buffer.alloc(4); c.writeUInt32BE(crc(td));
  return Buffer.concat([len, td, c]);
};
const raw = Buffer.alloc((W * 3 + 1) * H);
for (let y = 0; y < H; y++) { raw[y * (W * 3 + 1)] = 0; Buffer.from(px.buffer, y * W * 3, W * 3).copy(raw, y * (W * 3 + 1) + 1); }
const ihdr = Buffer.alloc(13); ihdr.writeUInt32BE(W, 0); ihdr.writeUInt32BE(H, 4); ihdr[8] = 8; ihdr[9] = 2;
fs.writeFileSync(args[1], Buffer.concat([Buffer.from([137, 80, 78, 71, 13, 10, 26, 10]), chunk('IHDR', ihdr),
  chunk('IDAT', zlib.deflateSync(raw)), chunk('IEND', Buffer.alloc(0))]));
console.log(`topdown ${args[1]} ${W}x${H}: layout ${dump.layout}, band z ${zlo}..${zhi}, boxes ${boxes.length}/${dump.boxes.length}, ` +
  `retail brushes ${retail.length}, nodes ${dump.nodes.length} (added ${dump.nodes.filter(n => n.added).length}), marks ${dump.marks.length}`);
console.log(`links crossing mapkit boxes: ${crossing.length}${crossing.length ? ' (' + crossing.join(' ') + ')' : ''}`);
console.log(`traverse links: ${traverses.length} (${traverses.map(([n, m]) => `${n.i}->${m.i}`).join(' ')})`);
if (inferredTraverses) console.log(`legacy dump: ${inferredTraverses} traverses inferred from begin/end node types; regenerate the dump for exact negotiation flags`);
