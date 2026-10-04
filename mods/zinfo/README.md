# zinfo - a Tab zombie counter (NOT part of the original game)

A mod folder, kept apart from the base game. Vanilla rules on every zombie map; the only addition: while you hold
Tab (the scoreboard), two numbers show at the left, above the round number, in its font and size:

- **zombies alive** - `get_enemy_count()`: zombies, dogs and specials alive now.
- **left this round** - `level.zombie_total` (not spawned yet) + alive.

Start with `+set fs_game mods/zinfo` or `+set fs_mods zinfo` (stacks with other mods, e.g. `fs_mods horde zinfo`;
or tick "Tab: zombie counter" in `tools\mod-launcher.hta`). `zinfo 0` in the console hides the counter.
Script: `maps/zinfo/_zinfo.gsc` sends the numbers with SetClientDvars twice a second; the engine draws them only
when the mod sets `bo1_mod_tabinfo 1` (src/cgame/cg_sp_hud.cpp, CG_SP_DrawTabInfo).

## noperks (dvar `noperks` 0/1, default 0)

`+set noperks 1` removes every perk machine at map load: buy triggers, machine models, Quick Revive's clip entity,
Mule Kick, and the machines' clip brushes baked in the bsp (engine: src/qcommon/cm_noperks.cpp, count in
`noperks_clip`). Pack-a-Punch and the box stay; the free-perk power-up never drops. Script: mods/noperks
(`maps/noperks/_noperks.gsc`; load it with `+set fs_mods noperks`, beside any other mods). Log (games_mp.log): `noperks: removed ...`
and one `noperks PASS|FAIL <perk> at <origin>` self-test line per machine; `noperks_selftest 1` with `noperks 0` prints
the same trace for the retail level (every spot solid).
