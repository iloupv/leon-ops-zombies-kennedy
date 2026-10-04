import { center, contains, distance, editName, editPath, finding, segmentBlocked } from './doctor-static.mjs';
import fs from 'node:fs';
const baseAssets = JSON.parse(fs.readFileSync(new URL('./doctor-base-assets.json', import.meta.url), 'utf8'));

const vectorPattern = '\\((-?[\\d.e+]+),\\s*(-?[\\d.e+]+),\\s*(-?[\\d.e+]+)\\)';
const vector = m => m.slice(0, 3).map(Number);
export const nearestNode = (nodes, p) => p && nodes.filter(n => n.type !== 0).reduce((best, n) => !best || distance(n.origin, p) < distance(best.origin, p) ? n : best, null);

export function analyzeLive(layout, stat, text, dump, exitCode, { stuckSeconds = 15, startupCeilingSec } = {}) {
  const findings = [], actors = new Map(), pathEvents = [], stuckEvents = [], photos = [], checks = [], baseAssetErrors = [], scriptErrors = [];
  const scriptGroups = new Map();
  const spawns = [];
  const culls = [];
  const windows = stat.windows.map(w => ({ ...w, spawned: 0, arrived: 0, tore: 0, climbed: 0, entered: 0, reached: 0, boards: 0, times: {}, stopped: [] }));
  const zones = stat.zones.map(z => ({ ...z, actors: new Set(), playerInside: false }));
  const nodes = dump?.nodes || stat.nodes;
  const add = (code, e, observed, fix, severity = 'FAIL', position = center(e)) => findings.push(finding(code, severity, e.i === undefined ? '$' : editPath(e.i), position, observed, fix));
  let player, playerTime = -Infinity, purchase, done, round, begun = false, ended = false, sampleCount = 0, started = false;
  let errorCount = 0, playerSamples = 0;
  const note = (w, step, a, time) => {
    if (!a[step]) { a[step] = true; w[step]++; (w.times[step] ||= []).push(time - a.birth); }
  };
  const actorRE = new RegExp(`mapkit_doctor actor (\\d+) (\\d+) (\\d+) (-?\\d+) ${vectorPattern} got (\\d+) tore (-?\\d+) emerged (\\d+)(?: target (\\S+))?( ground 1)?`);
  const playerRE = new RegExp(`mapkit_doctor player (\\d+) ${vectorPattern}`);
  const pathRE = new RegExp(`mapkit_doctor path (\\d+) (\\d+) (-?\\d+) ${vectorPattern} goal (.*)`);
  const nativePathRE = /AI \(entity (\d+), origin ([-\d.]+) ([-\d.]+) ([-\d.]+)\) couldn't find path to goal/;
  const lines = text.split(/\r?\n/);
  if (lines.filter(line => /mapkit_doctor BEGIN /.test(line)).length > 1)
    add('map-restarted', {}, 'Multiple doctor BEGIN markers occurred in one launch; actor lives/checks cannot be safely combined.', 'Inspect the map restart/error in console.log and rerun a single uninterrupted route.');
  for (let lineIndex = 0; lineIndex < lines.length; lineIndex++) {
    const line = lines[lineIndex];
    let m;
    // mapkit-fix-3d: the native no-path line has no level time; it gets the time of the next timed line (its frame).
    if ((m = line.match(/^(?:bo1_[a-z_]+:.*?\btime (\d+)|mapkit_doctor (?:actor|player|path|cull) (\d+))/)))
      for (let k = pathEvents.length - 1; k >= 0 && pathEvents[k].frameTime === undefined; k--) pathEvents[k].frameTime = +(m[1] ?? m[2]);
    // mapkit-fix-3c: the game's own init line (win_main.cpp, printed where bo1_autoquit starts counting, after the map
    // load) or the doctor's BEGIN (GSC runs only on a loaded map) = startup finished.
    if (/bo1_headless(?:_client)?: (?:listen server|dedicated)/.test(line)) started = true;
    if (/mapkit_doctor BEGIN /.test(line)) { begun = started = true; continue; }
    if (/mapkit_doctor END/.test(line)) ended = true;
    if ((m = line.match(/bo1_actor_spawn: time (\d+) total \d+ actor \d+ ent (\d+)/))) spawns.push({ time: +m[1], ent: +m[2], logLine: lineIndex + 1 });
    if ((m = line.match(new RegExp(`mapkit_doctor cull (\\d+) (\\d+) ${vectorPattern} (\\S+)`)))) culls.push({ time: +m[1], ent: +m[2], position: vector(m.slice(3)), reason: m[6], logLine: lineIndex + 1 });
    if ((m = line.match(/mapkit_doctor purchase (\d+)/))) purchase = Number(m[1]);
    if ((m = line.match(/mapkit_selftest DONE pass (\d+) fail (\d+)/))) done = { pass: +m[1], fail: +m[2] };
    if ((m = line.match(/mapkit: check round PASS round (\d+) -> (\d+)/))) round = [+m[1], +m[2]];
    if ((m = line.match(playerRE))) {
      playerSamples++;
      playerTime = +m[1]; player = vector(m.slice(2));
      if (!done) for (const z of zones) if (contains(z, player, 0.5)) z.playerInside = true;
    }
    if ((m = line.match(actorRE)) && !done) {
      const [time, serial, ent, source] = m.slice(1, 5).map(Number), p = vector(m.slice(5));
      const got = +m[8], tore = +m[9], emerged = +m[10];
      sampleCount++;
      let a = actors.get(serial);
      if (!a) { a = { serial, ent, source, birth: time, firstLine: lineIndex + 1, samples: [], last: p, anchor: p, anchorTime: time, lastTore: tore, stopped: false }; actors.set(serial, a); }
      a.samples.push({ time, logLine: lineIndex + 1, position: p });
      const w = windows.find(w => w.i === source);
      if (w) {
        note(w, 'spawned', a, time);
        const atSource = !m[11] || m[11] === `mkwin${source}`;
        if (got && atSource) note(w, 'arrived', a, time);
        if (tore >= 0 && atSource) note(w, 'tore', a, tore);
        if (!atSource && m[11] !== 'none') a.retargeted = m[11];
        // Keep geometric entry separate from the retail completed-traverse witness.
        const room = layout.edits.find(e => e.kind === 'room' && e.name === w.room) || zones.find(z => z.name === w.zone);
        const inside = room && contains(room, p, 0.5);
        if (inside) note(w, 'entered', a, time);
        if (emerged && atSource) note(w, 'climbed', a, time);
        if (inside && player && Math.abs(time - playerTime) <= 500 && distance(player, p) < 64) note(w, 'reached', a, time);
      }
      for (const z of zones) if (contains(z, p, 0.5)) z.actors.add(serial);
      if (distance(p, a.anchor) > 16 || tore > a.lastTore || a.lastGot !== got || a.lastEmerged !== emerged) {
        a.anchor = p; a.anchorTime = time; a.stopped = false;
      } else if (!a.stopped && time - a.anchorTime >= stuckSeconds * 1000 && !a.reached) {
        a.stopped = true;
        stuckEvents.push({ time, serial, ent, source, position: p, node: nearestNode(nodes, p)?.i, seconds: (time - a.anchorTime) / 1000 });
      }
      // mapkit-fix-4c: m[12] = ' ground 1' while a riser is still in the ground (retail rise starts 45 below the spot)
      if (!a.fell && !m[12] && p[2] < (stat.lowestFloor ?? -Infinity) - 24) {
        a.fell = true;
        const e = w || { i: source >= 0 ? source : undefined, origin: p };
        add('actor-below-floor', e, `Actor ${ent} life ${serial} at ${JSON.stringify(p)} is below lowest authored floor ${stat.lowestFloor}.`, 'Close the floor hole at these XY coordinates with a solid box whose top matches room mins[2]; inspect nearby open sides.', 'FAIL', p);
      }
      a.last = p; a.time = time; a.lastTore = tore; a.lastGot = got; a.lastEmerged = emerged;
    }
    if ((m = line.match(new RegExp(`mapkit_doctor reach (\\d+) (\\d+) (\\d+) (\\d+) ${vectorPattern}`))) && !done) {
      const a = actors.get(+m[2]), w = windows.find(w => w.i === +m[4]);
      if (a && w && a.ent === +m[3] && a.source === w.i) {
        note(w, 'entered', a, +m[1]); note(w, 'reached', a, +m[1]);
      }
    }
    if ((m = line.match(pathRE))) pathEvents.push({ time: +m[1], ent: +m[2], source: +m[3], position: vector(m.slice(4)), goal: m[7], kind: 'bad_path', logLine: lineIndex + 1, phase: done ? 'photos' : 'checks' });
    if ((m = line.match(nativePathRE))) pathEvents.push({ ent: +m[1], position: vector(m.slice(2)), kind: 'no-path', logLine: lineIndex + 1, phase: done ? 'photos' : 'checks' });
    if (/BO1_FARENT/.test(line)) {
      const v = line.match(/origin \(([-\d.]+) ([-\d.]+) ([-\d.]+)\)/);
      const p = v ? vector(v.slice(1)) : null;
      const w = p && windows.reduce((best, w) => w.origin && (!best || distance(w.origin, p) < distance(best.origin, p)) ? w : best, null);
      add('far-entity', w || {}, line.trim(), 'Inspect the logged entity and its spawn/floor position; close uncovered exterior edges.', 'FAIL', p);
    }
    if ((m = line.match(/mapkit_doctor photo (\d+) (\d+) (.*) timescale (.*)/))) photos.push({ edit: +m[1], shot: +m[2], position: m[3], speed: m[4] });
    if ((m = line.match(/mapkit: check (\S+) (.*?)\b(PASS|FAIL|SKIP)\b(.*)/))) {
      const [, kind, label, status, detail] = m;
      const name = label.trim();
      let edit = layout.edits.findIndex((e, i) => (e.kind === kind || (kind.startsWith('unlock') && ['zone', 'window'].includes(e.kind))) && editName(e, i) === name.replace(/ boards$/, ''));
      if (['perk', 'wallbuy', 'pap'].includes(kind) && purchase !== undefined) edit = purchase;
      if (kind === 'door_graph') edit = Number(detail.match(/edit (\d+)/)?.[1] ?? -1);
      const e = edit >= 0 ? { ...layout.edits[edit], i: edit } : {};
      checks.push({ kind, name, status, detail: detail.trim(), edit });
      if (kind === 'window' && name.endsWith(' boards')) {
        const w = windows.find(w => w.i === edit), count = detail.match(/(\d+)\/(\d+)/);
        if (w && count) w.boards = +count[1];
      }
      if (status !== 'PASS') add('selftest-' + kind, e, `${kind} ${name}: ${status} ${detail.trim()}`, purchaseFix(kind, e), status === 'FAIL' ? 'FAIL' : 'WARN');
    }
    if (/script runtime error #|throwing script exception|Unknown function|script compile error|script link error/i.test(line)) {
      errorCount++;
      const context = lines.slice(lineIndex, lineIndex + 12).join('\n');
      const location = context.match(/file '([^']+)', line (\d+)/);
      const key = location ? `${location[1]}:${location[2]}` : line.replace(/#\d+/g, '#N');
      scriptErrors.push({ location: key, logLine: lineIndex + 1, context });
      if (!scriptGroups.has(key)) scriptGroups.set(key, { count: 0, context });
      scriptGroups.get(key).count++;
    }
    if ((m = line.match(/Could not load (xmodel|material|weapon).*?"([^"]+)"/))) {
      const asset = m[2];
      const edit = layout.edits.findIndex(e => ['material', 'floorMaterial', 'wallMaterial', 'ceilingMaterial', 'model', 'weapon'].some(k => e[k] === asset));
      if (edit >= 0) add('load-error', { ...layout.edits[edit], i: edit }, line.trim(), `Replace ${asset} with a loaded asset from the catalog at this placement.`);
      else baseAssetErrors.push({ asset, type: m[1], stockBaseline: layout.base === baseAssets.base && baseAssets.assets.includes(`${m[1]}:${asset}`), message: line.trim(), logLine: lineIndex + 1 });
    }
    if (/mapkit: cannot read|mapkit:.*more than.*dropped/.test(line)) add('load-error', {}, line.trim(), 'Correct the layout path or reduce the authored geometry count.');
  }
  // mapkit-fix-3: exit 21 = tools/headless.ps1's watchdog hit -TimeoutSec. With DONE and END logged the checks all finished
  // and only the quit came late (a slow load while other runs hold the CPU, diaries mapmaker-1 R05 / mapmaker-2 D7): WARN.
  // mapkit-fix-3c: exit 21 before the init line = the machine never finished loading the map (startup ~120 s under load
  // on 2026-09-27); no layout check ran, so the check findings below would only cascade from that.
  const summary = () => ({ findings, windows, zones: zones.map(z => ({ ...z, zombieCount: z.actors.size, actors: undefined })), checks, pathEvents, stuckEvents, culls, baseAssetErrors, scriptErrors, photos, done, round, errorCount, sampleCount, playerSamples, actors: actors.size, ended, started });
  if (exitCode === 21 && !started) {
    add('startup-timeout', {}, `STARTUP TIMEOUT, not a failed check: the game never logged its init line (map loaded) before the wrapper's ceiling${startupCeilingSec ? ` (${startupCeilingSec} s for startup on top of the run budget)` : ''}; no layout check ran.`, 'Nothing is known about the layout yet: rerun when fewer games run on the machine; if it repeats, read wrapper.log / console.log for where init stopped.');
    return summary();
  }
  if (exitCode === 21 && done && ended) add('run-timeout-late', {}, 'Headless wrapper exited 21 (timed out) after the self-test DONE and doctor END were logged.', 'Nothing to fix in the layout; the run was slow to quit (machine under load). Re-run if in doubt.', 'WARN');
  else if (exitCode !== 0) add('run-exit', {}, exitCode === 21 ? 'Headless wrapper exited 21 (timed out) AFTER the game loaded (init line seen) and before the self-test DONE / doctor END: a check hung or failed to finish, not a slow startup.' : `Headless wrapper exited ${exitCode}.`, 'Read wrapper.log and console.log; fix the crash/script error or increase the computed budget if simulation ran below 4x.');
  if (!begun || !playerSamples) add('telemetry-missing', {}, 'Doctor observer/player telemetry is missing.', 'Rebuild the executable with doctor support, check script errors and window/spawner zone wiring, then rerun.');
  if (!done) add('incomplete', {}, 'Self-test DONE marker is missing; checks did not finish.', 'Fix the first script/load failure or allow the computed time budget. An exit code of 0 alone is not completion.');
  if (!ended) add('photos-incomplete', {}, 'Post-test screenshot route did not finish.', 'Check script errors and wrapper timeout; require mapkit_doctor END and fresh photos.', 'WARN');
  if (!round) add('round-unverified', {}, 'No successful round advancement check was recorded.', 'Ensure spawners are wired to active zones, all zombies can enter, and the self-test reaches its round check.');
  for (const [location, group] of scriptGroups) add('script-error', {}, `${group.count} script-error entries at ${location}. First context:\n${group.context}`, 'Correct the named script file:line. If it is retail script, fix the earlier layout zone/target findings first, then attach console.log to a kit bug report if it persists. All occurrences remain in events.json.');
  const unexpectedAssets = baseAssetErrors.filter(e => !e.stockBaseline);
  if (unexpectedAssets.length) add('base-asset-warnings', {}, `${unexpectedAssets.length} asset-load errors are absent from the stock Five client baseline and not explicitly referenced by this layout (including ${unexpectedAssets.slice(0, 3).map(e => e.asset).join(', ')}). All names/log lines are in events.json.`, 'Inspect screenshots and base asset setup; attach console.log to a kit issue if anything is missing. Do not rename unrelated layout assets.', 'WARN');
  for (const w of windows) {
    for (const a of actors.values()) if (a.source === w.i && !a.reached) w.stopped.push({ serial: a.serial, ent: a.ent, position: a.last, node: nearestNode(nodes, a.last)?.i, lastLevelMs: a.time, stage: a.entered ? 'inside' : a.tore ? 'tearing' : a.arrived ? 'at window' : 'outside' });
    if (!w.reached) add('window-no-reach', w, `${w.spawned} observed spawns, ${w.arrived} at entrance, ${w.tore} tore boards, ${w.climbed} completed traverse, ${w.entered} entered room, ${w.reached} reached player.`, w.spawned ? 'Inspect the last positions and nearest nodes below; connect exterior nodes to the traverse and its interior landing to the player zone, clearing solids along that route.' : `Add/fix a spawner with zone ${w.zone || '<interior zone>'} behind this window; make this its closest window and ensure its zone becomes active.`);
  }
  for (const z of zones) {
    if (!z.playerInside) add('zone-player-unvisited', z, 'The walking test client never occupied this zone before DONE.', 'Connect this zone through a buyable doorway and put a reachable window/purchase in it for the self-test route.');
    if (!z.actors.size) add('zone-no-zombies', z, 'No sampled zombie occupied this zone.', 'Connect its window landing and spawner zone to the active play area.', 'WARN');
  }
  // mapkit-fix-4c: every riser must report a climb-out that reached the player (mapkit: check riser NAME PASS)
  if (layout.empty) for (const e of layout.edits.map((e, i) => ({ ...e, i })).filter(e => e.kind === 'riser'))
    if (!checks.some(c => c.edit === e.i && c.kind === 'riser')) add('riser-unverified', e, 'No riser result: the self-test did not reach this riser.', 'Check script errors and that the riser zone becomes active (a zone the test client can walk into).');
  for (const e of layout.edits.map((e, i) => ({ ...e, i })).filter(e => ['perk', 'wallbuy', 'door', 'debris', 'mysterybox', 'pap', 'powerswitch'].includes(e.kind)))
    if (!checks.some(c => c.edit === e.i && c.kind === e.kind)) add('purchase-unverified', e, 'No purchase result for this placement.', e.kind === 'mysterybox' ? 'The retail box only exercises its selected starting location. Set this placement as the starting box in a separate layout to verify it.' : 'Fix the earlier incomplete route or skipped placement, then rerun the doctor.', 'WARN');
  // mapkit-fix-3 mood lighting: every light on the empty base reports its tint and renderer use
  for (const e of layout.edits.map((e, i) => ({ ...e, i })).filter(e => e.kind === 'light' && layout.empty))
    if (!checks.some(c => c.edit === e.i && c.kind === 'light')) add('light-unverified', e, 'No light self-test line for this edit.', 'Let the self-test finish (mapkit_check_lights runs after the purchases), then rerun the doctor.', 'WARN');
  const clusters = new Map();
  // A native no-path printed in the spawn frame before the spawn marker opens that life: within 5 lines, or further up
  // when its frame (next timed line) is the spawn's own level time (sluice run 53912 :30708 -> :30717, 9 lines of
  // other actors' samples between them).
  const spawnLine = s => pathEvents.filter(e => e.ent === s.ent && e.kind === 'no-path'
    && e.logLine < s.logLine && (e.logLine >= s.logLine - 5 || e.frameTime === s.time)).at(-1)?.logLine ?? s.logLine;
  const lifeSpawn = a => spawns.find(s => s.ent === a.ent && s.time === a.birth && s.logLine > a.firstLine && s.logLine <= a.firstLine + 8)
    ?? spawns.filter(s => s.ent === a.ent && s.logLine <= a.firstLine).at(-1);
  // A diagnostic attempt is not a persistent failure when this SAME life resumes movement.
  // Native diagnostics can precede the first sampler tick; spawn markers bound entity reuse.
  for (const event of pathEvents) {
    event.node = nearestNode(nodes, event.position)?.i;
    if (event.phase === 'photos') { event.resolution = 'post-DONE camera route'; continue; }
    const a = [...actors.values()].find(a => {
      if (a.ent !== event.ent) return false;
      // The first sampler line of a life can print just BEFORE its bo1_actor_spawn marker (same
      // level time): that marker, not the previous life's, opens this life.
      const spawn = lifeSpawn(a);
      const start = spawn ? Math.min(spawnLine(spawn), a.firstLine) : a.firstLine;
      const next = spawns.find(s => s.ent === a.ent && s.logLine > Math.max(a.firstLine, spawn?.logLine ?? 0));
      const nextLife = [...actors.values()].find(b => b.ent === a.ent && b.firstLine > a.firstLine);
      const end = Math.min(next ? spawnLine(next) : Infinity, nextLife?.firstLine ?? Infinity);
      a.lifeEnd = end;
      return event.logLine >= start && event.logLine < end;
    });
    if (a) {
      event.serial = a.serial; event.source = a.source;
      // The cull line carries the position at death: a life culled >32 u from its no-path point had moved on.
      const lastSeen = [...a.samples, ...culls.filter(c => c.ent === a.ent && c.logLine < a.lifeEnd && c.time >= a.birth)];
      const resumed = lastSeen.find(s => s.logLine > event.logLine && distance(s.position, event.position) > 32);
      if (resumed) {
        event.resolution = 'same life resumed movement';
        event.recovery = { time: resumed.time, position: resumed.position, logLine: resumed.logLine };
      }
    }
    if (!event.resolution) {
      const spawn = spawns.filter(s => s.ent === event.ent && spawnLine(s) <= event.logLine).at(-1);
      const initial = spawn && (event.time === undefined ? event.logLine === spawnLine(spawn) && event.logLine < spawn.logLine
        : event.time >= spawn.time && event.time - spawn.time <= 1000);
      // Recovery needs 32 u of travel: at a fresh zombie's ~12-20 u/s start plus the 500 ms sampler that takes
      // up to ~3 s. A test-cleanup cull inside 1 s always counts; up to 3 s only if the life had started moving
      // (cull position >4 u from the no-path point). A zombie that never moved stays a finding.
      // mapkit-fix-4e: every self-test cleanup kill counts (_mapkit.gsc mapkit_test_kill callers). other_entrance (riser
      // check) and riser_during_window (window checks) were missing: sluice_works ent 223 spawned with a no-path at
      // (-800,3350), culled other_entrance 650 ms later -> an unresolved path-cluster WARN.
      const testCullReasons = ['other_window', 'round_cleanup', 'unlock_other_window', 'other_entrance', 'riser_during_window'];
      const cull = initial && culls.find(c => c.ent === event.ent && c.logLine > event.logLine && c.time >= spawn.time
        && (c.time - spawn.time <= 1000 || (c.time - spawn.time <= 3000 && distance(c.position, event.position) > 4))
        && testCullReasons.includes(c.reason));
      if (cull) { event.resolution = 'spawn trial cut short by test cleanup before recovery could be observed'; event.cull = cull; }
    }
  }
  for (const event of [...pathEvents, ...stuckEvents]) {
    event.node ??= nearestNode(nodes, event.position)?.i;
    if (event.resolution) continue;
    const key = event.position.map(v => Math.round(v / 64)).join(',');
    let c = clusters.get(key);
    if (!c) { c = { position: event.position, paths: 0, stuck: 0, node: event.node, source: event.source }; clusters.set(key, c); }
    if (event.kind) c.paths++; else c.stuck++;
  }
  for (const c of clusters.values()) {
    const w = windows.find(w => w.i === c.source) || windows.reduce((b, w) => !b || distance(w.origin, c.position) < distance(b.origin, c.position) ? w : b, null);
    add('path-cluster', w || {}, `${c.paths} no-path events, ${c.stuck} stationary episodes >=${stuckSeconds}s near node ${c.node ?? 'none'}. Events retained in events.json.`, 'Inspect this position against solid boxes and window sill; provide floor and clear walking nodes on each side. A transient no-path event can recover; check the per-window reach count.', 'WARN', c.position);
  }
  if (dump) for (const w of windows.filter(w => w.exterior)) {
    const begin = nodes.filter(n => n.type === 17).sort((a, b) => distance(a.origin, w.exterior) - distance(b.origin, w.exterior))[0];
    if (!begin || distance(begin.origin, w.exterior) > 64 || !begin.traverses?.some(i => nodes.some(n => n.i === i && n.type === 18))) add('traverse-missing', w, 'Live graph has no directed begin->end negotiation link for this window.', 'Align the window origin/yaw with its gap, supply supporting exterior/interior floors, and clear both traverse endpoints.');
  }
  if (dump && layout.empty) {
    const spawn = layout.edits.find(e => e.kind === 'spawn');
    const byIndex = new Map(nodes.filter(n => n.type !== 0).map(n => [n.i, n]));
    const starts = spawn ? nodes.filter(n => n.type !== 0 && distance(n.origin, spawn.origin) <= 192 && !segmentBlocked(spawn.origin, n.origin, stat.boxes, 16)) : [];
    const visited = new Set(starts.map(n => n.i)), pending = [...visited];
    for (let k = 0; k < pending.length; k++) for (const id of byIndex.get(pending[k])?.links || [])
      if (byIndex.has(id) && !visited.has(id)) { visited.add(id); pending.push(id); }
    for (const z of zones) {
      z.liveGraphReachable = nodes.some(n => visited.has(n.i) && contains(z, n.origin, 0.5));
      if (!z.liveGraphReachable) {
        const door = layout.edits.map((e, i) => ({ ...e, i })).find(e => ['door', 'debris'].includes(e.kind) && e.to === z.name);
        const bought = door && checks.some(c => c.edit === door.i && c.status === 'PASS' && c.kind === door.kind);
        add('zone-live-graph', door || z, `The dumped engine graph has no directed spawn route to ${z.name} (${editPath(z.i)}).${bought ? ' Its blocker purchase passed, but graph connectivity did not recover.' : ''}`,
          bought && z.staticReachable ? `The offline open-door route exists. Temporarily remove ${editPath(door.i)} from a diagnostic copy and rerun; if links return, report this blocker plus dump.json as a kit path-reconnection bug. Do not shift valid floors/windows to hide it.` : 'Connect clear walking nodes through the doorway into this zone, verify the door opens, and restore any missing floor.');
      }
    }
    for (const w of windows.filter(w => w.exterior)) {
      const begin = nodes.filter(n => n.type === 17 && distance(n.origin, w.exterior) < 64).sort((a, b) => distance(a.origin, w.exterior) - distance(b.origin, w.exterior))[0];
      if (begin && !nodes.some(n => n.type !== 0 && n.type !== 18 && n.links.includes(begin.i))) add('exterior-live-graph', w, 'The live traverse begin has no incoming exterior walking link.', 'Enable exterior room nodeSpacing and clear/support the route between the spawner and the traverse begin.');
    }
    const bad = nodes.filter(n => n.type === 0);
    if (bad.length) {
      const p = bad[0].origin, authored = stat.nodes.reduce((best, n) => !best || distance(n.origin, p) < distance(best.origin, p) ? n : best, null);
      const e = authored ? { ...layout.edits[authored.edit], i: authored.edit } : {};
      add('live-bad-nodes', e, `Engine dump contains ${bad.length} bad node(s); first index ${bad[0].i} at ${JSON.stringify(p)}.`, 'Move solids away from these nodes, restore floor, or replace the affected room grid with explicit clear node edits.', 'WARN', p);
    }
  }
  return summary();
}

