// Conservative budget for _mapkit.gsc::mapkit_check. No game launch or dependencies.
// Usage: node tools/mapkit/selftest_budget.mjs mods/mapkit/layouts/relay_station.json 4
import fs from 'node:fs';

const [file, speed = '1'] = process.argv.slice(2);
const timescale = Number(speed);
if (!file || !Number.isFinite(timescale) || timescale <= 0) {
  throw new Error('Usage: selftest_budget.mjs <layout.json> [positive timescale]');
}
// Layouts allow // and /* */ comments; quoted asset names and escapes remain intact.
const source = fs.readFileSync(file, 'utf8').replace(/^\uFEFF/, '');
const clean = source.replace(/"(?:\\.|[^"\\])*"|\/\/[^\r\n]*|\/\*[\s\S]*?\*\//g,
  token => token.startsWith('"') ? token : ' ');
const layout = JSON.parse(clean);
const counts = {};
for (const edit of layout.edits) counts[edit.kind] = (counts[edit.kind] || 0) + 1;
const windows = counts.window || 0;
const blockers = layout.empty ? (counts.door || 0) + (counts.debris || 0) : 0;
const levelSeconds = 5 + 25 * ((counts.perk || 0) + (counts.wallbuy || 0)) + 40 * (counts.pap || 0) + 60 * (counts.powerswitch || 0) + 90 * windows + (layout.empty ? 90 * (counts.riser || 0) : 0)
  + blockers * (29 + 90 * windows) + 120 + (layout.empty ? 30 * (counts.mysterybox || 0) : 0);
const autoQuitMs = Math.ceil(levelSeconds * 1000 / timescale) + 45000;
console.log(JSON.stringify({ layout: layout.name, timescale, counts, levelSeconds,
  // mapkit-fix-3c: autoQuitMs counts after init (map load), -TimeoutSec from process start: 420 s startup ceiling on top.
  autoQuitMs, startupCeilingSec: 420, timeoutSec: 420 + Math.ceil(autoQuitMs / 1000) + 60,
  completion: 'mapkit_selftest DONE pass N fail M',
  note: 'Budget assumes the requested timescale is sustained. Require DONE; a timeout or missing marker is incomplete.'
}, null, 2));
