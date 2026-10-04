# Map kit: Five assets, JSON layouts

Select `+set fs_game mods/mapkit +set bo1_mod_mapkit <layout>` before `+devmap zombie_pentagon`.
Layouts live in `mods/mapkit/layouts/<layout>.json`; JSON comments are accepted. Without the selector, mapkit is off.
The supported asset base is Five (`"base":"zombie_pentagon"`). Coordinates are inches, Z up; the examples use floor Z=16.

```json
{"name":"my_map","base":"zombie_pentagon","empty":true,"edits":[]}
```

`empty:true` removes the retail world, collision, entities, and pathnodes, while retaining its loaded assets.
Author the floors, ceilings, player spawn, zones, windows and zombie spawners. Without a `powerswitch` edit, empty-base perks and
Pack-a-Punch have power from the start; with one, they and doors marked `"power": true` wait until it is thrown. Lighting is flat unless a `light` edit tints an area (mood lighting); decorative light strips do not cast light. Relocated window brush
prefabs use their material's unlit colour technique where available. With `empty` omitted, edits overlay the retail map.

## Current capabilities

Every recognized edit kind is below. Examples are individual edits, not a connected map; use the complete example below.
Self-test suffixes follow `mapkit: check `. “Indirect” means there is no separate PASS for that kind; it is not a claim
that geometry, visual quality, or arbitrary entity behavior has been validated. Names default to `edit<index>`.

