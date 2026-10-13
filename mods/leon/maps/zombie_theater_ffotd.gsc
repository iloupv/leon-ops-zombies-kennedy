#include maps\_utility;
#include common_scripts\utility;

// LEON MOD (test) - not part of the original game. Loaded only with +set fs_game mods/leon.
// Retail maps\zombie_theater_ffotd.gsc is this file with both functions empty: the map's own "fix for the day"
// hook. The engine builds the "leon" model at level load from rtmodels/leon.rlm (+set bo1_rtmodels leon),
// which the mod's converter writes from the player's own Resident Evil 4; this test lines four of him up in
// front of the first player (main_end is not reached in this port, so the test starts from main_start). The
// engine picks each one's RE4 cycle (idle / walk / run / back step) from how the script moves him.

main_start()
{
	logPrint( "leon: main_start\n" );
	precacheModel( "leon" );
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
	yaw = player.angles[1];
	forward = anglesToForward( ( 0, yaw, 0 ) );
	right = anglesToRight( ( 0, yaw, 0 ) );
	logPrint( "leon: player at " + player.origin + " yaw " + yaw + "\n" );
	player enableInvulnerability(); // the test's camera stays put while the zombies come

	// facing check: one walking a counterclockwise circle, always facing along it (nearest the camera he goes to
	// the camera's right, farthest to its left)
	leon = leon_spawn( player.origin + forward * 260, yaw - 90, "circle" );
	leon thread leon_circle( player.origin + forward * 330, 70, 69 );

	// one lane each, farther lanes for the faster ones; every lane crosses the view
	leon_spawn( player.origin + forward * 170, yaw + 180, "idle" );
	leon = leon_spawn( player.origin + forward * 230 - right * 80, yaw - 90, "walk" );
	leon thread leon_pace( right, 160, 69, false );
	leon = leon_spawn( player.origin + forward * 320 - right * 160, yaw - 90, "run" );
	leon thread leon_pace( right, 320, 173, false );
	leon = leon_spawn( player.origin + forward * 400 + right * 40, yaw + 90, "back" );
	leon thread leon_pace( right * -1, 100, 78, true );
}

leon_spawn( origin, yaw, role )
{
	leon = spawn( "script_model", origin );
	leon.angles = ( 0, yaw, 0 );
	leon setModel( "leon" );
	logPrint( "leon: " + role + " at " + leon.origin + " facing " + leon.angles + "\n" );
	return leon;
}

// Back and forth along dir: out facing the way he goes (or, backward, facing the start: a back step), a pause,
// then home facing home.
leon_pace( dir, dist, speed, backward )
{
	self endon( "death" );
	home = self.origin;
	away = home + dir * dist;
	time = dist / speed;
	for ( ;; )
	{
		if ( backward )
			self.angles = vectorToAngles( home - away );
		else
			self.angles = vectorToAngles( away - home );
		self moveTo( away, time );
		wait time;
		wait 1.5;
		self.angles = vectorToAngles( home - away );
		self moveTo( home, time );
		wait time;
		wait 1.5;
	}
}

leon_circle( center, radius, speed )
{
	self endon( "death" );
	step = 0.1;
	dtheta = speed * step / radius * 57.29578;
	theta = vectorToAngles( self.origin - center )[1];
	for ( ;; )
	{
		theta += dtheta;
		self.angles = ( 0, theta + 90, 0 );
		self moveTo( center + ( cos( theta ) * radius, sin( theta ) * radius, 0 ), step );
		wait step;
	}
}
