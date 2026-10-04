// One map-maker command. Only tools/headless.ps1 may launch the game.
import fs from 'node:fs';
import path from 'node:path';
import { fileURLToPath } from 'node:url';
import { spawn, spawnSync } from 'node:child_process';
import { parseLayout, analyzeStatic, loadCatalog, finding, center, editName } from './doctor-static.mjs';
import { analyzeLive, analyzeClient } from './doctor-live.mjs';

const root = path.resolve(path.dirname(fileURLToPath(import.meta.url)), '../..');
const argv = process.argv.slice(2), file = argv[0];
if (!file || file.startsWith('--') || argv.slice(1).some(a => !['--static', '--help'].includes(a))) {
  console.error('Usage: node tools/mapkit/doctor.mjs <layout.json> [--static]'); process.exit(2);
}
const source = path.resolve(file);
const name = path.basename(source, path.extname(source)).replace(/[^\w-]/g, '_').slice(0, 63) || 'layout';
const out = path.join(root, 'build/doctor', name);
const runId = `run-${new Date().toISOString().replace(/[:.]/g, '-')}-${process.pid}`;
fs.mkdirSync(out, { recursive: true });
const artifact = suffix => path.join(out, `${runId}-${suffix}`);
const relative = suffix => `${runId}-${suffix}`;
let layout, stat = { findings: [], windows: [], zones: [], nodes: [], boxes: [] }, live, dump, wrapperExit, budget;
const files = {}, errors = [];
console.log(`Doctor: ${source}\nReport: ${path.join(out, 'report.md')}`);
try {
  const raw = fs.readFileSync(source, 'utf8');
  fs.writeFileSync(artifact('layout.json'), raw);
  layout = parseLayout(raw);
} catch (error) { errors.push(finding('json', 'FAIL', '$', null, error.message, 'Correct the JSON syntax at the reported line/column or ensure the input file is readable.')); }
if (layout) {
  let catalog;
  try { catalog = loadCatalog(); }
  catch (error) { errors.push(finding('catalog-unavailable', 'WARN', '$.base', null, error.message, 'Provide the read-only Five/common_zombie fastfiles in the Steam zone directory; asset checks were not performed.')); }
  try { stat = analyzeStatic(layout, catalog); }
  catch (error) { errors.push(finding('static-incomplete', 'FAIL', '$.edits', null, error.message, 'Reduce geometry/node count or correct the indicated field, then rerun.')); }
  console.log(`Static: ${stat.nodes.length} estimated walking nodes; ${stat.findings.filter(f => f.severity === 'FAIL').length} FAIL, ${stat.findings.filter(f => f.severity === 'WARN').length} WARN.`);
  if (catalog) fs.writeFileSync(artifact('catalog.json'), JSON.stringify({ sources: catalog.sources, strings: catalog.strings.size, materials: catalog.materials.size, note: 'Identifier presence catalog; live load/purchase checks establish typed asset availability.' }, null, 2));
}

