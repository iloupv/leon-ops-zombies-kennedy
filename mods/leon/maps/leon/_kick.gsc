#include maps\_utility;
#include common_scripts\utility;

#using_animtree( "generic_human" );

// LEON MOD - RE4's stagger and kick. A zombie shot in the head or the legs staggers (BO1's own "shot" flinch, as a
// Ganado reels from a head or knee shot) when the hit took at least 15% of its full health, and not again for 4 s
// after it recovers; while one is staggered in reach in front of a player, the engine shows him RE4's
// action prompt (his kick flags: <player> leonsetkickflags( ready, kicking ), on his player state - bgame/bg_leon.h) and
// the use key (F) kicks: RE4's kick on him (his client's pose, from leon_kick_stamp; the others' from his flags), and
// every zombie its sweep catches dies at once and is sent flying (a launched ragdoll). A staggered zombie wears a dull
// yellow halo through its stagger, seen from afar (it can be kicked): leon_staggered lists them for every player's
// client (CG_LeonKickHalos). Every player kicks (player_init, from _leon's setup of each player).
//   leon_kick 0      off
//   leon_kicktest 1  test: every 3 s the nearest zombie is put in front of the player and staggered (2: another
//                    one beside it too, for the sweep; 3: 400" ahead instead)

init()
{
	if ( getDvar( "leon_kick" ) == "0" )
		return;
	level.leon_staggered = [];
	level thread leon_zombie_watch();
	players = getEntArray( "player", "classname" );
	while ( players.size == 0 )
	{
		wait 0.5;
		players = getEntArray( "player", "classname" );
	}
	if ( getDvar( "leon_kicktest" ) == "1" || getDvar( "leon_kicktest" ) == "2" || getDvar( "leon_kicktest" ) == "3" )
		players[0] thread leon_kick_test();
}

// self: a player coming in - his kick (his client's halos and stamp start clear)
player_init()
{
	if ( getDvar( "leon_kick" ) == "0" )
		return;
	self leonsetkickflags( 0, 0 );
	self setClientDvar( "leon_kick_stamp", "0" );
	list = "";
	if ( isDefined( level.leon_staggered ) )
		for ( i = 0; i < level.leon_staggered.size; i++ )
			list = list + level.leon_staggered[i] + " ";
	self setClientDvar( "leon_staggered", list );
	self thread leon_kick_watch();
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
		reach = 40;
		if ( getDvar( "leon_kicktest" ) == "3" )
			reach = 400; // (3: far ahead - the glow seen from afar)
		z forceTeleport( self.origin + forward * reach, ( 0, self.angles[1] + 180, 0 ) );
		if ( getDvar( "leon_kicktest" ) == "2" && zombies.size > 1 )
		{
			// (2: another one beside it, not staggered - the kick's sweep kills it too)
			others = array_remove( zombies, z );
			other = getClosest( self.origin, others );
			right = anglesToRight( ( 0, self.angles[1], 0 ) );
			other forceTeleport( self.origin + forward * 45 + right * 30, ( 0, self.angles[1] + 180, 0 ) );
		}
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
		// Leon's shots, his character's perk, stagger more easily: a smaller share, a shorter rest between staggers
		leon = attacker maps\leon\_character::current() == "leon";
		// not while staggering, nor for 4 s after it (Leon's: 2.5 s)
		rest = 4000;
		if ( leon )
			rest = 2500;
		if ( isDefined( self.leon_stagger_end ) && getTime() < self.leon_stagger_end + rest )
			continue;
		// only a hit that took a real share of him (15% of his full health; Leon's 7%): weak hits do not stun-lock
		full = self.health + amount;
		if ( isDefined( self.maxhealth ) && self.maxhealth > full )
			full = self.maxhealth;
		share = 0.15;
		if ( leon )
			share = 0.07;
		if ( amount < full * share )
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
	self thread leon_stagger_glow();
	// through his own animscript (animScripted moved him by the reel's root with no collision but the ground: through
	// stairs, out of the playable area): the AI's "gravity" mode steps him with physics
	self.leon_reel = reel;
	self animcustom( ::leon_stagger_anim );
	wait length;
}

leon_stagger_anim()
{
	self endon( "death" );
	self AnimMode( "gravity" );
	self OrientMode( "face current" );
	self ClearAnim( %body, 0.2 );
	self SetFlaggedAnimKnobAllRestart( "leon_stagger", self.leon_reel, %body, 1, 0.2, 1 );
	self waittillmatch( "leon_stagger", "end" );
}

// The halo through a stagger: its entity number in leon_staggered (CG_LeonKickHalos draws it) until the stagger ends
// or it dies (a kick's ragdoll flies without it)
leon_stagger_glow()
{
	entnum = self getEntityNumber();
	level.leon_staggered[ level.leon_staggered.size ] = entnum;
	leon_staggered_publish();
	while ( isDefined( self ) && isAlive( self ) && self leon_staggered() )
		wait 0.05;
	kept = [];
	for ( i = 0; i < level.leon_staggered.size; i++ )
		if ( level.leon_staggered[i] != entnum )
			kept[ kept.size ] = level.leon_staggered[i];
	level.leon_staggered = kept;
	leon_staggered_publish();
}

leon_staggered_publish()
{
	list = "";
	for ( i = 0; i < level.leon_staggered.size; i++ )
		list = list + level.leon_staggered[i] + " ";
	players = getEntArray( "player", "classname" );
	for ( i = 0; i < players.size; i++ )
		players[i] setClientDvar( "leon_staggered", list );
}

