#include maps\_utility;
#include common_scripts\utility;
#include maps\_zombiemode_utility;

// LEON MOD - not part of the original game. Super Salvador, RE4's island chainsaw carrier (em20's type 22), comes in
// every Hellhound round: 9 s into it, out of the dogs' lightning at a dog's spawn spot, shouting "Te voy a matar!" - an
// RE4 enemy on a hidden zombie (maps\leon\_rtenemy.gsc), running 70 % faster than a zombie runs. His model, a quarter
// larger than RE4's (tools/leon-converter/leon_rlm.py ENEMIES), takes his hits at its size (leonpassdamage). The engine
// plays his idle by how he moves, and for every step the Mercenaries' chainsaw walk, the saw swung round and round
// (em10.cpp CSawWalkAtk), smoking and throwing sparks. He laughs as he comes. A player the saw reaches is caught - one
// hit, down (or the game over) - and he saws him (C_SawCriHit). Head and leg shots stagger him (his chainsaw flinch)
// with no halo: he is never kicked. His kill: 1000 points and a Fire Sale; the last enemy of the round, his death drops
// its Max Ammo, as the last dog's does.
//   leon_salvador 0        off
//   leon_salvadortest 1    test: one comes 5 s into the level, dog round or not (2: and the player cannot be hurt; 3: and
//                          he is staggered every 6 s; 4: and shot at his head, chest and legs in turn)

init()
{
	if ( getDvar( "leon_salvador" ) == "0" || !maps\leon\_rtenemy::built( "salvador" ) )
		return;
	precacheModel( "salvador" );
	// his saw's sparks and smoke (common_zombie's: on every zombies map)
	level._effect["leon_saw_sparks"] = loadFx( "destructibles/fx_dest_elec_shower_sm" );
	level._effect["leon_saw_smoke"] = loadFx( "system_elements/fx_dust_sm_brown_em" );
	level thread salvador_rounds();
	if ( getDvar( "leon_salvadortest" ) != "" )
		level thread salvador_test();
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
	test = getDvar( "leon_salvadortest" );
	if ( test == "2" || test == "3" || test == "4" )
	{
		players[0] enableInvulnerability();
		level.leon_boss_testgod = true;
	}
	flag_wait( "begin_spawning" ); // (the match under way: the intro's invulnerability over)
	wait 5;
	ai = salvador_come( 0 );
	if ( test == "4" && isDefined( ai ) )
		ai thread salvador_shot_test( players[0] );
	while ( test == "3" && isDefined( ai ) && isAlive( ai ) )
	{
		wait 6;
		if ( isDefined( ai ) && isAlive( ai ) )
			ai thread stagger();
	}
}

// test: pistol rounds across his model's head, chest and legs in turn (from 60 in front of him), each hit logged as his
// zombie takes it (its part: where on his model - leonpassdamage)
salvador_shot_test( player )
{
	self endon( "death" );
	self thread salvador_shot_log();
	heights = array( 82, 45, 15 );
	for ( i = 0; ; i++ )
	{
		wait 0.5;
		forward = anglesToForward( ( 0, self.angles[1], 0 ) );
		at = self.origin + ( 0, 0, heights[ i % 3 ] );
		magicBullet( "m1911_zm", at + forward * 60, at - forward * 30, player );
	}
}

salvador_shot_log()
{
	self endon( "death" );
	for ( ;; )
	{
		self waittill( "damage", amount, attacker, dir, point, mod, modelName, tagName, partName );
		part = "none";
		if ( isDefined( partName ) )
			part = partName;
		logPrint( "leon: salvador shot " + amount + " " + mod + " part " + part + " height " + int( point[2] - self.origin[2] )
		          + " health " + self.health + "\n" );
	}
}

salvador_come( delay )
{
	level endon( "end_game" );
	wait delay;
	target = maps\leon\_rtenemy::target();
	if ( !isDefined( target ) )
		return undefined;
	spot = salvador_spot( target );
	if ( !isDefined( spot ) )
	{
		logPrint( "leon: salvador - no spot to come from\n" );
		return undefined;
	}
	return salvador_spawn( spot, target );
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
	// a boss's health: eight zombies' of the round, at least 3000, half again for each more player
	health = int( level.zombie_health * 8 );
	if ( health < 3000 )
		health = 3000;
	health = int( health * ( 1 + 0.5 * ( get_players().size - 1 ) ) );
	yaw = vectorToAngles( target.origin - spot.origin );
	ai = maps\leon\_rtenemy::spawn_enemy( spot.origin, yaw[1], "salvador", health, ::stagger );
	if ( !isDefined( ai ) )
	{
		logPrint( "leon: salvador - the spawn failed\n" );
		return undefined;
	}
	ai maps\_zombiemode_spawner::set_zombie_run_cycle( "run" );
	model = ai.leon_model;
	model leonPassDamage( ai, 90 ); // (his hits on his model, at its size - 90 tall: the hidden zombie a man's)
	if ( isDefined( level._effect ) && isDefined( level._effect["lightning_dog_spawn"] ) )
		playFx( level._effect["lightning_dog_spawn"], spot.origin );
	playSoundAtPosition( "zmb_hellhound_bolt", spot.origin );
	model playSound( "salvador_spawn" );
	ai thread salvador_pace();
	model thread salvador_saw( ai );
	ai thread salvador_laughs( model );
	ai thread salvador_attacks( model );
	ai thread salvador_died();
	model thread maps\leon\_rtenemy::corpse( ai, "salvador_death" );
	logPrint( "leon: salvador at " + spot.origin + " health " + health + " round " + level.round_number + "\n" );
	return ai;
}

