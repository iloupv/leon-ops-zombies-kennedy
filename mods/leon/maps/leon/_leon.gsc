#include maps\_utility;
#include common_scripts\utility;

// LEON MOD - not part of the original game. Loaded only with +set fs_game mods/leon (PlayLeon.bat), from the mod's
// maps\_mod_main.gsc (the engine runs it after every map's level main), so it plays on any map. The engine builds the
// "leon" model at level load from rtmodels/leon.rlm (+set bo1_rtmodels leon), which the mod's converter writes from
// the player's own Resident Evil 4, and poses him with RE4's motions; here the player becomes Leon:
//   leon_thirdperson 1  third person (cg_thirdPerson is a cheat dvar: set once the level runs)
//   leon_test player    headless test: the player is stepped through the cycles (a headless client sends no input)
//   leon_test npc       headless test: a line of script_model Leons walking / running / back stepping / circling

init()
{
	logPrint( "leon: init on " + getDvar( "mapname" ) + "\n" );
	maps\leon\_character::precache(); // Leon, and Ada, HUNK and Luis where they were built
	precacheModel( "t5_weapon_m14_world" ); // the npc test's weapon (common_zombie: on every map)
	level thread leon_players(); // every player (co-op: those who join too) as his character
	level thread leon_spawn_test();
	level thread maps\leon\_kick::init(); // RE4's stagger and kick
	level thread maps\leon\_merchant::init(); // RE4's Merchant at the Mystery Box
	level thread maps\leon\_bingo::init(); // Leon's "Bingo?" as a special round starts
	level thread maps\leon\_helmet::init(); // Moon's astronaut helmet on him
	maps\leon\_blood::init(); // more blood on his hits (its effects loaded now, with the level's)
	level thread leon_game_over();
	level thread leon_box_weapons();
}

// The Mystery Box keeps to the guns RE4's motions can hold: no dual wields (the CZ75s, and the HS10, which Pack-a-Punch
// makes a pair: Leon holds one gun, the other floated in his left hand).
leon_box_weapons()
{
	while ( !isDefined( level.zombie_weapons ) )
		wait 0.5;
	wait 0.5; // (all of the map's added)
	out = strTok( "cz75dw_zm cz75dw_upgraded_zm hs10_zm", " " );
	for ( i = 0; i < out.size; i++ )
		if ( isDefined( level.zombie_weapons[ out[i] ] ) )
			level.zombie_weapons[ out[i] ].is_in_box = false;
}

// The game lost (_zombiemode's end_game: solo with no Quick Revive he never dies - his health is put back and the game
// ends): the client plays RE4's death and his scream at once (CG_RuntimeXModel_PlayerState), not 3 s later as the end
// camera (pm_type 5) takes over.
leon_game_over()
{
	level waittill( "end_game" );
	level.leon_game_over = true;
	players = getEntArray( "player", "classname" );
	for ( i = 0; i < players.size; i++ )
		players[i] setClientDvar( "leon_death_stamp", getTime() + 1 );
}

// Every player, as he comes in (the host at the start, another when he joins): his character, and what the mod gives
// each one - the kick, the pickup sound, the Merchant's notice of his score, his "Hasta luego!", his weapons put away
// while down, his pain heard by the others.
leon_players()
{
	for ( ;; )
	{
		players = getEntArray( "player", "classname" );
		for ( i = 0; i < players.size; i++ )
		{
			if ( isDefined( players[i].leon_setup ) )
				continue;
			players[i].leon_setup = true;
			players[i] thread leon_player_setup();
		}
		wait 0.25;
	}
}

leon_player_setup()
{
	self endon( "disconnect" );
	// as soon as the game has given him his character (it sets the model at the spawn), not a few seconds later
	while ( !isDefined( self.model ) || self.model == "" )
		wait 0.05;
	wait 0.1;
	if ( !isDefined( level.leon_game_over ) )
		self setClientDvar( "leon_death_stamp", "0" );
	if ( getDvar( "leon_nomodel" ) != "1" ) // tests: BO1's own player model
	{
		self maps\leon\_character::apply( self maps\leon\_character::first_pick() );
		self thread leon_keep_model();
		self thread maps\leon\_character::choose(); // "Press KNIFE to switch character"
	}
	self thread leon_down_no_weapons();
	self thread leon_pain_heard();
	level thread maps\leon\_pickup::init( self ); // RE4's pickup sound on a gun or ammo
	self maps\leon\_kick::player_init();
	self maps\leon\_merchant::player_init();
	self thread maps\leon\_bingo::player_init();
	logPrint( "leon: player " + self getEntityNumber() + " model " + self.model + "\n" );
	if ( getDvar( "leon_cooptest" ) == "1" )
		self thread leon_coop_test(); // tests: two copies, each seeing the other
}

// Tests (co-op, headless): every player invulnerable; one who joins stands 120" in front of the host, the
// two facing each other, each in the other's view.
leon_coop_test()
{
	self enableInvulnerability();
	players = getEntArray( "player", "classname" );
	if ( players.size < 2 || players[0] == self )
		return;
	host = players[0];
	forward = anglesToForward( ( 0, host.angles[1], 0 ) );
	self setOrigin( host.origin + forward * 120 );
	self setPlayerAngles( ( 0, host.angles[1] + 180, 0 ) );
	logPrint( "leon: cooptest - player " + self getEntityNumber() + " in front of the host\n" );
}

