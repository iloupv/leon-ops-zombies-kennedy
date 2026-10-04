// The horde mod's + commands for a zombie count, in ONE place: tools/mod-launcher.hta loads this file (<script src>)
// and tools/mergecheck.sh's check_play runs it with node, so what the launcher plays and what the check runs cannot drift.
// Plain ES3 (the HTA's JScript and node both run it). o = { n: max zombies, round: start round, napalm, sonic, zinfo, spawnAll, fillRate, noperks, unjam, carryover }.
function hordeFlags(o) {
  var n = parseInt(o.n, 10); if (isNaN(n)) n = 128;
  n = Math.max(24, Math.min(1024, n));
  // fs_mods: the mods stacked (src/universal/com_files.cpp); fs_game keeps the logs/config in mods/horde. The launcher puts
  // mapkit in front on a kit map ("+set fs_mods mapkit horde ...": the layout's hooks run first).
  var a = "+set fs_game mods/horde +set fs_mods horde zinfo" + (o.noperks ? " noperks" : "") + " +set horde_hud 0 +set horde_max_zombies " + n +
          " +set bo1_mod_maxactors " + Math.min(1024, Math.max(32, n + 40)); // +40 reserve, within MAX_ACTORS_CAP (L43q)
  // renderer skinned-vertex cache: retail 8 MB runs out past ~150 visible zombies (bodies and the gun vanish)
  if (n > 100) a += " +set r_mod_skinCacheMB " + Math.max(32, 16 * Math.ceil((n * 70 * 1.5 / 600 + 0.15625) / 16)); // L53: measured peak rounded up to 70 MiB at 600, 50% headroom + 160 KiB reserve
  // renderer special-model slots per frame (every visible zombie, every corpse, the gun; retail 256): past them the gun
  // and the bodies vanish while the eye fx still draw (L43, Kino 320)
  if (n > 150) a += " +set r_mod_gfxEnts " + Math.min(2048, 256 * Math.ceil((n + 128) / 256));
  // mod: L54 model lighting protects three frames of moving models, with reserve for attachments/debris.
  if (n > 300) a += " +set r_mod_modelLightingEntries " + Math.min(5120, 32 * Math.ceil(3 * (n + 128) / 32));
  // Five's two O(n^2) script loops as natives (same results; L43: server frame 32.8 -> 20.7 ms at 330 alive)
  a += " +set bo1_mod_scriptperf 1";
  // L64: the listen-server thread on the P-cores of a hybrid CPU + above-normal thread priority (no-op elsewhere). Kino 1000,
  // unpinned like the launcher, 3 interleaved runs each: server p50 96.0 -> 78.1 ms, frames per 5 s 39.7 -> 50.4 (whole process
  // pinned to the P-cores: 82.9 ms, 48.9)
  a += " +set bo1_mod_pcore 1";
  // past ~300: the larger script variable pool (bo1_mod_scriptvars 2, as tools/stress.ps1), 11-bit networked entity numbers (the retail 1022 run out at ~330 alive; the
  // renderer and cgame size for them at startup, so on the command line), shared shader constant sets (128 per frame)
  // animated models drawn per frame r_dobjLimit (retail default 512 of its 1024 slots: Kino 600 ran out, L43), and
  // effect elements alive at once (retail 2048): past ~300 the eye / blood fx run out (FX_ELEM_LIMIT) and effects are dropped (L43)
  // L55: from 900 bo1_mod_netents 2 also networks 1280..1535 (at 1000 alive the 1533 networked numbers were full: spawn refusals)
  if (n > 300) a += " +set bo1_mod_scriptvars 2 +set bo1_mod_netents " + (n >= 900 ? 2 : 1) + " +set r_mod_scsShare 1 +set r_mod_fxElems 4096 +set r_dobjLimit 1024";
  // exact server options (same game, verified 0 mismatches by their mode 2; L55 in-run A/B at Kino 1000: moveaway+sectorlist
  // 74.9 -> 70.6 ms, teammove+sentientcopy 59.7 -> 54.4 ms server frame mean)
  if (n > 300) a += " +set bo1_mod_svfast 1"; // = sectorlist, moveaway, teammove, sentientcopy, entcontents, sentientscan 1 (the 900-character command line)
  // L56: the client draws frames while the listen server's frame still runs (retail waits: Kino 1000 fps 7.6 -> 91, server frames
  // 70 -> ~65 per 5 s; zombies move at the server's snapshot rate either way). Past ~300 the server frame outlasts 16.7 ms.
  if (n > 300) a += " +set cg_mod_asyncServer 1";
  var r = parseInt(o.round, 10);
  if (!isNaN(r) && r > 1) a += " +set horde_start_round " + r;
  if (o.napalm || o.sonic) a += " +set bo1_mod_zones zombie_temple";
  if (o.napalm) a += " +set horde_enemy_napalm 1";
  if (o.sonic) a += " +set horde_enemy_sonic 1";
  a += " +set zinfo " + (o.zinfo ? 1 : 0); // Tab zombie counter (horde's default is on)
  // L45: zombies spawn from every window of every opened zone, not only the zones near the players (default 0 = retail)
  if (o.spawnAll) a += " +set horde_spawn_all 1";
  // L43: spawns per second while below the cap, beside retail's one-at-a-time loop (the mod default -1 = max / 60, 0 = retail only)
  // L52: a zombie stuck in the crowd loses zombie-vs-zombie collision for 1.5 s (the mod's default is on; false = retail)
  if (o.unjam === false) a += " +set horde_unjam 0";
  var f = parseFloat(o.fillRate);
  // L43q: n/60 (15-16.7/s) cannot refill a 20-deaths/s horde. Keep explicit launcher overrides.
  if (isNaN(f) && n >= 900) f = 30;
  if (!isNaN(f)) a += " +set horde_fill_rate " + f;
  // L48: no perk machines (mods/noperks, in fs_mods above; engine: src/qcommon/cm_noperks.cpp)
  if (o.noperks) a += " +set noperks 1";
  // horde_carryover 0: a round also waits until every zombie is dead (retail); default carries alive zombies over
  if (o.carryover === false) a += " +set horde_carryover 0";
  return a;
}
if (typeof module != "undefined") module.exports = hordeFlags;