function purchaseFix(kind, e) {
  if (kind === 'perk') return `Move ${e.perk || 'the perk'} onto clear floor, turn yaw toward the room, and leave a 40-unit approach. If use succeeded without a grant, inspect the power notification/script error.`;
  if (kind === 'pap') return 'Put the machine on clear floor with yaw toward the room and a 40-unit approach; the player must hold a gun from the weapon table (see the FAIL detail).';
  if (kind === 'light') return 'Give the light an area that contains a room box or a model (a room edit via room, or mins/maxs around one); lights tint the empty base only.';
  if (kind === 'powerswitch') return 'Put the switch on a wall (snap:"wall") with a 40-unit clear approach; a perk that sold with the power off means a power_on set elsewhere.';
  if (kind === 'wallbuy') return `Check weapon/model against the catalog; place origin 4 units inside the wall with yaw into the room and a clear 40-unit approach.`;
  if (['door', 'debris', 'unlock'].includes(kind)) return 'Set from/to to adjacent zones, fill their doorway with mins/maxs, orient yaw toward to, and clear the approach on both sides.';
  if (kind === 'mysterybox') return 'Place the starting box on connected floor with 44 units clear in front; inspect the weapon-grant error in console.log.';
  if (kind === 'window') return 'Check source spawner zone, exterior floor/node grid, window wall gap/yaw, and clear interior landing; use the window stage counts and stopped positions below.';
  return 'Inspect the first failed placement above and its layout JSON path; restore connected floors, nodes and zone references.';
}

