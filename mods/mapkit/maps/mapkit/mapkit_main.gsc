#include maps\_utility;
#include common_scripts\utility;
#include maps\_zombiemode_utility;

// MAPKIT MOD - the level main of an "empty" layout (bo1_mod_mapkit = mods/mapkit/layouts/<name>.json with
// "empty": true). The engine loads this instead of maps\<mapname>::main; the base map's fastfile only supplies
// assets, and the entity string holds only the layout's entities (retail worldspawn + spawn/ent edits).
// Order follows retail maps\zombie_pentagon::main (only the parts that do not need Five's world), then
// _zombiemode::main, then the layout's script edits (maps\mapkit\_mapkit).

main()
{
	// _gameskill shouldShowCoverWarning reads level.early_level[level.script], set only by a dev block (undefined here)
	level.enable_cover_warning = false;
	level.mapkit_empty = true; // _mapkit: window/power/self-test behaviour of the empty base
	// the map's "fix for the day" hook, as retail zombie_pentagon::main calls it: mods/mapkit's ffotd is _mapkit::main_start,
	// mods/mapkit_horde's adds the horde rules
	maps\zombie_pentagon_ffotd::main_start();

	// Five's fx table (rise/gib/sparks fx the zombie scripts play) without createfx (Five's placed effects)
	maps\zombie_pentagon_fx::scriptedFX();
	maps\zombie_pentagon_fx::precacheFX();

	level.dogs_enabled = false;
	level.random_pandora_box_start = false;
	level thread maps\_callbacksetup::SetupCallbacks();

	// Five's weapon and powerup lists (a wall buy's weapon must be included, _zombiemode_weapons)
	maps\zombie_pentagon::include_weapons();
	maps\zombie_pentagon::include_powerups();
	level.use_zombie_heroes = true;
	level.disable_protips = 1;

	maps\_zombiemode::main();

	// zones: _zombiemode::main resets level.zones, so they are started after it (layout zone edits, _mapkit)
	level thread maps\mapkit\_mapkit::start_zones();

	level thread maps\zombie_pentagon_ffotd::main_end(); // _mapkit::main_end (mapkit_horde: + horde)
}