if (layout && !errors.some(e => e.severity === 'FAIL') && !stat.findings.some(f => f.code === 'schema') && !argv.includes('--static')) {
  const counts = Object.fromEntries(['window', 'perk', 'wallbuy', 'door', 'debris', 'mysterybox', 'pap', 'powerswitch'].map(k => [k, layout.edits.filter(e => e.kind === k).length]));
  // mapkit-fix-4c: boardshots photograph boarded windows only (open entrances have no boards); a riser waits like a window
  // mapkit-fix-4d: and only windows the kit places (an "origin"): a retail window re-targeted on the full base ("target",
  // no origin) is not photographed by _mapkit.gsc mapkit_boardshots, so mapkit_test failed boardshots-incomplete
  const boarded = layout.edits.filter(e => e.kind === 'window' && e.open !== true && Array.isArray(e.origin)).length;
  const risers = layout.empty ? layout.edits.filter(e => e.kind === 'riser').length : 0;
  const levelSeconds = 5 + 25 * (counts.perk + counts.wallbuy) + 40 * counts.pap + 60 * counts.powerswitch + 90 * counts.window + 90 * risers
    + (layout.empty ? (counts.door + counts.debris) * (29 + 90 * counts.window) + 30 * counts.mysterybox : 0) + 120;
  budget = { levelSeconds, timescale: 4, photoSeconds: counts.window * 5 + 5,
    autoQuitMs: Math.ceil(levelSeconds * 250) + 45000 + counts.window * 5000 + 5000 };
  // mapkit-fix-3c: bo1_autoquit counts from the end of init (map load included; win_main.cpp s_autoquitStartMs), the
  // wrapper's -TimeoutSec from process start. Init took ~120 s under load (3b handoff; 60 s allowed -> exit 21), so the
  // wrapper gets a startup ceiling on top. The checks' own budget (autoQuitMs after the init line) is unchanged.
  budget.startupCeilingSec = 420;
  budget.timeoutSec = budget.startupCeilingSec + Math.ceil(budget.autoQuitMs / 1000) + 60;
  const modName = `mapkit_doctor_${Date.now()}_${process.pid}`;
  const mod = path.join(root, 'build/Release/mods', modName);
  try {
    if (!fs.existsSync(path.join(root, 'build/Release/BO1Zombies.exe'))) throw new Error('Build Release first and run setup.ps1 (README.md).');
    fs.cpSync(path.join(root, 'mods/mapkit'), mod, { recursive: true });
    // Isolated mod copy: the input, shipped layouts and another run's scripts remain untouched.
    fs.writeFileSync(path.join(mod, 'layouts', `${name}.json`), JSON.stringify(layout, null, 2));
    const commands = `+set fs_game mods/${modName} +set bo1_mod_mapkit ${name} +set sv_cheats 1 +set timescale 4 +set bo1_testclient 1 +set bo1_testclient_fight 1 +set bo1_testclient_god 1 +set bo1_testclient_passive 1 +set mapkit_selftest 1 +set mapkit_doctor 1 +set bo1_scripterrors 100 +devmap zombie_pentagon`;
    files.console = relative('console.log'); files.wrapper = relative('wrapper.log');
    fs.writeFileSync(artifact('run.json'), JSON.stringify({ source, modName, commands, budget }, null, 2));
    console.log(`Live: one private-desktop Client run, 4x checks then 1x photos; budget ${budget.autoQuitMs / 1000}s real, timeout ${budget.timeoutSec}s. Stops after DONE + photos.`);
    const args = ['-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', path.join(root, 'tools/mapkit/doctor-run.ps1'),
      '-Commands', commands, '-AutoQuitMs', String(budget.autoQuitMs), '-TimeoutSec', String(budget.timeoutSec), '-SaveLog', artifact('console.log')];
    const start = Date.now();
    do {
      wrapperExit = await run('powershell.exe', args, artifact('wrapper.log'), path.join(mod, 'console_mp.log'));
      if (wrapperExit === 10 && Date.now() - start < 900000) { console.log('Headless slot busy; retrying in 30 seconds.'); await new Promise(resolve => setTimeout(resolve, 30000)); }
      else break;
    } while (true);
    const log = fs.existsSync(artifact('console.log')) ? fs.readFileSync(artifact('console.log'), 'utf8') : '';
    for (const candidate of ['mapkit_doctor_dump.json', 'mapkit_dump.json']) if (fs.existsSync(path.join(mod, candidate))) {
      dump = JSON.parse(fs.readFileSync(path.join(mod, candidate), 'utf8')); break;
    }
    live = analyzeLive(layout, stat, log, dump, wrapperExit, { startupCeilingSec: budget.startupCeilingSec });
    console.log(`Run: wrapper exit ${wrapperExit}; ${!live.started ? `STARTUP TIMEOUT (no init line within the ${budget.startupCeilingSec} s startup ceiling) - not a failed check` : live.done ? `self-test DONE pass ${live.done.pass} fail ${live.done.fail}` : 'game loaded but the self-test DONE is missing - checks did not finish'}.`);
    for (const photo of live.photos) {
      const from = path.join(mod, 'screenshots', `bo1_${photo.shot}.jpg`);
      if (fs.existsSync(from)) { photo.file = relative(`window-${photo.edit}.jpg`); fs.copyFileSync(from, path.join(out, photo.file)); }
      else errors.push(finding('screenshot-missing', 'WARN', `$.edits[${photo.edit}]`, center(layout.edits[photo.edit]), 'Camera was scheduled but no fresh JPEG was written in this run.', 'Inspect wrapper.log for the client state; rerun the complete doctor route.'));
    }
    // mapkit-fix-4b: the real-client pass of the mergecheck mapkit lane (1x -Client: boarded windows drawn, torn and rebuilt,
    // every kit door fired at, then the test player walks into every machine), so a map maker sees what a player sees
    const machines = counts.perk + counts.pap + counts.powerswitch + counts.mysterybox;
    if (live.started && !live.findings.some(f => f.code === 'startup-timeout') && (counts.window || machines)) {
      const g = `${modName}-client-games.log`;
      const clientCommands = `+set fs_game mods/${modName} +set bo1_mod_mapkit ${name} +set g_log ${g} +set bo1_testclient 1 +set bo1_testclient_passive 1 +set bo1_testclient_god 1 ${boarded ? '+set mapkit_boardshots 1 ' : ''}+set mapkit_clipshots 1 +set bo1_scripterrors 100 +devmap zombie_pentagon`;
      const clientArgs = ['-NoProfile', '-ExecutionPolicy', 'Bypass', '-File', path.join(root, 'tools/mapkit/doctor-run.ps1'),
        '-Commands', clientCommands, '-AutoQuitMs', '240000', '-TimeoutSec', String(budget.startupCeilingSec + 240 + 60), '-SaveLog', artifact('client-console.log')];
      console.log('Client pass: one 1x private-desktop Client run (boards drawn / rebuilt, door impacts, machine clips).');
      let clientExit;
      const clientStart = Date.now();
      do {
        clientExit = await run('powershell.exe', clientArgs, artifact('client-wrapper.log'), path.join(mod, 'console_mp.log'));
        if (clientExit === 10 && Date.now() - clientStart < 900000) await new Promise(resolve => setTimeout(resolve, 30000));
        else break;
      } while (true);
      const readText = f => fs.existsSync(f) ? fs.readFileSync(f, 'utf8') : '';
      const client = analyzeClient(layout, readText(path.join(mod, g)), readText(artifact('client-console.log')), clientExit, { boardshots: !!boarded });
      live.findings.push(...client.findings);
      live.client = client;
      console.log(`Client pass: exit ${clientExit}; boarded shots ${client.boarded.length} (drawn ${client.boarded.map(b => b.drawn).join('/') || '-'}), rebuild ${client.rebuild ? client.rebuild.join(' -> ') : 'none'}, machine clips ${client.clips ? `${client.clips.pass}/${client.clips.machines}` : 'untested'}.`);
    }
    fs.writeFileSync(artifact('events.json'), JSON.stringify({ paths: live.pathEvents, stuck: live.stuckEvents, baseAssetErrors: live.baseAssetErrors, scriptErrors: live.scriptErrors, windows: live.windows, zones: live.zones, checks: live.checks, photos: live.photos, client: live.client }, null, 2));
  } catch (error) { errors.push(finding('run-unavailable', 'FAIL', '$', null, error.message, 'Build Release, run setup.ps1 and inspect the wrapper log; launch only through tools/headless.ps1.')); }
} else if (argv.includes('--static')) errors.push(finding('live-not-run', 'WARN', '$', null, '--static requested: live behavior and screenshots were not tested.', 'Run the same command without --static for the complete diagnosis.'));

