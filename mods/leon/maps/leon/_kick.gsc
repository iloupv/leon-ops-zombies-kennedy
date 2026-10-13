#include maps\_utility;
#include common_scripts\utility;

#using_animtree( "generic_human" );

// LEON MOD - RE4's stagger and kick. A zombie shot in the head or the legs staggers (BO1's own "shot" flinch, as a
// Ganado reels from a head or knee shot) when the hit took at least 15% of its full health, and not again for 4 s
// after it recovers; while one is staggered in reach in front of Leon, the engine shows RE4's
// action prompt (leon_kick_ready: this script and the client share the process's dvars) and the use key (F) kicks:
// RE4's kick on him (the client's pose, from leon_kick_stamp), and every zombie its sweep catches dies at once and is
// sent flying (a launched ragdoll).
//   leon_kick 0      off
//   leon_kicktest 1  test: every 3 s the nearest zombie is put in front of the player and staggered

init()
{
	if ( getDvar( "leon_kick" ) == "0" )
		return;
	setDvar( "leon_kick_ready", "0" );
	setDvar( "leon_kick_stamp", "0" );
	level thread leon_zombie_watch();
	players = getEntArray( "player", "classname" );
	while ( players.size == 0 )
	{
		wait 0.5;
		players = getEntArray( "player", "classname" );
	}
	array_thread( players, ::leon_kick_watch );
	if ( getDvar( "leon_kicktest" ) == "1" )
		players[0] thread leon_kick_test();
}

// Test (headless: no aim at a zombie): every 3 s the nearest zombie is put in front of the player and staggered.
leon_kick_test()
{
	self endon( "disconnect" );
	self enableInvulnerability(); // the zombies it pulls in front of him would end the game
	for ( ;; )
	{
		wait 3;
		zombies = getAiArray( "axis" );
		if ( zombies.size == 0 )
			continue;
		z = getClosest( self.origin, zombies );
		forward = anglesToForward( ( 0, self.angles[1], 0 ) );
		z forceTeleport( self.origin + forward * 40, ( 0, self.angles[1] + 180, 0 ) );
		wait 0.1; // (the teleport lands first: the reel plays where he stands)
		z thread leon_stagger();
		logPrint( "leon: kicktest staggered a zombie at " + getTime() + "\n" );
	}
}

// Every zombie that spawns can stagger.
leon_zombie_watch()
{
	for ( ;; )
	{
		zombies = getAiArray( "axis" );
		for ( i = 0; i < zombies.size; i++ )
		{
			if ( !isDefined( zombies[i].leon_watched ) )
			{
				zombies[i].leon_watched = true;
				zombies[i] thread leon_stagger_watch();
			}
		}
		wait 0.2;
	}
}

leon_stagger_watch()
{
	self endon( "death" );
	for ( ;; )
	{
		self waittill( "damage", amount, attacker, dir, point, mod, modelName, tagName, partName );
		if ( !isDefined( attacker ) || !isPlayer( attacker ) || self.health <= 0 )
			continue;
		// not while staggering, nor for 4 s after it
		if ( isDefined( self.leon_stagger_end ) && getTime() < self.leon_stagger_end + 4000 )
			continue;
		// only a hit that took a real share of him (15% of his full health): weak hits do not stun-lock
		full = self.health + amount;
		if ( isDefined( self.maxhealth ) && self.maxhealth > full )
			full = self.maxhealth;
		if ( amount < full * 0.15 )
			continue;
		if ( !isDefined( mod ) || ( mod != "MOD_PISTOL_BULLET" && mod != "MOD_RIFLE_BULLET" ) )
			continue;
		if ( isDefined( self.has_legs ) && !self.has_legs )
			continue; // a crawler cannot reel
		part = "";
		if ( isDefined( partName ) )
			part = partName;
		else if ( isDefined( tagName ) )
			part = tagName;
		if ( !leon_is_head( part ) && !leon_is_leg( part ) )
			continue;
		self thread leon_stagger();
	}
}

leon_is_head( part )
{
	return isSubStr( part, "head" ) || isSubStr( part, "neck" ) || isSubStr( part, "helmet" );
}

leon_is_leg( part )
{
	return isSubStr( part, "knee" ) || isSubStr( part, "hip" ) || isSubStr( part, "ankle" ) || isSubStr( part, "leg" );
}

