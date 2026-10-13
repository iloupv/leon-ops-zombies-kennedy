#include maps\_utility;
#include common_scripts\utility;

// LEON MOD (test) - not part of the original game. Loaded only with +set fs_game mods/leon.
// Retail maps\zombie_theater_ffotd.gsc is this file with both functions empty: the map's own "fix for the day"
// hook, called at the start and the end of maps\zombie_theater::main(). The engine builds the "leon" model at
// level load from rtmodels/leon.rlm (+set bo1_rtmodels leon), which the mod's converter writes from the
// player's own Resident Evil 4; this test stands him in front of the first player.

main_start()
{
	precacheModel( "leon" );
}

main_end()
{
	level thread leon_spawn_test();
}

leon_spawn_test()
{
	flag_wait( "all_players_connected" );
	wait 1;
	player = get_players()[0];
	forward = anglesToForward( ( 0, player.angles[1], 0 ) );
	leon = spawn( "script_model", player.origin + forward * 120 );
	leon.angles = ( 0, player.angles[1] + 180, 0 );
	leon setModel( "leon" );
	iPrintLn( "leon: spawned at " + leon.origin );
}