const findings = [...stat.findings, ...(live?.findings || []), ...errors];
// Definite authoring faults precede cascading runtime symptoms at the same severity.
const priority = f => f.code === 'schema' || f.code === 'json' ? 0 : stat.findings.includes(f) ? 1 : 2;
findings.sort((a, b) => ({ FAIL: 0, WARN: 1 }[a.severity] - { FAIL: 0, WARN: 1 }[b.severity]) || priority(a) - priority(b) || a.path.localeCompare(b.path) || a.code.localeCompare(b.code));
findings.forEach((f, i) => f.rank = i + 1);
let status = findings.some(f => f.severity === 'FAIL') ? 'FAIL' : findings.length ? 'WARN' : live?.done && live.ended ? 'PASS' : 'WARN';
if (stat.boxes.length || dump) {
  if (!dump) {
    const points = stat.boxes.flatMap(b => [b.mins, b.maxs]);
    dump = { layout: layout.name, bounds: [0, 1].map(side => [0, 1, 2].map(a => (side ? Math.max : Math.min)(...points.map(p => p[a])) + (side ? 128 : -128))),
      boxes: stat.boxes.filter(b => b.contents !== 'none'), nodes: stat.nodes.map(n => ({ ...n, traverses: [] })), brushes: [], marks: [] };
  }
  fs.writeFileSync(artifact('dump.json'), JSON.stringify(dump));
  fs.writeFileSync(artifact('findings.json'), JSON.stringify(findings));
  const scale = Math.min(1, 1800 / Math.max(dump.bounds[1][0] - dump.bounds[0][0], dump.bounds[1][1] - dump.bounds[0][1]));
  const r = spawnSync(process.execPath, [path.join(root, 'tools/mapkit/topdown.mjs'), artifact('dump.json'), artifact('topdown.png'), '--scale', String(scale), '--zlo', String((stat.lowestFloor ?? 16) + 16), '--zhi', String((stat.lowestFloor ?? 16) + 64), '--findings', artifact('findings.json')], { encoding: 'utf8', cwd: root, windowsHide: true });
  fs.writeFileSync(artifact('topdown.txt'), r.stdout + r.stderr);
  if (r.status === 0) files.topdown = relative('topdown.png');
  else {
    findings.push(finding('topdown-failed', 'WARN', '$', null, 'Topdown generation failed; see topdown.txt.', 'Check dump bounds and rerun.'));
    if (status === 'PASS') status = 'WARN';
  }
}