| Kind | Status: empty / retail overlay | JSON example | Self-test line or coverage |
|---|---|---|---|
| `room` | Supported / supported | `{"kind":"room","name":"hall","mins":[-1000,2300,16],"maxs":[-600,2720,176],"wall":8,"nodeSpacing":64,"open":["+x"]}` | `pathnodes PASS goal lookup finds N/N layout nodes`; also `mapkit: selftest room hall dir ... hit ... frac ...` trace diagnostics |
| `box` | Supported / supported | `{"kind":"box","name":"panel","mins":[-1000,2380,16],"maxs":[-994,2460,60],"material":"mc/pent_art_wall_wood10"}` | Indirect: collision probes/topdown; no per-box PASS |
| `node` | Supported / supported | `{"kind":"node","origin":[-900,2500,16]}` (Z = walking surface, feet height) | Included in `pathnodes PASS ...` aggregate |
| `stairs` | Supported / supported | `{"kind":"stairs","name":"steps","mins":[-700,2560,16],"maxs":[-540,2712,64],"up":"+x","rise":12}` | Indirect: builds step boxes + a node per tread; a purchase on top PASSes only if reached (`layouts/raised_example.json`) |
| `spawn` | Supported / empty only | `{"kind":"spawn","origin":[-920,2510,16],"yaw":0,"zone":"reception_zone"}` | Indirect: spawned test client completes purchases/reach/round checks |
| `ent` | Supported raw entity / empty only | `{"kind":"ent","keys":{"classname":"script_model","model":"t5_weapon_m14_world","origin":[-990,2510,60]}}` | `ent editN SKIP (no runtime check for this kind yet)`; inspect model and console |
| `zone` | Retail zone manager / marker only on overlay | `{"kind":"zone","name":"reception_zone","mins":[-1000,2300,16],"maxs":[-596,2720,176]}` | Empty: `zone reception_zone PASS enabled, test client inside 0` (or 1). Overlay custom zone FAILs without an info_volume; `join` does not implement wiring |
| `spawner` | Retail round spawner / empty only | `{"kind":"spawner","origin":[-880,2100,16],"yaw":90,"zone":"reception_zone"}` | Indirect: source-attributed window reach and `round PASS round A -> B ...` |
| `window` | Generated Five prefab / existing retail entrance | `{"kind":"window","name":"intake","room":"hall","origin":[-880,2296,16],"yaw":90}` | `window intake PASS a zombie from it reached the player`; empty also `window intake boards PASS N/6 torn by zombies` |
| `window` `"open": true` | Open entrance (unboardable window/gap: retail window traverse, no boards, no rebuild) / empty only | `{"kind":"window","name":"gap1","room":"room","origin":[-720,2296,16],"yaw":90,"open":true}` + a `spawner` behind it | `window gap1 PASS a zombie from it reached the player`; the boards line is replaced by `window gap1 is an open entrance (no boards, no rebuild)` |
| `riser` | Retail ground riser (Five's climb-out anim and dirt fx, then straight at the player) / empty only | `{"kind":"riser","name":"pit1","origin":[-560,2640,16],"yaw":180,"zone":"room_zone"}` (origin = floor spot inside `zone`) | `riser pit1 PASS a zombie climbed out of it and reached the player` |
| `wallbuy` | Retail purchase / empty only | `{"kind":"wallbuy","origin":[-720,2716,64],"yaw":270,"weapon":"m14_zm","model":"t5_weapon_m14_world"}` | `wallbuy m14_zm PASS bought after ... ms, score now ...` |
| `perk` | Four Five perks / four Five perks | `{"kind":"perk","perk":"specialty_rof","origin":[-520,2696,16],"yaw":270}` | `perk specialty_rof PASS bought after ... ms at ...` |
| `powerswitch` | Five's power switch (lever, sparks, announcer) / unsupported | `{"kind":"powerswitch","name":"switch","origin":[-996,2620,16],"yaw":0,"snap":"wall"}` (one per layout) | `powerswitch switch PASS perk ... refused N uses with the power off; power_on ... ms after the flip` |
| `pap` | Retail Pack-a-Punch, 5000 points, power-gated / unsupported | `{"kind":"pap","name":"pap","origin":[-860,2700,16],"snap":"wall"}` | `pap pap PASS m14_zm -> m14_upgraded_zm after ... ms, paid 5000` |
| `light` | Mood lighting: per-area tint of the flat lighting / unsupported (retail light grid) | `{"kind":"light","name":"bunker_light","room":"room","preset":"dim bunker"}` or `{"kind":"light","mins":[...],"maxs":[...],"color":[0.7,0.9,1],"intensity":1.2}` | `light bunker_light PASS tint (0.55, 0.45, 0.34) at ..., N renderer samples` |
| `door` | Retail buy + zone unlock / unsupported | `{"kind":"door","name":"gate","mins":[-600,2420,16],"maxs":[-592,2690,144],"yaw":0,"from":"reception_zone","to":"transmitter_zone","cost":750}` | `door gate PASS buyable, charged 750`; `unlock transmitter_zone PASS locked -> enabled and active`; `unlock_window intake PASS zombie selected this entrance after opening ...` |
| `debris` | Retail buy + zone unlock / unsupported | `{"kind":"debris","name":"rubble","mins":[-192,2420,16],"maxs":[-184,2690,144],"yaw":0,"from":"transmitter_zone","to":"battery_zone","cost":1000}` | `debris rubble PASS buyable, charged 1000`, plus the same unlock/window checks |
| `mysterybox` | Retail weapon roll/grant / unsupported | `{"kind":"mysterybox","name":"crate","origin":[10,2670,16],"yaw":270}` | `mysterybox crate PASS bought weapon <weapon>` verifies payment and inventory; relocation is not measured |

## Placement rules

Entrances without boards (`layouts/entrances_example.json`, doctor PASS): the rule is that boards matter only where
zombies tear a window. A window with `"open": true` is placed like a boarded one and needs the same outside `spawner`;
zombies climb through with the retail traverse and there is nothing to tear or rebuild (retail: an `exterior_goal` with
no target has no barrier). Keep players out with the gap height or a `box`. A `riser` is a retail riser spawner of its
`zone` (`script_string "riser"`, rise spot `<zone>_spawners_rise` with `find_flesh`): while the zone is active, round
zombies climb out at the spot (Five/Kino/Moon anim and fx) and go for the player. Several risers in one zone share its
spots at random, as retail. Doctor: `riser-zone` (origin outside its zone), `riser-unverified` / `selftest-riser`.

Room `mins/maxs` enclose the usable interior; walls extend outward by `wall` (default 8). `open` accepts `+x,-x,+y,-y`.
`gaps` cut openings: `{"side":"-y","along":[-930,-830],"z":[52,120]}`. `along` uses world Y for X walls and world X
for Y walls. `floor:0` or `ceiling:0` omits that surface. `floorMaterial`, `wallMaterial`, `ceilingMaterial`, and
`texScale` (default 128) select appearance. Box `contents:"none"` is visual only; default `"solid"` collides.

Shared wall: two rooms that touch share ONE wall. The room that owns it keeps that side closed (its `wall` thickness
lies outside its `mins/maxs`) and cuts the doorway as a `gap`; the neighbour lists the same side in `open` and starts
`wall` units further out. Example (sluice_works): `intake` maxs x -600 with a `+x` gap, `pumphall` mins x -592 (= -600
+ 8) with `"open":["-x"]`. Overlapping room boxes are doctor `room-overlap`.

Power: `"power": true` on a `door` makes it Five's electric door (`electric_buyable_door`: "needs power" until the
switch is thrown, then bought for `cost`; with `"cost": 0` it opens by itself at power on). Debris has no power case.
A layout without a `powerswitch` has power from the start (doctor `power-no-switch` WARN on a power door).
Complete small example: `layouts/power_example.json` (switch, Juggernog, Pack-a-Punch; doctor PASS 10/0).

