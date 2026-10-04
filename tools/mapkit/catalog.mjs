#!/usr/bin/env node
// Asset catalog for map makers: what the empty base (zombie_pentagon + common_zombie fastfiles) has loaded.
//   node tools/mapkit/catalog.mjs                      wall-buy weapons with their world models, perks, counts
//   node tools/mapkit/catalog.mjs xmodel|material|weapon [substring]   one list, optionally filtered
//   node tools/mapkit/catalog.mjs --refresh <console_mp.log>           rebuild base-catalog.json from a
//     `+set mapkit_catalog 1` run (bo1_mapkit_catalog, src/game_sp/g_mapkit.cpp)
// Names that failed to load on the stock base (doctor-base-assets.json) are left out.
import fs from 'node:fs';
import crypto from 'node:crypto';
const here = f => new URL(f, import.meta.url);
const argv = process.argv.slice(2);

if (argv[0] === '--refresh') {
  const text = fs.readFileSync(argv[1], 'utf8');
  const out = { base: 'zombie_pentagon (mapkit empty base)', evidence: argv[1].replace(/\\/g, '/').split('/').slice(-3).join('/'),
    sha256: crypto.createHash('sha256').update(text).digest('hex'), weapons: {}, xmodels: [], materials: [] };
  for (const line of text.split(/\r?\n/)) {
    const m = line.match(/^mapkit_catalog (weapon|xmodel|material) (\S+)(?: (\S+))?$/);
    if (!m) continue;
    if (m[1] === 'weapon') out.weapons[m[2]] = m[3] === '-' ? null : m[3];
    else if (!/^[*$]/.test(m[2])) out[m[1] + 's'].push(m[2]); // '*' / '$' are engine-generated, not placeable
  }
  if (!Object.keys(out.weapons).length) throw new Error('no mapkit_catalog lines: run with +set mapkit_catalog 1');
  out.xmodels = [...new Set(out.xmodels)].sort(); out.materials = [...new Set(out.materials)].sort();
  fs.writeFileSync(here('./base-catalog.json'), JSON.stringify(out, null, 1));
  console.log(`base-catalog.json: ${Object.keys(out.weapons).length} weapons, ${out.xmodels.length} xmodels, ${out.materials.length} materials`);
  process.exit(0);
}

const cat = JSON.parse(fs.readFileSync(here('./base-catalog.json'), 'utf8'));
const failed = new Set(JSON.parse(fs.readFileSync(here('./doctor-base-assets.json'), 'utf8')).assets);
const ok = (type, name) => !failed.has(`${type}:${name}`);
const [type, filter = ''] = argv;
const match = n => n.toLowerCase().includes(filter.toLowerCase());
if (type === 'weapon') for (const [w, model] of Object.entries(cat.weapons).sort()) { if (match(w)) console.log(`${w}\t${model ?? '-'}`); }
else if (type === 'xmodel' || type === 'material') for (const n of cat[type + 's']) { if (match(n) && ok(type, n)) console.log(n); }
else if (type) { console.error('usage: catalog.mjs [weapon|xmodel|material [substring]] | --refresh <console_mp.log>'); process.exit(2); }
else {
  console.log('Wall buys ({"kind":"wallbuy","weapon":W,"model":M}): weapon  world model');
  for (const [w, model] of Object.entries(cat.weapons).sort())
    if (/_zm$/.test(w) && !/upgraded|_zm_zm$/.test(w) && model && ok('xmodel', model) && cat.xmodels.includes(model)) console.log(`  ${w}\t${model}`);
  console.log('Perks ({"kind":"perk","perk":P}): specialty_quickrevive specialty_armorvest specialty_fastreload specialty_rof');
  console.log(`Also: ${cat.xmodels.length} xmodels, ${cat.materials.length} materials - list with: node tools/mapkit/catalog.mjs xmodel|material [substring]`);
}
