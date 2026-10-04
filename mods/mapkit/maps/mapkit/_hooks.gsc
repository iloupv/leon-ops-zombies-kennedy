// MAPKIT MOD - not part of the original game. With +set fs_mods "mapkit ..." (mods/_stack/maps/_modstack.gsc) the layout's
// edits run from Five's hook like mods/mapkit/maps/zombie_pentagon_ffotd.gsc does with +set fs_game mods/mapkit.
// List mapkit first in fs_mods: it builds the map the later mods' hooks work on.

register()
{
	maps\_modstack::add( "zombie_pentagon_ffotd::main_start", maps\mapkit\_mapkit::main_start );
	maps\_modstack::add( "zombie_pentagon_ffotd::main_end", maps\mapkit\_mapkit::main_end );
}