// The reel: BO1's "shot" flinch played where he stands; the kick's window lasts it and a little after.
leon_stagger()
{
	self endon( "death" );
	if ( cointoss() )
		reel = %ai_zombie_shot_arm_left;
	else
		reel = %ai_zombie_shot_arm_right;
	length = getAnimLength( reel );
	self.leon_stagger_end = getTime() + int( length * 1000 ) + 400;
	self animscripted( "leon_stagger", self.origin, self.angles, reel, "normal", %body, 1 );
	wait length;
}

leon_staggered()
{
	return isDefined( self.leon_stagger_end ) && getTime() < self.leon_stagger_end && self.health > 0;
}

// The nearest staggered zombie in reach in front of the player, or undefined: RE4's prompt (em10.cpp
// em10ActEvtSetKick: 1.5 m, 60 degrees of his facing, 0.7 m of height), its reach widened to 85" (centre to centre
// the zombies close in nearer than Ganados: at RE4's 59" it showed only face to face).
leon_kick_target()
{
	forward = anglesToForward( ( 0, self.angles[1], 0 ) );
	zombies = getAiArray( "axis" );
	best = undefined;
	bestDist = 85;
	for ( i = 0; i < zombies.size; i++ )
	{
		z = zombies[i];
		if ( !z leon_staggered() )
			continue;
		offset = z.origin - self.origin;
		d = length( ( offset[0], offset[1], 0 ) );
		if ( d > bestDist || abs( offset[2] ) > 27 )
			continue;
		if ( d > 1 && vectorDot( forward, vectorNormalize( ( offset[0], offset[1], 0 ) ) ) < 0.5 )
			continue;
		best = z;
		bestDist = d;
	}
	return best;
}

leon_kick_watch()
{
	self endon( "disconnect" );
	for ( ;; )
	{
		wait 0.05;
		if ( isDefined( self.leon_kicking ) )
			continue;
		target = self leon_kick_target();
		if ( isDefined( target ) )
			setDvar( "leon_kick_ready", "1" );
		else
			setDvar( "leon_kick_ready", "0" );
		if ( isDefined( target ) && self useButtonPressed() )
			self leon_kick();
	}
}

// The kick: RE4's (em10.cpp plem10Kick) lands at its frame 16 (0.53 s) on every enemy within 1.2 m (47") of a point
// 0.3 m (12") in front of him, here 62" around a point 24" ahead (reaching the prompt's 85"); each dies and flies. Its
// hit sound (SE 0xF) plays once if it hits.
leon_kick()
{
	self endon( "disconnect" );
	self.leon_kicking = true;
	setDvar( "leon_kick_stamp", getTime() ); // the client starts Leon's kick on it
	setDvar( "leon_kick_ready", "0" ); // no prompt through the kick
	wait 0.53;
	forward = anglesToForward( ( 0, self.angles[1], 0 ) );
	center = self.origin + forward * 24;
	zombies = getAiArray( "axis" );
	hit = false;
	for ( i = 0; i < zombies.size; i++ )
	{
		z = zombies[i];
		offset = z.origin - center;
		if ( length( ( offset[0], offset[1], 0 ) ) > 62 || abs( offset[2] ) > 48 )
			continue;
		z thread leon_kicked( self, forward );
		hit = true;
	}
	if ( hit )
		self playLocalSound( "leon_kickhit" );
	if ( hit )
		logPrint( "leon: kick at " + getTime() + " hit\n" );
	else if ( zombies.size )
	{
		near = getClosest( center, zombies );
		logPrint( "leon: kick at " + getTime() + " missed, nearest " + distance( near.origin, center ) + " off " + ( near.origin - self.origin ) + " alive " + isAlive( near ) + "\n" );
	}
	wait 0.6;
	setDvar( "leon_kick_ready", "0" );
	while ( self useButtonPressed() )
		wait 0.05;
	self.leon_kicking = undefined;
}

leon_kicked( player, forward )
{
	// (no endon "death": the kill below sends it, and the ragdoll comes after)
	// dismembered: a limb torn off (BO1's own zombie gibs)
	gibs = array( "right_arm", "left_arm", "right_leg", "left_leg", "no_legs" );
	self.a.gib_ref = gibs[ randomInt( gibs.size ) ];
	self thread animscripts\zombie_death::do_gib();
	self DoDamage( self.health + 666, player.origin, player );
	if ( self.health <= 0 || !isAlive( self ) )
	{
		self StartRagdoll();
		self LaunchRagdoll( forward * 150 + ( 0, 0, 80 ) );
	}
}
