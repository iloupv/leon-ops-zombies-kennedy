// NIGHTMARE MOD - not part of the original game. Loaded with +set fs_mods "nightmare zinfo" (or "horde nightmare zinfo")
// +set nightmare_level 1|2 +set bo1_mod_zones "zombie_temple zombie_coast zombie_moon" (tools/nightmare_flags.js).
// Registered in the fs_mods base hooks (mods/_stack/maps/_modstack.gsc); the enemies are the shared modules in
// mods/_stack/maps/_modenemies, started from each zone's hook.

register()
{
	maps\_modstack::add( "zombie_pentagon_ffotd::main_start", maps\nightmare\_nightmare::main_start );
	// L61: every other map (Kino): the shared hook at the start of maps\_zombiemode::main()
	maps\_modstack::add( "_zombiemode_ffotd::main_start", maps\nightmare\_nightmare::main_start_any_map );
	maps\_modstack::add( "mod_zones/zombie_temple::main", maps\nightmare\_nightmare::temple_zone );
	maps\_modstack::add( "mod_zones/zombie_coast::main", maps\nightmare\_nightmare::coast_zone );
	maps\_modstack::add( "mod_zones/zombie_moon::main", maps\nightmare\_nightmare::moon_zone );
	maps\_modstack::add( "mod_zones/zombie_pentagon::main", maps\nightmare\_nightmare::pentagon_zone );
	maps\_modstack::add( "mod_zones/zombie_cosmodrome::main", maps\nightmare\_nightmare::cosmodrome_zone );
}
