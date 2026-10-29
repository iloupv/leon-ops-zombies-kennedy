#include maps\_utility;
#include common_scripts\utility;
#include maps\_zombiemode_utility;

#using_animtree( "generic_human" );

// LEON MOD - not part of the original game. Super Salvador, RE4's island chainsaw carrier (em20's type 22), comes in
// every Hellhound round: 9 s into it, out of the dogs' lightning at a dog's spawn spot, shouting "Te voy a matar!". A
// zombie of the map's own spawners carries his pathing, his hit locations and the round's count - hidden (zombie mode's
// hide only stops it being drawn: its hits and paths stay), its melee and its moans off - and his runtime model, built
// by the mod's setup from the player's own RE4 (rtmodels/salvador.rlm: tools/leon-converter/leon_rlm.py ENEMIES), is
// linked on it: the engine plays his idle, walk and run by how it moves, and each motion started here (his swing, its
// kill, his flinch, his fall) on every player's client (leon_rtactions: cgame_mp/cg_pose_mp.cpp). His saw putters all
// along, he laughs and taunts as he comes, and every attack is RE4's overhead swing (em10.cpp C_SawCriAtk): a player in
// front of the saw as it comes down (its hit frames, 20 to 26) is caught - one hit, down (or the game over). Head and
// leg shots stagger him (maps\leon\_kick.gsc: his chainsaw flinch) with no halo: he is never kicked. His kill: 1000
// points and a Fire Sale; the last enemy of the round, his death drops its Max Ammo, as the last dog's does.
//   leon_salvador 0        off
//   leon_salvadortest 1    test: one comes 5 s into the level, dog round or not (2: and the player cannot be hurt; 3: and
//                          he is staggered every 6 s)

init()
{
	level.leon_rtactions = [];
	if ( getDvar( "leon_salvador" ) == "0" || !built() )
		return;
	precacheModel( "salvador" );
	level thread salvador_rounds();
	if ( getDvar( "leon_salvadortest" ) != "" )
		level thread salvador_test();
}

// the setup built him (bo1_rtmodels_loaded: the runtime models this level has)
built()
{
	loaded = strTok( getDvar( "bo1_rtmodels_loaded" ), " " );
	for ( i = 0; i < loaded.size; i++ )
	{
		if ( loaded[i] == "salvador" )
			return true;
	}
	return false;
}

// self: a player coming in - the motions playing now
player_init()
{
	self setClientDvar( "leon_rtactions", rtactions_text() );
}

salvador_rounds()
{
	level endon( "end_game" );
	for ( ;; )
	{
		level waittill( "dog_round_starting" ); // (_zombiemode_ai_dogs dog_round_start; its dogs come 7 s in)
		level thread salvador_come( 9 );
	}
}

salvador_test()
{
	players = getEntArray( "player", "classname" );
	while ( players.size == 0 )
	{
		wait 0.5;
		players = getEntArray( "player", "classname" );
	}
	if ( getDvar( "leon_salvadortest" ) == "2" || getDvar( "leon_salvadortest" ) == "3" )
		players[0] enableInvulnerability();
	flag_wait( "begin_spawning" ); // (the match under way: the intro's invulnerability over)
	wait 5;
	ai = salvador_come( 0 );
	while ( getDvar( "leon_salvadortest" ) == "3" && isDefined( ai ) && isAlive( ai ) )
	{
		wait 6;
		if ( isDefined( ai ) && isAlive( ai ) )
			ai thread stagger();
	}
}

salvador_come( delay )
{
	level endon( "end_game" );
	wait delay;
	target = salvador_target();
	if ( !isDefined( target ) )
		return;
	spot = salvador_spot( target );
	if ( !isDefined( spot ) )
	{
		logPrint( "leon: salvador - no spot to come from\n" );
		return undefined;
	}
	return salvador_spawn( spot, target );
}

salvador_target()
{
	players = get_players();
	valid = [];
	for ( i = 0; i < players.size; i++ )
	{
		if ( is_player_valid( players[i] ) )
			valid[ valid.size ] = players[i];
	}
	if ( valid.size == 0 )
		return undefined;
	return valid[ randomInt( valid.size ) ];
}

// a dog's spawn spot 400 to 1000 from the player he hunts (_zombiemode_ai_dogs dog_spawn_factory_logic's rule), else
// the first; the dog spawners' own where a map has no spots
salvador_spot( target )
{
	spots = [];
	if ( isDefined( level.enemy_dog_locations ) )
		spots = level.enemy_dog_locations;
	if ( spots.size == 0 && isDefined( level.enemy_dog_spawns ) )
		spots = level.enemy_dog_spawns;
	if ( spots.size == 0 )
		return undefined;
	spots = array_randomize( spots );
	for ( i = 0; i < spots.size; i++ )
	{
		d2 = distanceSquared( spots[i].origin, target.origin );
		if ( d2 > 400 * 400 && d2 < 1000 * 1000 )
			return spots[i];
	}
	return spots[0];
}

