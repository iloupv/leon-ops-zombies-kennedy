# nightmare - Five and Kino with every special enemy (NOT part of the original game)

A mod folder, kept apart from the base game. On Five (zombie_pentagon), on top of Five's own thief and dog rounds:

| enemy | from | Nightmare (`nightmare_level 1`) | Hell (`nightmare_level 2`) |
|---|---|---|---|
| George Romero (the director) | Call of the Dead | 1 (a new one 30 s after a death) | 3 at once (20 s) |
| astronaut (grab + headbutt teleports you back to the start room) | Moon | 1 (45 s) | 3 at once (20 s) |
| napalm zombie | Shangri-La | 1 (60 s) | 3 at once (25 s) |
| shrieker (sonic) zombie | Shangri-La | 1 (60 s) | 3 at once (25 s) |

All of them come in the first round (measured, 4x: every type within 5-20 s of the first zombie) and keep coming:
after a death, a new one of that type comes the given seconds later (no retail round cadence). Until zombies are through
the windows they appear at path nodes 500-1400 units from you in your zone. Five's thief and hellhounds are retail (see
maps/nightmare/_nightmare.gsc for why).

They are the retail AI scripts, unmodified (loaded from the other maps' fastfiles); the shared modules that start them
are in `mods/_stack/maps/_modenemies/` (`_director`, `_astro`, `_napalm`, `_sonic`), also used by mods/horde.
George is calm until you shoot or touch him; he cannot die: after enough damage he is "defeated", drops a power-up
and a free perk, vanishes and comes back later (on Call of the Dead he walks into the water; Five has none).

The base game never reads this folder. Without `+set fs_mods nightmare`, Five is the unchanged original.

## Start it

Launcher (tools/mod-launcher.hta): Map Five or Kino, Difficulty Normal / Nightmare / Hell. It stacks with the Horde checkbox
(`fs_mods horde nightmare zinfo`). The flags are in tools/nightmare_flags.js:

```
+set fs_game mods/nightmare +set fs_mods nightmare zinfo +set nightmare_level 2
+set bo1_mod_zones zombie_temple zombie_coast zombie_moon +devmap zombie_pentagon
```

`bo1_mod_zones` must list all three zones (the mod's scripts link their AI scripts; without one Five's scripts fail to
compile). Loading them takes longer (~60 s instead of ~20 s on this PC) and +384 MB of memory.

## Kino der Toten (zombie_theater)

The same four specials, levels, timers and first-round start as on Five (the astronaut's headbutt sends you to Kino's
start spawn), plus, from the start round:

| enemy | Nightmare | Hell |
|---|---|---|
| hellhound packs (Kino's own dogs) alongside the normal zombies | 2 every 60 s, at most 4 alive | 4 every 30 s, at most 8 |
| Five's Pentagon thief | 1, back 90 s after he died or escaped | 1, back 40 s after |
| Ascension's space monkeys | 1 pack of 3, the next 60 s after it is down | 2 packs of 3, the next 25 s after one is down |

The thief on Kino (no elevators): he spawns 500-1400 units from a player, hunts each player in turn and steals the
current weapon on contact (retail grab anim). Then he sprints to a far point and escapes after 30 s with the loot and a
Max Ammo. The next thief carries the escaped loot (and skips players already robbed), so killing him gives it back.
Killing a thief returns his weapons and gives full ammo, plus a Bonfire Sale if he dies before his first steal, otherwise
a Fire Sale. Kino's retail dog rounds stay. Five keeps its retail thief round and dog rounds.

The space monkeys (retail Ascension AI): each pack runs for a perk machine in an active zone (one a player owns first,
otherwise any), climbs it and pounds it; at 0 machine health the perk is taken from everyone who has it (retail), and the
pack moves to the next machine (an emptied machine is repaired for the next pack). Hurt monkeys chase the player; their
ground pound hurts players and kills zombies near them, specials too (George, astronauts, napalm), as in retail. No Max
Ammo / free perk reward after a pack (retail gives it after a monkey round's last pack).

Kino needs two more zones, for the thief and the monkeys (tools/nightmare_flags.js adds them for zombie_theater only):
`+set bo1_mod_zones zombie_temple zombie_coast zombie_moon zombie_pentagon zombie_cosmodrome +devmap zombie_theater`.

## Dvars (a value set on the command line wins over the level's preset)

- `nightmare_start_round` (default 1) - first round of every special.
- `nightmare_george_max`, `_respawn` (s after a death; only a napalm explosion kills him), `_pace` (s between spawns).
- `nightmare_astro_max`, `nightmare_astro_respawn`.
- `horde_enemy_napalm_max` / `_respawn`, the same for `horde_enemy_sonic` (shared with horde; `_respawn` 0 = horde's
  per-round count `_per_round`).
- Kino only: `nightmare_dog_round`, `_pack` (dogs per pack), `_interval` (s between packs), `_max` (alive at once);
  `nightmare_thief_round`, `_respawn` (s), `_flee` (s from his first steal to his escape);
  `nightmare_monkey_round`, `_packs` (packs at once), `_per_pack`, `_respawn` (s after a pack is down).

## Not done

- Nightmare (level 1) keeps retail Shangri-La's first napalm/shrieker round (5-7): no napalm/shrieker before that.

## Limits (with a client; the console prints "mod zones: N materials linked ..." with every count at load)

The engine leaves a mod zone's world materials and its own xmodels (map props) unlinked until something uses them
(src/database/db_registry.cpp DB_ModDefer; bo1_mod_lazymodels 0 turns the xmodel part off). Hell, measured L62:

| level + zones | materials /4096 | xmodels /2000 | images /8192 |
|---|---|---|---|
| Five + temple coast moon | 2860 | 776 | 5981 |
| Kino + temple coast moon pentagon | 3090 | 616 | 6578 |
| Kino + the five zones (shipped) | 3103 | 617 | 7066 |