// mapkit-fix-4b: the real-client pass (1x -Client, +set mapkit_boardshots 1 +set mapkit_clipshots 1), the checks of the
// mergecheck lane check_mapkit: boarded windows drawn (>= 6 scene brush models per boarded shot), a torn window rebuilt
// for points, END reached, every machine clip stops the walking player. gamesLog = the mod's g_log, consoleLog = -SaveLog.
export function analyzeClient(layout, gamesLog, consoleLog, exitCode, { boardshots = true } = {}) {
  const findings = [], at = i => [`$.edits[${i}]`, layout.edits[i] ? center(layout.edits[i]) : null];
  const lines = (gamesLog || '').split(/\r?\n/), consoleLines = (consoleLog || '').split(/\r?\n/);
  const boarded = lines.map(l => l.match(/boardshots shot (\d+) window (\d+) boarded/)).filter(Boolean).map(m => ({ shot: +m[1], edit: +m[2] }));
  const drawn = consoleLines.map(l => l.match(/scene brush models drawn (\d+) of (\d+)/)).filter(Boolean).map(m => +m[1]);
  const rebuild = lines.map(l => l.match(/boardshots rebuild 1 .*score (\d+) -> (\d+)/)).find(Boolean);
  const clipEnd = lines.map(l => l.match(/clipshots END machines (\d+) pass (\d+) fail (\d+)/)).find(Boolean);
  const clipFails = lines.map(l => l.match(/clipshots (\w+) edit (\d+) .* gap ([\d.]+) FAIL/)).filter(Boolean);
  const scriptErrors = consoleLines.filter(l => /file 'maps\/mapkit/.test(l));
  const asserts = consoleLines.filter(l => /ASSERTBEGIN|BO1_HEADLESS ASSERT/.test(l));
  if (exitCode !== 0) findings.push(finding('client-run-exit', 'FAIL', '$', null, `Real-client run exited ${exitCode}${asserts.length ? `; ${asserts.length} assert lines, first: ${asserts[0].trim().slice(0, 160)}` : ''}.`, 'Read the client console log (client-console.log) at the first assert/error; a map maker sees this crash in play.'));
  else if (asserts.length) findings.push(finding('client-assert', 'FAIL', '$', null, `${asserts.length} assert lines, first: ${asserts[0].trim().slice(0, 160)}`, 'Read client-console.log at the first assert.'));
  if (scriptErrors.length) findings.push(finding('client-script-error', 'FAIL', '$', null, scriptErrors[0].trim().slice(0, 200), 'Fix the _mapkit.gsc error shown.'));
  if (boardshots) {
    if (!lines.some(l => l.includes('boardshots END')) || !boarded.length) findings.push(finding('boardshots-incomplete', 'FAIL', '$', null, `boarded shots ${boarded.length}, END ${lines.some(l => l.includes('boardshots END')) ? 'yes' : 'missing'}${lines.find(l => l.includes('boardshots FAIL')) ? `; ${lines.find(l => l.includes('boardshots FAIL')).trim()}` : ''}.`, 'Zombies must reach and tear a window, and the player must rebuild it; check the window spawners and nodes.'));
    boarded.forEach((b, k) => { if (drawn[k] !== undefined && drawn[k] < 6) findings.push(finding('boards-not-drawn', 'FAIL', ...at(b.edit), `Boarded window photo ${b.shot}: ${drawn[k]} scene brush models drawn (a boarded window draws >= 6).`, 'The boards are invisible to players; see notes/mapkit-fix-4a-report.md (brush model culling).')); });
    if (!rebuild || +rebuild[2] <= +rebuild[1]) findings.push(finding('boardshots-rebuild', 'FAIL', '$', null, rebuild ? `rebuild score ${rebuild[1]} -> ${rebuild[2]}` : 'no rebuild line', 'Rebuilding a torn window must restore boards and give points (retail 10 per board).'));
  }
  if (!clipEnd) findings.push(finding('clipshots-incomplete', 'WARN', '$', null, 'No "clipshots END" line: machine collision was not tested.', 'Rerun the doctor; inspect client-console.log.'));
  for (const m of clipFails) findings.push(finding('machine-no-collision', 'FAIL', ...at(+m[2]), `The test player walked into the ${m[1]} (gap ${m[3]} to its clip).`, 'Machines get a player+monster clip at load (cm_mapkit.cpp CM_Mapkit_AddMachineClips); check "mapkit: N machine clips" in the console.'));
  return { findings, boarded: boarded.map((b, k) => ({ ...b, drawn: drawn[k] })), rebuild: rebuild ? [+rebuild[1], +rebuild[2]] : null, clips: clipEnd ? { machines: +clipEnd[1], pass: +clipEnd[2], fail: +clipEnd[3] } : null };
}