salvador_spawn( spot, target )
{
	if ( !isDefined( level.enemy_spawns ) || level.enemy_spawns.size == 0 )
		return;
	// one of the map's zombie spawners, as a chaser (no window to come through: _zombiemode_spawner
	// should_skip_teardown) from the spot, put back as it was
	spawner = level.enemy_spawns[ randomInt( level.enemy_spawns.size ) ];
	keptOrigin = spawner.origin;
	keptString = spawner.script_string;
	keptTarget = spawner.target;
	spawner.origin = spot.origin;
	spawner.script_string = "zombie_chaser";
	spawner.target = "";
	ai = spawn_zombie( spawner );
	spawner.origin = keptOrigin;
	spawner.script_string = keptString;
	spawner.target = keptTarget;
	if ( !isDefined( ai ) )
	{
		logPrint( "leon: salvador - the spawn failed\n" );
		return;
	}
	yaw = vectorToAngles( target.origin - spot.origin );
	ai forceTeleport( spot.origin, ( 0, yaw[1], 0 ) );
	ai hide();
	ai.no_eye_glow = true; // (_zombiemode_spawner delayed_zombie_eye_glow, half a second on)
	ai.no_gib = 1; // (no limbs of the hidden zombie flying)
	ai.shrinked = true; // (_zombiemode_audio do_zombies_playvocals plays nothing for him: his voice is RE4's)
	ai.disableMelee = true; // (animscripts\zombie_melee CanMeleeInternal: his attack is the saw's)
	ai.leon_salvador = true;
	ai.custom_damage_func = ::salvador_catch_damage;
	ai.thundergun_fling_func = ::salvador_flung;
	ai.thundergun_disintegrate_func = ::salvador_flung;
	ai.thundergun_knockdown_func = ::salvador_knocked;
	// a boss's health: eight zombies' of the round, at least 3000, half again for each more player
	health = int( level.zombie_health * 8 );
	if ( health < 3000 )
		health = 3000;
	health = int( health * ( 1 + 0.5 * ( get_players().size - 1 ) ) );
	ai.maxhealth = health;
	ai.health = health;
	ai maps\_zombiemode_spawner::set_zombie_run_cycle( "run" );
	model = spawn( "script_model", ai.origin );
	model.angles = ai.angles;
	model setModel( "salvador" );
	model linkTo( ai );
	ai.leon_model = model;
	if ( isDefined( level._effect ) && isDefined( level._effect["lightning_dog_spawn"] ) )
		playFx( level._effect["lightning_dog_spawn"], spot.origin );
	playSoundAtPosition( "zmb_hellhound_bolt", spot.origin );
	model playSound( "salvador_spawn" );
	ai thread salvador_hidden();
	ai thread salvador_saw( model );
	ai thread salvador_taunts( model );
	ai thread salvador_attacks( model );
	ai thread salvador_died( model );
	model thread salvador_corpse( ai );
	logPrint( "leon: salvador at " + spot.origin + " health " + health + " round " + level.round_number + "\n" );
	return ai;
}

// (a zombie's scripts may show it again: its rise, its entrance)
salvador_hidden()
{
	self endon( "death" );
	for ( ;; )
	{
		self hide();
		wait 0.5;
	}
}

// the saw's putter while it idles (em10.cpp: SE 0x4D every 60 frames while it runs) - not through a swing or its kill
salvador_saw( model )
{
	self endon( "death" );
	for ( ;; )
	{
		if ( !isDefined( self.leon_busy ) || self.leon_busy == "stagger" )
			model playSound( "salvador_saw" );
		wait 1;
	}
}

// his laugh or his taunt every 4 to 8 s as he comes (em10.cpp em10BreathSe: 120 to 240 frames)
salvador_taunts( model )
{
	self endon( "death" );
	wait 3;
	for ( ;; )
	{
		wait randomFloatRange( 4, 8 );
		if ( isDefined( self.leon_busy ) )
			continue;
		if ( randomInt( 2 ) )
			model playSound( "salvador_laugh" );
		else
			model playSound( "salvador_chase" );
	}
}

salvador_attacks( model )
{
	self endon( "death" );
	for ( ;; )
	{
		wait 0.05;
		if ( isDefined( self.leon_busy ) )
			continue;
		// a player in front within 80 (RE4's em10CsawAtkCk: 1.5 m, 45 degrees of his facing)
		target = self salvador_in_reach( 80, 0.7 );
		if ( isDefined( target ) )
			self thread salvador_swing( model, target );
	}
}