const report = renderReport();
fs.writeFileSync(path.join(out, 'report.md'), report);
fs.writeFileSync(artifact('report.md'), report);
fs.writeFileSync(artifact('result.json'), JSON.stringify({ status, source, budget, wrapperExit, findings, live }, null, 2));
console.log(report);
process.exitCode = status === 'FAIL' ? 1 : status === 'WARN' ? 2 : 0;

async function run(command, args, logFile, gameLog) {
  return await new Promise((resolve, reject) => {
    const fd = fs.openSync(logFile, 'a');
    const child = spawn(command, args, { cwd: root, windowsHide: true, stdio: ['ignore', fd, fd] });
    // mapkit-fix-3c: report the game's own startup markers as they appear (BUDGET = level script started, init line = map loaded).
    const start = Date.now(), marks = [[/mapkit_selftest BUDGET/, 'level script started (map loading)'], [/bo1_headless(?:_client)?: (?:listen server|dedicated)/, 'game loaded (init line; the run budget starts)']];
    const watch = setInterval(() => {
      if (!marks.length || !gameLog || !fs.existsSync(gameLog)) return;
      let text = ''; try { text = fs.readFileSync(gameLog, 'latin1'); } catch { return; }
      while (marks.length && marks[0][0].test(text)) console.log(`Startup: ${marks.shift()[1]} after ${Math.round((Date.now() - start) / 1000)} s.`);
    }, 2000);
    let beat = 0;
    const timer = setInterval(() => { if (++beat % 2 === 0 || marks.length) console.log(`Doctor is running (${Math.round((Date.now() - start) / 1000)} s${marks.length ? `, waiting for: ${marks[0][1]}` : ''}); ${path.basename(logFile)} records wrapper progress.`); }, 30000);
    child.once('error', error => { clearInterval(timer); clearInterval(watch); fs.closeSync(fd); reject(error); });
    child.once('exit', code => { clearInterval(timer); clearInterval(watch); fs.closeSync(fd); resolve(code ?? -1); });
  });
}