// 70 % faster than a zombie's run (kept: a zombie's scripts may set its pace back)
salvador_pace()
{
	self endon( "death" );
	for ( ;; )
	{
		self.moveplaybackrate = 1.7;
		wait 0.5;
	}
}

// self: his model - the saw's sparks off its blade and smoke from its engine as it runs (not once he is dead)
salvador_saw( ai )
{
	ai endon( "death" );
	self endon( "death" );
	for ( i = 0; ; i++ )
	{
		playFxOnTag( level._effect["leon_saw_sparks"], self, "tag_saw_blade" );
		if ( i % 2 == 0 )
			playFxOnTag( level._effect["leon_saw_smoke"], self, "tag_saw_engine" );
		wait 0.3;
	}
}

// his laugh every 4 to 8 s as he comes (em10.cpp em10BreathSe: 120 to 240 frames)
salvador_laughs( model )
{
	self endon( "death" );
	wait 3;
	for ( ;; )
	{
		wait randomFloatRange( 4, 8 );
		if ( !isDefined( self.leon_busy ) )
			model playSound( "salvador_laugh" );
	}
}

// the Mercenaries' chainsaw walk sweeps the saw round as he goes (em10.cpp CSawWalkAtk: its sweeps catch whoever they
// touch): a player in its reach is caught. Not once the game is over (solo, his catch ended it: he sawed on at it)
salvador_attacks( model )
{
	self endon( "death" );
	for ( ;; )
	{
		wait 0.05;
		if ( isDefined( self.leon_busy ) || isDefined( level.leon_game_over ) || level.intermission )
			continue;
		// the saw's reach, a quarter over RE4's (his size): 70 in front, 60 degrees either side of his facing
		target = self maps\leon\_rtenemy::in_reach( 70, 0.5 );
		if ( isDefined( target ) )
			self thread salvador_catch( model, target );
	}
}

// caught: the saw's kill (C_SawCriHit, 99 frames: 3.3 s), turned to the player, and the player down at once, whatever
// his health
salvador_catch( model, player )
{
	self endon( "death" );
	self.leon_busy = "kill";
	yaw = vectorToAngles( player.origin - self.origin );
	self maps\leon\_rtenemy::hold( yaw[1] );
	model maps\leon\_rtenemy::motion( "kill" );
	self maps\leon\_rtenemy::hit( player, 0, true );
	logPrint( "leon: salvador caught player " + player getEntityNumber() + " at " + getTime() + "\n" );
	wait 3.3;
	self maps\leon\_rtenemy::free();
}

// maps\leon\_kick.gsc's stagger on him (self): his chainsaw flinch (Dm_Small's, 76 frames: 2.5 s), stepping back as
// RE4's does (its root's travel, a quarter over for his size) - not through his kill
stagger()
{
	self endon( "death" );
	if ( isDefined( self.leon_busy ) && self.leon_busy == "kill" )
		return;
	model = self.leon_model;
	self notify( "leon_interrupt" );
	self.leon_busy = "stagger";
	self.leon_stagger_end = getTime() + 2500;
	yaw = self.angles[1];
	self maps\leon\_rtenemy::hold( yaw );
	model maps\leon\_rtenemy::motion( "stagger" );
	model playSound( "salvador_pain" );
	self thread maps\leon\_rtenemy::travel( "0 0 1 -1 -5 -11 -18 -25 -33 -40 -46 -53 -58 -61 -63 -63 -61 -61 -61 -60 -56 -51 "
	                                         + "-46 -41 -35 -30", yaw );
	logPrint( "leon: salvador staggered at " + getTime() + "\n" );
	wait 2.5;
	if ( isDefined( self.leon_busy ) && self.leon_busy == "stagger" )
		self maps\leon\_rtenemy::free();
}

// his kill: 1000 points to the player who made it and a Fire Sale where he fell; the round's last enemy, its Max Ammo
// too (_zombiemode_ai_dogs dog_death: the last dog's, which waited on him)
salvador_died()
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
