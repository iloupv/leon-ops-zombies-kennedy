#include maps\_utility;
#include common_scripts\utility;

// LEON MOD (test) - not part of the original game. Loaded only with +set fs_game mods/leon.
// Retail maps\zombie_theater_ffotd.gsc is this file with both functions empty: the map's own "fix for the day"
// hook. The engine builds the "leon" model at level load from rtmodels/leon.rlm (+set bo1_rtmodels leon),
// which the mod's converter writes from the player's own Resident Evil 4; this test stands him around the
// first player (main_end is not reached in this port, so the test starts from main_start).

main_start()
{
	logPrint( "leon: main_start\n" );
	precacheModel( "leon" );
	precacheModel( "t5_weapon_m14_world" );
	level thread leon_spawn_test();
}

main_end()
{
}

leon_spawn_test()
{
	players = getEntArray( "player", "classname" );
	while ( players.size == 0 )
	{
		wait 0.5;
		players = getEntArray( "player", "classname" );
	}
	wait 3;
	player = players[0];
	logPrint( "leon: player at " + player.origin + " yaw " + player.angles[1] + "\n" );
	// a ring of four, so one is in view whichever way the camera looks, with a stock model beside each as a control
	for ( i = 0; i < 4; i++ )
	{
		yaw = i * 90;
		forward = anglesToForward( ( 0, yaw, 0 ) );
		right = anglesToRight( ( 0, yaw, 0 ) );
		leon = spawn( "script_model", player.origin + forward * 150 );
		leon.angles = ( 0, yaw + 180, 0 );
		leon setModel( "leon" );
		control = spawn( "script_model", player.origin + forward * 150 + right * 40 + ( 0, 0, 40 ) );
		control setModel( "t5_weapon_m14_world" );
		logPrint( "leon: spawned at " + leon.origin + " facing " + leon.angles + "\n" );
	}
}