// a player he can reach, or undefined: within dist of him, a third of it in height, cosMin of his facing
salvador_in_reach( dist, cosMin )
{
	forward = anglesToForward( ( 0, self.angles[1], 0 ) );
	players = get_players();
	for ( i = 0; i < players.size; i++ )
	{
		if ( !is_player_valid( players[i] ) )
			continue;
		offset = players[i].origin - self.origin;
		flat = ( offset[0], offset[1], 0 );
		d = length( flat );
		if ( d > dist || abs( offset[2] ) > 60 )
			continue;
		if ( d > 1 && vectorDot( forward, vectorNormalize( flat ) ) < cosMin )
			continue;
		return players[i];
	}
	return undefined;
}

// RE4's overhead swing (C_SawCriAtk, 78 frames: 2.6 s): turned to the player, the saw raised and brought down, its hit
// frames 20 to 26 (0.67 to 0.87 s) - a player in front of the saw then is caught
salvador_swing( model, target )
{
	self endon( "death" );
	self endon( "salvador_interrupt" );
	self.leon_busy = "attack";
	yaw = vectorToAngles( target.origin - self.origin );
	self salvador_hold( yaw[1] );
	model salvador_motion( "attack" );
	model playSound( "salvador_saw_swing" );
	model playSound( "salvador_roar" );
	self thread salvador_travel( "attack", yaw[1] );
	wait 0.65;
	caught = undefined;
	for ( i = 0; i < 5 && !isDefined( caught ); i++ )
	{
		caught = self salvador_in_reach( 75, 0.5 );
		if ( !isDefined( caught ) )
			wait 0.05;
	}
	if ( isDefined( caught ) )
	{
		self salvador_catch( model, caught );
		return;
	}
	wait 1.7;
	self salvador_free( model );
}

// caught: the saw's kill (C_SawCriHit, 99 frames: 3.3 s) and the player down at once, whatever his health
salvador_catch( model, player )
{
	self.leon_busy = "kill";
	model salvador_motion( "kill" );
	model playSound( "salvador_saw_hit" );
	// (through the player's death invulnerability after a hit, as _zombiemode's own failsafe kill: it held off the
	// catch of a player hurt a moment before)
	// always one hit: not even a kick's invulnerability holds it off (the invulnerable tests aside)
	if ( getDvar( "leon_salvadortest" ) != "2" && getDvar( "leon_salvadortest" ) != "3" )
		player disableInvulnerability();
	self.leon_catching = true;
	setSavedDvar( "player_deathInvulnerableTime", 0 );
	player DoDamage( player.health + 1000, self.origin, self );
	if ( isDefined( level.startInvulnerableTime ) )
		setSavedDvar( "player_deathInvulnerableTime", level.startInvulnerableTime );
	self.leon_catching = undefined;
	// (solo with no Quick Revive's life the game ends - _zombiemode's damage then returns 0, his health shown unchanged)
	logPrint( "leon: salvador caught player " + player getEntityNumber() + " at " + getTime() + "\n" );
	wait 3.3;
	self salvador_free( model );
}

// _zombiemode player_damage_override's damage from him (self: his zombie): his catch's, the player's health and more;
// nothing else (his zombie's own hits - none: its melee is off)
salvador_catch_damage( player )
{
	if ( isDefined( self.leon_catching ) )
		return player.health + 1000;
	return 0;
}

// maps\leon\_kick.gsc's stagger on him (self): his chainsaw flinch (Dm_Small's, 76 frames: 2.5 s), stepping back as
// RE4's does - a swing it cuts short, not his kill
stagger()
{
	self endon( "death" );
	if ( isDefined( self.leon_busy ) && self.leon_busy == "kill" )
		return;
	model = self.leon_model;
	self notify( "salvador_interrupt" );
	self.leon_busy = "stagger";
	self.leon_stagger_end = getTime() + 2500;
	yaw = self.angles[1];
	self salvador_hold( yaw );
	model salvador_motion( "stagger" );
	model playSound( "salvador_pain" );
	self thread salvador_travel( "stagger", yaw );
	logPrint( "leon: salvador staggered at " + getTime() + "\n" );
	wait 2.5;
	if ( isDefined( self.leon_busy ) && self.leon_busy == "stagger" )
		self salvador_free( model );
}

// the Thundergun's blast on him (_zombiemode_weap_thundergun: in place of its fling, its knockdown and its
// disintegration of a zombie): a share of his health, and he stands
salvador_flung( player )
{
	self DoDamage( int( self.maxhealth * 0.2 ), player.origin, player );
}

salvador_knocked( player, gib )
{
	self DoDamage( int( self.maxhealth * 0.1 ), player.origin, player );
}

// his zombie held where it stands, turned to yaw, while a motion plays on him (his own path off)
salvador_hold( yaw )
{
	self.leon_hold_yaw = yaw;
	self animcustom( ::salvador_hold_anim );
}