Mood lighting (empty base only): a `light` edit tints everything whose lighting point is inside its area - room/box
faces (sampled at one point inside each room, so a light tints whole rooms; name the room with `room`), perk machines,
zombies, the player's gun. `room` takes the bounds of the room, zone or box edit with that name; else give `mins`/`maxs`.
Presets: `dim bunker` (warm, 0.55), `red alarm` (red, 1.0), `cold lab` (blue-white, 1.15). `color` (0..2 per channel)
and `intensity` (0..2, 1 = today's flat grey) override the preset. Overlapping lights: the later edit wins. The doctor
WARNs `light-dark` (tint luminance < 0.30) and `light-bright` (> 1.60 or a channel at the 2.0 cap), `light-overlay` on
a retail overlay (its light grid stays). The load prints `mapkit: light edit N tint r g b over (mins)-(maxs)`. Example:
`layouts/mood_example.json`.
Rounds: every round start is logged as `mapkit: round N start at <level ms>` (games_mp.log / the doctor's console.log).

Rooms generate walking nodes at `nodeSpacing` (default 56; 0 disables), starting 24 units inside their walls.
Automatic grid candidates inside static standing clearance are omitted; explicit nodes still report bad placements.
Stairs and raised areas: a `stairs` edit is the whole flight; `up` (+x,-x,+y,-y) is the rising direction and `maxs.z` the
top tread. The kit builds round(height/rise) solid steps (`rise` default 12, max 18 = zombie step height) and puts nodes on
every tread (`nodeSpacing` across, default 56); treads must be 32+ deep (use 40+), the load prints a warning otherwise. A `box`
with `"walkable":1` gets a node grid on its top, so a platform/catwalk needs no hand-placed nodes. Hand-placed `node` edits
use the walking-surface Z (the kit adds 16 and drops them to the floor). The doctor's offline walk steps up/down 18 like the engine.
Leave usable routes around furniture and inspect `node_clearance` and topdown after a load. Thin wall panels in Relay
are 6 units deep. A node-lookup PASS alone does not prove all paths are traversable.

For an empty-base window, `origin` is the wall centre at floor height; `yaw` points INTO the play room. Leave an
80–100-unit-wide gap, Z=floor+36..floor+104, and an exterior floor/duct with nodes. The kit supplies the clip, six panels,
repair boards, goal and traverse node pair. Add a `spawner` outside each entrance, with the play room's `zone`.
The goal's zone is inferred from origin + forward*48 + Z32, before retail struct indexing. Spawners are associated with
the nearest window in their zone; put each beside its intended exterior approach. Source identity stays attached to
the spawned zombie as its AI goals change. `room` names the interior room used by reach validation.
An empty-base layout with no `window` at all (an arena) still works: its spawners become retail `zombie_chaser`
spawners that go straight for the player (the load prints `mapkit: no window edits: spawners are zombie_chaser`).

On a retail overlay, a window instead reuses an existing entrance:
`{"kind":"window","name":"north_window","spawn":[-372,3200,24],"target":"auto2774","every":3,"room":"test_room"}`.
This is the recipe in `mapkit_test.json`; it does not construct a new retail window or zone.

Door/debris `mins/maxs` fill an authored doorway; yaw points from `from` toward `to`. Bounds create the solid blocker
and a usable trigger; `material` controls its visible box. Purchase removes collision, reconnects paths and unlocks
the destination through the retail zone manager. Destination zones start locked except a player-spawn zone.
Without blockers, all authored zones start enabled. Enabled zones become active through player occupancy.
List blockers in progression order for the test route. The self-test verifies a spawn from every destination window
and `door_graph` checks active links before (disconnected) and after (connected) each purchase.

Loops are supported: two blockers may lead to the same zone. The one bought second (in edit order) is a loop-closer, because
its `to` zone is already enabled. For it the test checks only the path through its own doorway (nodes in its `from`/`to`
zones or within 96 units of its bounds) and prints `door_graph NAME PASS ... (loop-closer: path through this doorway only)`
and `unlock ZONE PASS loop-closer: already enabled through another blocker, still active`. Example 2x2 loop
(`layouts/sluice_works.json`): `a -door-> b -door-> d` and `a -debris-> c -debris-> d`; the last debris is the loop-closer.
`{"kind":"debris","name":"sluice_debris","mins":[...],"maxs":[...],"yaw":0,"from":"filter_zone","to":"control_zone","cost":1250}`.
Keep each blocker's `from` and `to` zones on the two sides of its own doorway.

Purchase triggers must not intersect (wall buy: 64-unit cube 24 in front of the model; perk: radius 44 in front of the machine;
box: 80x80). The load prints `mapkit: purchase triggers of edit A and edit B intersect by N units` and the doctor FAILs
`purchase-overlap` with the distance to move; a wall buy beside a perk needs about 100+ units between origins.

Mystery-box origin is floor height; yaw faces the player. Cost defaults to 950. Multiple placements use the retail
location system; `start:0` excludes a location from the initial selection. The shipped proof covers a weapon grant,
not teddy relocation. Door/debris default cost is 750. Perks, `pap` and `powerswitch` accept `snap:"wall"`: the nearest
wall of 8 directions within 200 units wins (it replaces `yaw`), the machine backs off it and stands on the floor; without
`snap` the origin and `yaw` are used as given.

Perk machines, Pack-a-Punch, the power switch panel and the mystery box block players and zombies like retail Five's
clip brushes: the load adds a hidden player+monster clip around each (`mapkit: N machine clips` in the console), buying
through it still works, and pathnodes/links inside it are dropped so zombies path around. A `node` edit placed inside
a machine now fails `node_clearance`; move it clear. Self-test: `+set mapkit_clipshots 1` (passive god test client)
walks into every machine and logs `mapkit: clipshots <kind> edit N ... gap G PASS|FAIL` and `clipshots END`.

## Tested asset catalog

These names load on the empty Five base. Fastfile names below are the source files containing the asset identifiers;
Five also loads shared `common_zombie.ff`. They are not instructions to copy or modify the Steam files.
All four perk purchases and the three wall-buy pairs are exercised by `three_room_progression`; the box check exercises its models
and random weapon grant. This is a deliberately verified starter catalog, not every asset in the game.

**Everything else that is loaded:** `node tools/mapkit/catalog.mjs` lists every wall-buy weapon with its world model
(the `weapon`/`model` pair a `wallbuy` needs) and the perk names; `node tools/mapkit/catalog.mjs xmodel|material|weapon
[substring]` lists one kind, e.g. `catalog.mjs material concrete`. Names come from the loaded base (102 weapons,
710 xmodels, 2272 materials), minus the known stock load failures. Loaded is not tested: run the doctor on a new pick.

| Placement | Layout identifier / model | Fastfile containing the name |
|---|---|---|
| Juggernog | `specialty_armorvest` / `zombie_vending_jugg`, `zombie_vending_jugg_on` | `common_zombie.ff`, `zombie_pentagon.ff` |
| Quick Revive | `specialty_quickrevive` / `zombie_vending_revive`, `zombie_vending_revive_on` | `common_zombie.ff`, `zombie_pentagon.ff` |
| Speed Cola | `specialty_fastreload` / `zombie_vending_sleight`, `zombie_vending_sleight_on` | `common_zombie.ff`, `zombie_pentagon.ff` |
| Double Tap | `specialty_rof` / `zombie_vending_doubletap`, `zombie_vending_doubletap_on` | `common_zombie.ff`, `zombie_pentagon.ff` |
| M14 wall buy or decorative model | `m14_zm` / `t5_weapon_m14_world` | `common_zombie.ff`, `zombie_pentagon.ff` |
| MP5K wall buy or decorative model | `mp5k_zm` / `t5_weapon_mp5_world` | `common_zombie.ff` |
| Stakeout wall buy or decorative model | `ithaca_zm` / `t5_weapon_ithaca_world` | `common_zombie.ff` |
| Mystery box body/lid (automatic) | `zombie_treasure_box`, `zombie_treasure_box_lid` | `zombie_pentagon.ff` |
| Window brush prefab (automatic) | Five `pf82` brush models; not a named xmodel | `zombie_pentagon.ff` |

Use the `perk` kind instead of manually assembling a vending model and trigger. Other perk identifiers are skipped.
The diary's guessed MP5K/Olympia world-model names failed to load. MP5K uses `mp5_world`, not `mp5k_world`. A weapon name alone
does not prove its decorative model is loaded. Test new pairs for `Could not load xmodel` and an actual purchase.

Relay's known materials in `zombie_pentagon.ff`: `mc/jun_art_concrete_base02_dark` (floor), `mc/pent_art_wall_wood10`
(wood wall), `mc/rus_metal_panel02`, `mc/rus_metal_panel03`, `mc/jun_art_metal_green01`, `mc/jun_art_metal_black`,
`mc/mat_zombie_ceiling_tile01`, `mc/global_light`. List more with:

```powershell
node tools/mapkit/materials.mjs zombie_pentagon 'wall|floor|metal'
```

This lists material strings, not validated weapon/model pairs. Empty-base box colours do not provide baked lighting.

## PowerShell build and test loop

**Step 1 after every layout edit: run the doctor.** It performs static checks, drives one private-desktop
game run, and writes a self-contained report with a marked topdown PNG, logs, and player-side window pictures:

```powershell
node tools/mapkit/doctor.mjs mods/mapkit/layouts/relay_station.json
```

Read `build/doctor/relay_station/report.md` (the same report prints to stdout). Problems are ranked with
coordinates, the exact `$.edits[index].field` to inspect, observations and a suggested layout fix. Fix the
first relevant cause, rerun this command, and compare the archived reports next to the latest report.
Exit codes are **0 PASS, 1 FAIL, 2 WARN**; missing DONE, untested purchases, missing telemetry and screenshots
remain visible. `--static` skips the game and reports live coverage as untested. JSON comments are supported.

The doctor uses an isolated build copy of your layout and mod scripts. It does not rewrite the input. Build
and set up the run folder once using the commands below. The doctor launches only through `tools/headless.ps1`,
waits for a free slot, and uses a computed maximum budget. The existing self-test walks the test client to
purchases/windows and opens doors at **4x**. After DONE it changes to **1x**, places the camera on each window's
player side and takes back-buffer screenshots, then quits; camera placement never contributes to reach tests.
The run is a functional test with funds/power and round cleanup. During purchases it kills attackers within 96 units
of the passive client, so crowds cannot prevent use-button tests. Window trials keep the selected entrance's actors alive.

Static graph links are clearance estimates; the report also checks the actual engine graph and observed
actor positions. Actor lives are tracked separately even when entity numbers are reused. Counts are sampled
every 0.5 level seconds; retail entrance, chunk-destruction and completed-emerging fields distinguish arrival,
tearing and climbing from geometric room entry. Exact reach witnesses also cover the gaps between sampler ticks.
Every no-path event and stationary cluster is retained in the report's linked event file. A no-path attempt is
informational only with recorded movement resuming for the same life, an explicit test cleanup cutting short its first
second of life, or the post-DONE camera phase. Unexplained attempts and stationary episodes remain WARNs.
Asset checks use identifier presence in Five/common_zombie fastfiles; live load/purchase checks establish typed availability.
Unreferenced messages matching the recorded stock Five client baseline are informational. New messages remain WARNs;
an authored asset that fails to load remains a FAIL even if its name occurs in the stock baseline.

The kit omits the two automatic candidates obstructed by `mapkit_room`'s crate and builds the underlying door graph
before dynamic blockers disconnect it. The doctor independently checks the final active graph after purchases.
Relay's original diary includes kit bugs fixed by mapkit-fix-1; the doctor measures the current executable
and does not reproduce historical failures merely because they appear in that diary.

The manual commands below remain useful for focused follow-up and standard baseline measurements.

Run from the project folder. Copy the game into the run folder once with setup.ps1 (see README.md). Never launch the executable directly.

```powershell
$cmake = 'C:/Program Files (x86)/Microsoft Visual Studio/2022/BuildTools/Common7/IDE/CommonExtensions/Microsoft/CMake/CMake/bin/cmake.exe'
& $cmake -S . -B build -G 'Visual Studio 17 2022' -A Win32 '-DDXSDK_DIR=C:/path/to/DXSDK'
if ($LASTEXITCODE) { throw 'configure failed' }
& $cmake --build build --config Release -- '-m' '-nr:false' '-v:minimal' '-nologo'
if ($LASTEXITCODE) { throw 'build failed' }
powershell -NoProfile -ExecutionPolicy Bypass -File setup.ps1
```

After JSON/script-only edits, copy the mod, calculate the conservative budget, then require completion and zero FAILs:

```powershell
Copy-Item mods/mapkit -Destination build/Release/mods -Recurse -Force
$layout = 'three_room_progression'
$speed = 4
$budget = node tools/mapkit/selftest_budget.mjs "mods/mapkit/layouts/$layout.json" $speed | ConvertFrom-Json
$log = "build/$layout-$(Get-Date -Format yyyyMMdd-HHmmss).log"
$commands = "+set fs_game mods/mapkit +set bo1_mod_mapkit $layout +set sv_cheats 1 +set timescale $speed +set bo1_testclient 1 +set bo1_testclient_fight 1 +set bo1_testclient_god 1 +set bo1_testclient_passive 1 +set mapkit_selftest 1 +set bo1_scripterrors 100 +devmap zombie_pentagon"
powershell -NoProfile -ExecutionPolicy Bypass -File tools/headless.ps1 -Zombies -Commands $commands -AutoQuitMs $budget.autoQuitMs -TimeoutSec $budget.timeoutSec -WaitSec 60 -SaveLog $log
# exit 21 = the wrapper's watchdog hit -TimeoutSec; with DONE in the log the checks finished and only the quit was late
# (a slow load while other runs hold the CPU): not a layout failure, the DONE line below decides. The doctor reports it as a WARN.
# Without the 'bo1_headless_client: listen server' init line in the log the map never finished loading (timeoutSec allows
# 420 s of startup): a startup timeout, not a failed check - the doctor prints STARTUP TIMEOUT (finding startup-timeout).
if ($LASTEXITCODE -and $LASTEXITCODE -ne 21) { throw "wrapper failed: $LASTEXITCODE" }
$done = Select-String -Path $log -Pattern 'mapkit_selftest DONE pass (\d+) fail (\d+)' | Select-Object -Last 1
if (-not $done -or $done.Matches[0].Groups[2].Value -ne '0') { throw 'self-test incomplete or failed' }
if (Select-String -Path $log -Pattern 'throwing script exception|script runtime error|Unknown function') { throw 'script error' }
Select-String -Path $log -Pattern 'mapkit: check |mapkit_selftest'
node tools/mapkit/topdown.mjs build/Release/mods/mapkit/mapkit_dump.json "build/$layout.png" --zlo 32 --zhi 80 --scale 1
```

`-SaveLog` archives the fresh `fs_game` console log at the requested worktree-relative path, including checks and DONE.
Live files are `build/Release/mods/mapkit/console_mp.log` and `games_mp.log`; the latter appends across runs, so use its
last `InitGame` section. With `-RunDir`, console logs follow that run folder's fs_home/mod path (mods may be junctioned).
Screenshot files for the default run folder are `build/Release/mods/mapkit/screenshots/bo1_<ms>.jpg`; copy them
to distinct evidence names before the next run. Missing DONE is incomplete even if the process exits 0.

The budget includes sequential purchase timeouts, 90 seconds per entrance, zone-unlock spawn checks and round advancement,
divides level time by the requested speed, and adds a real-time allowance. Slower-than-requested simulation can still
need more real time. The self-test grants test funds before each purchase independently of power; uses movement, aim
and use commands; checks the actual perk/weapon inventory; waits for source-attributed window reach and real board
movement; then kills remaining zombies to check round advance. It is a functional route, not an unassisted combat test.
Keep fight/god/passive enabled for that route. For normal combat, omit `mapkit_selftest` and `bo1_testclient_passive`.

Screenshots use `-Client -ShotsAtMs` at **1x**. `bo1_testclient_passive 1` makes the test client walk to
`bo1_testclient_goto x,y,z` and face `bo1_testclient_look x,y,z` (no `bo1_testclient_fight` needed). Closed doors
block the walk: add `+set mapkit_open_all 1` to open every door/debris for free at spawn (console: `mapkit: open_all
triggered N blockers`). Full command for an area behind doors (control room of `sluice_works`):
`tools\headless.ps1 -Zombies -Client -ShotsAtMs "26000 33000" -AutoQuitMs 36000 -TimeoutSec 150 -Commands "+set fs_game mods/mapkit
+set bo1_mod_mapkit sluice_works +set bo1_testclient 1 +set bo1_testclient_god 1 +set bo1_testclient_passive 1
+set mapkit_open_all 1 +set bo1_testclient_goto -340,2940,16 +set bo1_testclient_look -150,3120,60 +devmap zombie_pentagon"`.
Allow ~1 s of walk per 150 units after the ~15 s load before the first shot. `AutoQuitMs`, timeout and shot times are real
time. Use 4x for functional routes; keep the standard 90-second measurement and screenshots at 1x.

Topdown draws walking links in green, actual traverses as cyan dashed arrows, and ordinary walking-wall crossings in
red. A traverse crossing its window wall is expected. `--zlo/--zhi` select the filled brush slice only: nodes/links
include every height, and walking crossing checks use node feet+8..64 independently of the displayed slice. Legacy
dumps without traverse flags use a stated begin/end-node inference. Room outlines, zones and edit marks help orient you.

## Complete progression example

[`layouts/three_room_progression.json`](layouts/three_room_progression.json) is a ready-to-run Relay-derived example:
reception -> 750-point transmitter door -> 1000-point battery debris; a battery mystery box; all four supported perks;
M14, MP5K and Stakeout stations; four exterior ducts/windows/spawners spread over three zones. Rooms have ceilings and generated
nodes. Zone activation, power, blockers and windows need no map-specific GSC. Keep exterior ducts out of playable zone
volumes and retain the gaps matched to their windows.

`relay_station.json` preserves the map maker's two-perk design. `mapkit_room.json` remains the smaller empty example;
`mapkit_test.json` is a retail overlay and includes a legacy zone marker that cannot pass the empty-zone check.
`stress_arena.json` is a crowd experiment, not a complete progression map. Existing checks are retained for these layouts;
only a completed zero-FAIL run proves a particular layout passed.
