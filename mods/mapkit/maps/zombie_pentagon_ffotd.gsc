// MAPKIT MOD - not part of the original game. Loaded only with +set fs_game mods/mapkit.
// Retail maps\zombie_pentagon_ffotd.gsc is this file with both functions empty (the map's "fix for the day"
// hook, called at the start and at the end of maps\zombie_pentagon::main()). The layout's edits are applied by
// maps\mapkit\_mapkit.gsc; with bo1_mod_mapkit empty it does nothing, so the base game stays identical.

main_start()
{
	maps\mapkit\_mapkit::main_start();
}

main_end()
{
	maps\mapkit\_mapkit::main_end();
}
