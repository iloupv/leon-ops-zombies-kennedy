#include maps\_utility;
#include common_scripts\utility;

// LEON MOD - not part of the original game. Loaded only with +set fs_game mods/leon (PlayLeon.bat).
// Retail maps\zombie_theater_ffotd.gsc is this file with both functions empty: the map's own "fix for the day"
// hook. The engine builds the "leon" model at level load from rtmodels/leon.rlm (+set bo1_rtmodels leon),
// which the mod's converter writes from the player's own Resident Evil 4, and plays his RE4 cycles (idle / walk /
// run / back step) from how each Leon moves. Here the player becomes Leon (main_end is not reached in this port,
// so it starts from main_start):
//   leon_thirdperson 1  third person (cg_thirdPerson is a cheat dvar: set once the devmap level runs)
//   leon_test player    headless test: the player is stepped through the cycles (a headless client sends no input)
//   leon_test npc       headless test: a line of script_model Leons walking / running / back stepping / circling

main_start()
{
	logPrint( "leon: main_start\n" );
	precacheModel( "leon" );
	precacheModel( "t5_weapon_m14_world" ); // the npc test's weapon (common_zombie: on every map)
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

	player detachAll();
	player setModel( "leon" );
	logPrint( "leon: player model " + player.model + "\n" );
	if ( getDvar( "leon_thirdperson" ) == "1" )
		setDvar( "cg_thirdPerson", "1" );
	if ( getDvar( "leon_tpangle" ) != "" )
		setDvar( "cg_thirdPersonAngle", getDvar( "leon_tpangle" ) ); // tests: the camera around the player
	if ( getDvar( "leon_aimtest" ) != "" )
		setDvar( "cg_thirdPersonAimTest", getDvar( "leon_aimtest" ) ); // tests: aiming without input
	if ( getDvar( "leon_aimpitch" ) != "" )
		player setPlayerAngles( ( getDvarFloat( "leon_aimpitch" ), yaw, 0 ) ); // tests: the view pitch (+ down)

	test = getDvar( "leon_test" );
	if ( test == "" )
		return;
	player enableInvulnerability(); // the test's camera stays put while the zombies come
	if ( test == "player" )
	{
		player thread leon_player_moves( forward, right );
		return;
	}

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
	leon attach( "t5_weapon_m14_world", "tag_weapon_right" ); // checks the weapon tag
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

// Moves the player (a headless test has no input): each phase is announced on screen, so a screenshot
// names the cycle it should show. Facing stays the spawn yaw: the strafe and the back steps are relative to it.
leon_player_moves( forward, right )
{
	self endon( "disconnect" );
	start = self.origin;
	wait 2;
	for ( ;; )
	{
		self leon_player_phase( "run forward", forward, 190, 1.5 );
		self leon_player_phase( "stand", forward, 0, 1.5 );
		self leon_player_phase( "back", forward * -1, 90, 2.5 );
		self leon_player_phase( "strafe right", right, 150, 1.5 );
		self leon_player_phase( "strafe left", right * -1, 150, 1.5 );
		self leon_player_phase( "walk forward", forward, 60, 2 );
		self leon_player_phase( "stand", forward, 0, 1.5 );
		self setOrigin( start );
	}
}

leon_player_phase( name, dir, speed, time )
{
	iPrintLnBold( "leon test: " + name );
	logPrint( "leon: player " + name + " at " + getTime() + "\n" );
	// in steps: a headless client sends no input, so the server runs no player movement (setVelocity does nothing)
	step = 0.05;
	for ( t = 0; t < time; t += step )
	{
		self setOrigin( self.origin + dir * speed * step );
		wait step;
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