// His pain grunt for the other players (his own client plays it on his hit: CG_RuntimeXModel_PlayerHit), at most every
// half second, his character's
leon_pain_heard()
{
	self endon( "disconnect" );
	last = -1000;
	for ( ;; )
	{
		self waittill( "damage", amount, attacker );
		if ( getTime() - last < 500 || isDefined( self.revivetrigger ) )
			continue;
		last = getTime();
		alias = self maps\leon\_character::current() + "_pain";
		if ( !soundExists( alias ) )
			alias = "leon_pain";
		if ( !soundExists( alias ) )
			continue;
		players = getEntArray( "player", "classname" );
		for ( i = 0; i < players.size; i++ )
			if ( players[i] != self )
				self playSoundToPlayer( alias, players[i] );
	}
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
	// (the host: the tests' switches; leon_player_setup gives him his character meanwhile)
	while ( !isDefined( player.leon_character ) && getDvar( "leon_nomodel" ) != "1" )
		wait 0.05;
	while ( !isDefined( player.model ) || player.model == "" )
		wait 0.05;
	wait 0.1;
	yaw = player.angles[1];
	forward = anglesToForward( ( 0, yaw, 0 ) );
	right = anglesToRight( ( 0, yaw, 0 ) );
	logPrint( "leon: player at " + player.origin + " yaw " + yaw + "\n" );
	logPrint( "leon: player model " + player.model + "\n" );
	if ( player.model != "" && player.model == player maps\leon\_character::current() )
		setDvar( "leon_spawned", "1" ); // Melty's test mode (the engine's Com_MeltyTestFrame): Leon is in
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
	if ( getDvar( "leon_perks" ) != "" )
		player thread leon_perks_test( getDvar( "leon_perks" ) ); // tests: perks given ("all": the eight)
	if ( getDvar( "leon_powerup" ) != "" )
		player thread leon_powerup_test( getDvar( "leon_powerup" ) ); // tests: a power-up at his feet (e.g. minigun)
	if ( getDvar( "leon_give" ) != "" )
	{
		// tests: weapons in hand, ammo full, the first out (e.g. python_zm, or "galil_zm mp40_zm")
		given = strTok( getDvar( "leon_give" ), " " );
		for ( i = 0; i < given.size; i++ )
		{
			player giveWeapon( given[i] );
			player giveMaxAmmo( given[i] );
		}
		player switchToWeapon( given[0] );
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

// Down (last stand: Quick Revive in solo, or waiting for a teammate) he lies there with RE4's death and cannot shoot:
// BO1's last stand pistol is put away until he is revived (or bleeds out).
leon_down_no_weapons()
{
	self endon( "disconnect" );
	for ( ;; )
	{
		self waittill( "player_downed" );
		wait 0.05; // after the last stand has handed him its pistol
		self disableWeapons();
		logPrint( "leon: down, weapons off\n" );
		self waittill_any( "player_revived", "bled_out", "death", "spawned_player" );
		self enableWeapons();
		logPrint( "leon: up, weapons on\n" );
	}
}

// Leon again whenever the game gives him back a character model (a respawn, a revive).
leon_keep_model()
{
	self endon( "disconnect" );
	for ( ;; )
	{
		wait 0.25;
		character = self maps\leon\_character::current();
		if ( isDefined( self.model ) && self.model != character && self.model != "" )
		{
			self detachAll();
			self setModel( character );
		}
	}
}

// Tests: perks given at the spawn through the machines' own give_perk (_zombiemode_perks: the bottle's end), so
// their effects and icons are the bought ones'; "all" gives the eight, any map, machine or not
leon_perks_test( list )
{
	self endon( "disconnect" );
	wait 2;
	if ( list == "all" )
		list = "specialty_armorvest specialty_quickrevive specialty_fastreload specialty_rof specialty_longersprint "
			+ "specialty_flakjacket specialty_deadshot specialty_additionalprimaryweapon";
	perks = strTok( list, " " );
	machines = getEntArray( "zombie_vending", "targetname" );
	for ( i = 0; i < perks.size; i++ )
	{
		if ( self HasPerk( perks[i] ) )
			continue;
		// (a perk with no machine on the map has no icon loaded: give_perk's icon threw a script error - the perk alone)
		onMap = false;
		for ( m = 0; m < machines.size; m++ )
		{
			if ( isDefined( machines[m].script_noteworthy ) && machines[m].script_noteworthy == perks[i] )
				onMap = true;
		}
		if ( onMap )
			self maps\_zombiemode_perks::give_perk( perks[i] );
		else
			self SetPerk( perks[i] );
	}
	logPrint( "leon: perks given: " + list + "\n" );
}

// Tests: the named power-up (_zombiemode_powerups: minigun, nuke, insta_kill...) dropped at his feet once the map's
// power-ups are set up, so he picks it up
leon_powerup_test( name )
{
	self endon( "disconnect" );
	while ( !isDefined( level.zombie_powerups ) )
		wait 0.5;
	wait 2;
	if ( !isDefined( level.zombie_powerups[ name ] ) )
	{
		logPrint( "leon: no power-up " + name + " on this map\n" );
		return;
	}
	level thread maps\_zombiemode_powerups::specific_powerup_drop( name, self.origin );
	logPrint( "leon: power-up " + name + " dropped\n" );
}

// Damages the player every 3 s from a point in front of him (tests: his hit reaction, voice and going down).
leon_hurt_loop( damage )
{
	self endon( "disconnect" );
	if ( getDvar( "leon_hurtqr" ) == "1" )
	{
		self setPerk( "specialty_quickrevive" ); // solo: down (last stand) instead of game over
		self.lives = 1; // (the machine's solo life: _zombiemode's solo death checks self.lives)
	}
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