function renderReport() {
  const esc = x => String(x ?? '—').replace(/\|/g, '\\|').replace(/\r?\n/g, '<br>');
  const pos = p => p ? p.map(v => Number(v.toFixed(2))).join(', ') : 'not localized';
  const lines = [`# ${status} — ${layout?.name || name}`, '', `Input: \`${source}\`. ${new Date().toISOString()}.`, '',
    `Static: ${stat.findings.length} findings. Live: ${live ? `wrapper ${wrapperExit}; DONE ${live.done ? `${live.done.pass} PASS / ${live.done.fail} FAIL` : 'missing'}; ${live.actors} actor lives / ${live.sampleCount} samples; ${live.errorCount} script-error log entries; round ${live.round?.join(' → ') || 'unverified'}. Functional route 4x; post-DONE photos 1x.` : 'not completed'}`, '',
    '## Ranked problems', ''];
  if (!findings.length) lines.push('No problems observed in this route.');
  for (const f of findings) lines.push(`### ${f.rank || findings.indexOf(f) + 1}. ${f.severity} — ${f.code}`, '',
    `**WHERE:** \`${f.path}\` at (${pos(f.position)}).`, '', `**WHAT:** ${esc(f.observed)}`, '', `**SUGGESTED FIX:** ${esc(f.fix)}`, '');
  if (live) {
    lines.push('## Windows', '', 'Times are first observed elapsed seconds from that actor’s first sample (0.5s sampling). Climbed = retail completed-emerging flag; entered = geometric room crossing; tore = retail chunk-destruction timestamp, not proximity. Later actors may legitimately tear no boards.', '',
      '| Window / JSON path | Spawned | At window | Tore boards | Climbed | Entered | Reached player | Boards torn | First step seconds: arrival / tear / climb / player |', '|---|---:|---:|---:|---:|---:|---:|---:|---|');
    for (const w of live.windows) lines.push(`| ${esc(editName(w, w.i))} / $.edits[${w.i}] | ${w.spawned} | ${w.arrived} | ${w.tore} | ${w.climbed} | ${w.entered} | ${w.reached} | ${w.boards} | ${['arrived', 'tore', 'climbed', 'reached'].map(k => w.times[k]?.length ? (Math.min(...w.times[k]) / 1000).toFixed(2) : '—').join(' / ')} |`);
    lines.push('', 'Last sampled positions for lives without a player-reach witness (may include actors killed by the test route):', '', '| Window | Actor / life | Last position | Nearest node | Last stage |', '|---|---|---|---:|---|');
    for (const w of live.windows) for (const a of w.stopped) lines.push(`| ${esc(editName(w, w.i))} | ${a.ent} / ${a.serial} | ${pos(a.position)} | ${a.node ?? 'none'} | ${a.stage} |`);
    lines.push('', '## Zones', '', '| Zone / JSON path | Offline spawn route, doors open | Live graph route | Player visited by walking route | Observed zombie lives |', '|---|---|---|---|---:|');
    for (const z of live.zones) lines.push(`| ${esc(z.name)} / $.edits[${z.i}] | ${z.staticReachable ? 'yes' : 'no'} | ${z.liveGraphReachable === undefined ? 'unverified' : z.liveGraphReachable ? 'yes' : 'no'} | ${z.playerInside ? 'yes' : 'no'} | ${z.zombieCount} |`);
    lines.push('', '## Purchases and round checks', '', '| Placement | Result | Observation |', '|---|---|---|');
    for (const c of live.checks.filter(c => ['perk', 'wallbuy', 'door', 'debris', 'mysterybox', 'round', 'pap', 'powerswitch'].includes(c.kind))) lines.push(`| ${esc(c.kind + ' ' + c.name)} ${c.edit >= 0 ? `$.edits[${c.edit}]` : ''} | ${c.status} | ${esc(c.detail)} |`);
    lines.push('', `All ${live.pathEvents.length} no-path events (${live.pathEvents.filter(e => e.resolution).length} with recovery, cleanup or post-DONE context) and ${live.stuckEvents.length} stationary episodes (>=15 level seconds, <=16 units movement, no board/stage progress) are retained with position, nearest node, evidence and log line where available in [events.json](${relative('events.json')}).`,
      `${live.baseAssetErrors.filter(e => e.stockBaseline).length} unreferenced asset-load messages match the stock Five client baseline; these remain informational in events.json. Authored asset failures and new messages remain ranked findings.`, '');
  }
  if (files.topdown) lines.push('## Topdown', '', `![Problem ranks circled on the map](${files.topdown})`, '');
  if (live?.photos.length) {
    lines.push('## Window views', '', 'Camera was placed 160 units inside each window after all tests, at 1x; these images do not establish walking reachability.', '');
    for (const photo of live.photos.filter(p => p.file)) lines.push(`### ${esc(editName(layout.edits[photo.edit], photo.edit))} — $.edits[${photo.edit}]`, '', `![Player-side window view](${photo.file})`, '');
  }
  lines.push('## Evidence and limits', '', stat.assumptions || 'Static geometry coverage is limited to the authored layout.', '',
    'Asset catalog checks identifier presence in read-only Five/common_zombie fastfiles; purchases and load errors verify runtime use. The route grants funds/power, opens doors and kills remaining zombies for round advancement. PASS covers this functional route, not every possible combat scenario.', '',
    `[Input snapshot](${relative('layout.json')})${files.console ? ` · [Console](${files.console}) · [Wrapper](${files.wrapper})` : ''}. Exit codes: 0 PASS, 1 FAIL, 2 WARN.`, '');
  return lines.join('\n');
}