salvador_hold_anim()
{
	self endon( "death" );
	self AnimMode( "zonly_physics" );
	self OrientMode( "face angle", self.leon_hold_yaw );
	self ClearAnim( %body, 0.2 );
	self SetFlaggedAnimKnobAllRestart( "salvador_hold", %ai_zombie_idle_v1, %body, 1, 0.2, 1 );
	self waittill( "salvador_free" );
}

salvador_free( model )
{
	self.leon_busy = undefined;
	self notify( "salvador_free" );
	model salvador_motion_end();
}

// RE4 moves his whole model by its root keys through a motion (the swing's lunge, the flinch's steps back): the engine
// plays the motion in place, so his zombie is moved along the root's travel here - inches along his facing every 0.1 s
// (3 of RE4's frames), sampled from the motions - stopping at whatever it cannot move into
salvador_curve( name )
{
	if ( name == "attack" )
		return "0 0 4 9 14 18 22 26 31 37 43 48 51 53 55 56 57 58 58 58 59 59 59 59 60 60";
	return "0 0 1 -1 -4 -9 -14 -20 -26 -32 -37 -42 -46 -49 -50 -50 -49 -49 -49 -48 -45 -41 -37 -33 -28 -24";
}

salvador_travel( name, yaw )
{
	self endon( "death" );
	self endon( "salvador_free" );
	self endon( "salvador_interrupt" );
	curve = strTok( salvador_curve( name ), " " );
	dir = anglesToForward( ( 0, yaw, 0 ) );
	done = 0;
	last = curve.size - 1;
	for ( step = 1; step <= last * 2; step++ )
	{
		wait 0.05;
		k = step * 0.5;
		lo = int( k );
		hi = lo + 1;
		if ( hi > last )
			hi = last;
		d = int( curve[lo] ) + ( int( curve[hi] ) - int( curve[lo] ) ) * ( k - lo );
		move = d - done;
		if ( abs( move ) < 0.5 )
			continue;
		next = self.origin + dir * move;
		if ( !self mayMoveToPoint( next ) )
			return;
		self forceTeleport( next, ( 0, yaw, 0 ) );
		done = d;
	}
}

// his kill: 1000 points to the player who made it and a Fire Sale where he fell; the round's last enemy, its Max Ammo
// too (_zombiemode_ai_dogs dog_death: the last dog's, which waited on him)
salvador_died( model )
{
	self waittill( "death", attacker );
	origin = self.origin;
	by = "nobody";
	if ( isDefined( attacker ) && isPlayer( attacker ) )
		by = "player " + attacker getEntityNumber();
	else if ( isDefined( attacker ) && isDefined( attacker.classname ) )
		by = attacker.classname;
	logPrint( "leon: salvador died at " + getTime() + ", killed by " + by + "\n" );
	if ( isDefined( attacker ) && isPlayer( attacker ) )
		attacker maps\_zombiemode_score::add_to_player_score( 1000 );
	if ( isDefined( level.zombie_powerups ) && isDefined( level.zombie_powerups["fire_sale"] ) )
		level thread maps\_zombiemode_powerups::specific_powerup_drop( "fire_sale", origin );
	if ( isDefined( level.flag ) && isDefined( level.flag["dog_round"] ) && flag( "dog_round" ) && get_enemy_count() == 0
	     && level.zombie_total == 0 )
	{
		level.last_dog_origin = origin + ( 40, 0, 0 );
		level notify( "last_dog_down" );
	}
}

// self: his model - left where he fell, his fall played and its last frame held 10 s, then gone (his zombie's death or
// its removal)
salvador_corpse( ai )
{
	while ( isDefined( ai ) && isAlive( ai ) )
		wait 0.05;
	self unlink();
	self salvador_motion( "death" );
	self playSound( "salvador_death" );
	wait 10;
	self salvador_motion_end();
	self delete();
}

// self: his model - the motion it plays from now on every player's client (leon_rtactions: "<entity> <motion> <ms>")
salvador_motion( name )
{
	level.leon_rtactions[ "" + self getEntityNumber() ] = name + " " + getTime();
	rtactions_publish();
}

salvador_motion_end()
{
	level.leon_rtactions[ "" + self getEntityNumber() ] = undefined;
	rtactions_publish();
}

rtactions_text()
{
	text = "";
	keys = getArrayKeys( level.leon_rtactions );
	for ( i = 0; i < keys.size; i++ )
		text = text + keys[i] + " " + level.leon_rtactions[ keys[i] ] + " ";
	return text;
}

rtactions_publish()
{
	text = rtactions_text();
	players = getEntArray( "player", "classname" );
	for ( i = 0; i < players.size; i++ )
		players[i] setClientDvar( "leon_rtactions", text );
}
