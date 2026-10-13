#include maps\_utility;
#include common_scripts\utility;

// LEON MOD - not part of the original game. Loaded only with +set fs_game mods/leon (PlayLeon.bat), from the mod's
// maps\_zombiemode_ffotd.gsc (the shared hook every zombie map runs), so it plays on any map. The engine builds the
// "leon" model at level load from rtmodels/leon.rlm (+set bo1_rtmodels leon), which the mod's converter writes from
// the player's own Resident Evil 4, and poses him with RE4's motions; here the player becomes Leon:
//   leon_thirdperson 1  third person (cg_thirdPerson is a cheat dvar: set once the level runs)
//   leon_test player    headless test: the player is stepped through the cycles (a headless client sends no input)
//   leon_test npc       headless test: a line of script_model Leons walking / running / back stepping / circling

init()
{
	logPrint( "leon: init on " + getDvar( "mapname" ) + "\n" );
	precacheModel( "leon" );
	precacheModel( "t5_weapon_m14_world" ); // the npc test's weapon (common_zombie: on every map)
	level thread leon_spawn_test();
	level thread maps\leon\_kick::init(); // RE4's stagger and kick
}

leon_spawn_test()
{
	players = getEntArray( "player", "classname" );
	while ( players.size == 0 )
	{
		wait 0.5;
		players = getEntArray( "player", "classname" );
	}
	player = players[0];
	// as soon as the game has given him his character (it sets the model at the spawn), not a few seconds later
	while ( !isDefined( player.model ) || player.model == "" )
		wait 0.05;
	wait 0.1;
	yaw = player.angles[1];
	forward = anglesToForward( ( 0, yaw, 0 ) );
	right = anglesToRight( ( 0, yaw, 0 ) );
	logPrint( "leon: player at " + player.origin + " yaw " + yaw + "\n" );

	if ( getDvar( "leon_nomodel" ) != "1" ) // tests: BO1's own player model
	{
		player detachAll();
		player setModel( "leon" );
		player thread leon_keep_model();
	}
	logPrint( "leon: player model " + player.model + "\n" );
	if ( getDvar( "leon_thirdperson" ) == "1" )
		setDvar( "cg_thirdPerson", "1" );
	if ( getDvar( "leon_tpangle" ) != "" )
		setDvar( "cg_thirdPersonAngle", getDvar( "leon_tpangle" ) ); // tests: the camera around the player
	if ( getDvar( "leon_setdvar" ) != "" )
		setDvar( getDvar( "leon_setdvar" ), getDvar( "leon_setvalue" ) ); // tests: one (cheat) dvar once the level runs
	if ( getDvar( "leon_aimtest" ) != "" )
		setDvar( "cg_thirdPersonAimTest", getDvar( "leon_aimtest" ) ); // tests: aiming without input
	if ( getDvar( "leon_aimpitch" ) != "" )
		player setPlayerAngles( ( getDvarFloat( "leon_aimpitch" ), yaw, 0 ) ); // tests: the view pitch (+ down)
	if ( getDvar( "leon_damagelog" ) == "1" )
		player thread leon_damage_log(); // tests: the damage the player takes (zombie attacks)
	if ( getDvar( "leon_power" ) != "" )
		player thread leon_power_on( getDvarFloat( "leon_power" ) ); // tests: the map's power switch thrown
	if ( getDvar( "leon_give" ) != "" )
	{
		player giveWeapon( getDvar( "leon_give" ) ); // tests: a weapon in hand (e.g. python_zm)
		player switchToWeapon( getDvar( "leon_give" ) );
	}
	if ( getDvar( "leon_wallbuy" ) == "1" )
		player thread leon_goto_wallbuy(); // tests: in front of a wall weapon (its use hint)
	if ( getDvar( "leon_emptyclip" ) == "1" )
		player thread leon_empty_clip(); // tests: a reload to make (a headless client fires only aimed in)
	if ( getDvar( "leon_hurttest" ) != "" )
		player thread leon_hurt_loop( getDvarInt( "leon_hurttest" ) ); // tests: hits (and going down) without zombies

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

// Logs every damage the player takes, and once a second his health and the nearest zombie's distance.
leon_damage_log()
{
	self endon( "disconnect" );
	self thread leon_damage_log_events();
	for ( ;; )
	{
		wait 1;
		zombies = getAiArray( "axis" );
		nearest = 99999;
		for ( i = 0; i < zombies.size; i++ )
		{
			d = distance( zombies[i].origin, self.origin );
			if ( d < nearest )
				nearest = d;
		}
		logPrint( "leon: health " + self.health + " zombies " + zombies.size + " nearest " + int( nearest ) + "\n" );
	}
}

leon_damage_log_events()
{
	self endon( "disconnect" );
	for ( ;; )
	{
		self waittill( "damage", amount, attacker, dir, point, mod );
		logPrint( "leon: damage " + amount + " mod " + mod + " health " + self.health + "\n" );
	}
}

// Throws the map's power switch (its use trigger, "use_power_switch" / "use_elec_switch") after `delay` s (tests: the
// lights the power turns on).
leon_power_on( delay )
{
	wait delay;
	names = array( "use_power_switch", "use_elec_switch", "use_master_switch" );
	for ( i = 0; i < names.size; i++ )
	{
		trig = getEnt( names[i], "targetname" );
		if ( isDefined( trig ) )
		{
			logPrint( "leon: power switch " + names[i] + " at " + getTime() + "\n" );
			trig notify( "trigger", self );
			return;
		}
	}
	logPrint( "leon: no power switch found\n" );
}

// Puts the player 60 units in front of the level's first wall weapon, facing it (tests: its use hint).
leon_goto_wallbuy()
{
	wait 1;
	trigs = getEntArray( "weapon_upgrade", "targetname" );
	if ( trigs.size == 0 )
	{
		logPrint( "leon: no wall weapon\n" );
		return;
	}
	trig = getClosest( self.origin, trigs );
	dir = vectorNormalize( ( self.origin[0] - trig.origin[0], self.origin[1] - trig.origin[1], 0 ) );
	spot = groundpos( trig.origin + dir * 40 + ( 0, 0, 20 ) );
	self setOrigin( spot );
	self setPlayerAngles( vectorToAngles( trig.origin - ( spot + ( 0, 0, 60 ) ) ) );
	logPrint( "leon: at wall weapon " + trig.zombie_weapon_upgrade + " " + trig.origin + "\n" );
}

// One round left in the clip every 3 s (tests: a reload is always there to make).
leon_empty_clip()
{
	self endon( "disconnect" );
	for ( ;; )
	{
		self setWeaponAmmoClip( self getCurrentWeapon(), 1 );
		wait 3;
	}
}

// Leon again whenever the game gives him back a character model (a respawn, a revive).
leon_keep_model()
{
	self endon( "disconnect" );
	for ( ;; )
	{
		wait 0.25;
		if ( isDefined( self.model ) && self.model != "leon" && self.model != "" )
		{
			self detachAll();
			self setModel( "leon" );
		}
	}
}

// Damages the player every 3 s from a point in front of him (tests: his hit reaction, voice and going down).
leon_hurt_loop( damage )
{
	self endon( "disconnect" );
	if ( getDvar( "leon_hurtqr" ) == "1" )
		self setPerk( "specialty_quickrevive" ); // solo: down (last stand) instead of game over
	for ( ;; )
	{
		wait 3;
		self doDamage( damage,self.origin + anglesToForward( self.angles ) * 64 + ( 0, 0, 48 ) );
		logPrint( "leon: hurt " + damage + " health " + self.health + "\n" );
	}
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