leon_staggered()
{
	return isDefined( self.leon_stagger_end ) && getTime() < self.leon_stagger_end && self.health > 0;
}

// The nearest staggered zombie in reach in front of the player, or undefined: RE4's prompt (em10.cpp
// em10ActEvtSetKick: 1.5 m, 60 degrees of his facing, 0.7 m of height), widened: 110" of reach (centre to centre the
// zombies close in nearer than Ganados: at RE4's 59" it showed only face to face) and 48" of height (stairs, ramps).
leon_kick_target()
{
	forward = anglesToForward( ( 0, self.angles[1], 0 ) );
	zombies = getAiArray( "axis" );
	best = undefined;
	bestDist = 110;
	for ( i = 0; i < zombies.size; i++ )
	{
		z = zombies[i];
		if ( !z leon_staggered() )
			continue;
		offset = z.origin - self.origin;
		d = length( ( offset[0], offset[1], 0 ) );
		if ( d > bestDist || abs( offset[2] ) > 48 )
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
	ready = false;
	held = true; // (a use key already down - rebuilding a window, buying - is no press: only a new one kicks)
	for ( ;; )
	{
		wait 0.05;
		pressed = self useButtonPressed();
		fresh = pressed && !held;
		held = pressed;
		if ( isDefined( self.leon_kicking ) )
			continue;
		target = self leon_kick_target();
		// not offered while the use key is held from before: the offer takes the use key (Player_LeonKickTakesUse), and
		// a window being rebuilt with it held stopped - or was kicked from once that held key counted
		offer = isDefined( target ) && ( !pressed || fresh );
		if ( offer != ready )
		{
			ready = offer;
			if ( ready )
				self leonsetkickflags( 1, 0 );
			else
				self leonsetkickflags( 0, 0 );
		}
		if ( offer && fresh )
		{
			self leon_kick();
			ready = false;
			held = true;
		}
	}
}

// The kick: RE4's (em10.cpp plem10Kick) lands at its frame 16 (0.53 s) on every enemy within 1.2 m (47") of a point
// 0.3 m (12") in front of him, here 82" around a point 30" ahead (reaching the prompt's 110"), 60" of height (stairs);
// each dies and flies. Its hit sound (SE 0xF) plays once if it hits.
leon_kick()
{
	self endon( "disconnect" );
	self.leon_kicking = true;
	self setClientDvar( "leon_kick_stamp", getTime() ); // his client starts the kick on it
	self leonsetkickflags( 0, 1 ); // no prompt, aim, shot or reload through it (PM_TankControls); the others see it
	self enableInvulnerability(); // no damage through the kick
	wait 0.53;
	forward = anglesToForward( ( 0, self.angles[1], 0 ) );
	center = self.origin + forward * 30;
	// Ada's kick, her character's perk: a wider sweep and a harder launch (maps\leon\_character.gsc)
	strong = self maps\leon\_character::current() == "ada";
	sweep = 82;
	if ( strong )
		sweep = 120;
	zombies = getAiArray( "axis" );
	hits = 0;
	for ( i = 0; i < zombies.size; i++ )
	{
		z = zombies[i];
		offset = z.origin - center;
		if ( length( ( offset[0], offset[1], 0 ) ) > sweep || abs( offset[2] ) > 60 )
			continue;
		z thread leon_kicked( self, forward, strong );
		hits++;
	}
	hit = hits > 0;
	if ( hit )
		self playLocalSound( "leon_kickhit" );
	if ( hit )
		logPrint( "leon: kick at " + getTime() + " hit " + hits + ", score " + self.score + "\n" );
	else if ( zombies.size )
	{
		near = getClosest( center, zombies );
		logPrint( "leon: kick at " + getTime() + " missed, nearest " + distance( near.origin, center ) + " off " + ( near.origin - self.origin ) + " alive " + isAlive( near ) + "\n" );
	}
	wait 1.25; // to the motion's end (50 frames: 1.67 s) and its blend out
	if ( hit )
		logPrint( "leon: kick's kills scored, score " + self.score + "\n" );
	self leonsetkickflags( 0, 0 );
	if ( getDvar( "leon_test" ) == "" && getDvar( "leon_kicktest" ) == "" ) // (the tests keep him invulnerable)
		self disableInvulnerability();
	while ( self useButtonPressed() )
		wait 0.05;
	self.leon_kicking = undefined;
}

leon_kicked( player, forward, strong )
{
	// (no endon "death": the kill below sends it, and the ragdoll comes after)
	// dismembered: a limb torn off (BO1's own zombie gibs)
	gibs = array( "right_arm", "left_arm", "right_leg", "left_leg", "no_legs" );
	self.a.gib_ref = gibs[ randomInt( gibs.size ) ];
	self thread animscripts\zombie_death::do_gib();
	// a melee kill (SP DoDamage: damage, origin, attacker, piece, means of death - SP's names: "melee" - hit location):
	// it scores as a knife kill (_zombiemode_score: the melee bonus); with none it scored a plain kill's 50
	self DoDamage( self.health + 666, player.origin, player, -1, "melee", "torso_upper" );
	if ( self.health <= 0 || !isAlive( self ) )
	{
		self StartRagdoll();
		if ( isDefined( strong ) && strong )
			self LaunchRagdoll( forward * 260 + ( 0, 0, 120 ) ); // (Ada's)
		else
			self LaunchRagdoll( forward * 150 + ( 0, 0, 80 ) );
	}
}
