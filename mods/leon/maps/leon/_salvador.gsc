#include maps\_utility;
#include common_scripts\utility;
#include maps\_zombiemode_utility;

// LEON MOD - not part of the original game. Super Salvador, RE4's island chainsaw carrier (em20's type 22), comes in
// every Hellhound round: 9 s into it, out of the dogs' lightning at a dog's spawn spot, shouting "Te voy a matar!" - an
// RE4 enemy on a hidden zombie (maps\leon\_rtenemy.gsc), running 20 % faster than a zombie runs. His model, a quarter
// larger than RE4's (tools/leon-converter/leon_rlm.py ENEMIES), takes his hits at its size (leonpassdamage). The engine
// plays his idle by how he moves, and for every step the Mercenaries' chainsaw walk, the saw swung round and round
// (em10.cpp CSawWalkAtk), smoking and throwing sparks - the Mercenaries' Waterworld's chainsaw heard starting as he comes,
// then revving all the while, a swish on every sweep. He laughs and shouts as he comes.
// He takes no dog's place while the round's dogs still come (the dogs' spawning waits on the enemies there are), and
// holds its end once they all have. A player the saw reaches is caught - one hit, down (or the game over) - and he spins
// on. Head and leg shots, the Thundergun and a trap stagger him (his chainsaw flinch), with no halo: he is never
// kicked, and a trap does not kill him. His kill: 1000 points and a Fire Sale; the last enemy of the round, his death
// drops its Max Ammo, as the last dog's does.
//   leon_salvador 0        off
//   leon_salvadortest 1    test: one comes 5 s into the level, dog round or not (2: and the player cannot be hurt; 3: and
//                          he is staggered every 6 s; 4: and shot at his head, chest and legs in turn; 5: none then,
//                          the first round ended at once and the next a dog round - its dogs and enemies logged; 6:
//                          and the Thundergun fired at him whenever he is near - with leon_give thundergun_zm; 7: and
//                          a trap's kill tried on him)

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
	if ( test == "2" || test == "3" || test == "4" || test == "5" || test == "6" )
	{
		players[0] enableInvulnerability();
		level.leon_boss_testgod = true;
	}
	flag_wait( "begin_spawning" ); // (the match under way: the intro's invulnerability over)
	if ( test == "5" )
	{
		level thread salvador_dog_round_test();
		return;
	}
	wait 5;
	ai = salvador_come( 0 );
	if ( test == "4" && isDefined( ai ) )
		ai thread salvador_shot_test( players[0] );
	if ( test == "6" && isDefined( ai ) )
		ai thread salvador_thundergun_test( players[0] );
	if ( test == "7" && isDefined( ai ) )
		ai thread salvador_trap_test();
	while ( test == "3" && isDefined( ai ) && isAlive( ai ) )
	{
		wait 6;
		if ( isDefined( ai ) && isAlive( ai ) )
			ai thread stagger();
	}
}

// test: the Thundergun fired at him by the player (_zombiemode_weap_thundergun wait_for_thundergun_fired: its blast on
// the player's weapon_fired, the gun in his hand), turned to him, once he is within 250 - again 6 s on; and where he is,
// every second (standing still after his flinch: held)
salvador_thundergun_test( player )
{
	self endon( "death" );
	self thread salvador_where_log();
	for ( ;; )
	{
		wait 0.25;
		if ( distance( self.origin, player.origin ) > 250 )
			continue;
		player setPlayerAngles( vectorToAngles( self.origin + ( 0, 0, 40 ) - player getEye() ) );
		wait 0.05;
		logPrint( "leon: salvador thundergun fired at " + getTime() + ", " + int( distance( self.origin, player.origin ) )
		          + " away, holding " + player getCurrentWeapon() + "\n" );
		player notify( "weapon_fired" );
		wait 6;
	}
}

// test: a trap's kill on him twice (the map's first trap's zombie_trap_death, as its trigger runs it on a zombie in it -
// he is not in it, so its mark comes off at once), each time whether he lives and is unmarked, 3 s on
salvador_trap_test()
{
	self endon( "death" );
	traps = getEntArray( "zombie_trap", "targetname" );
	if ( traps.size == 0 )
	{
		logPrint( "leon: salvador trap test - no trap on this map\n" );
		return;
	}
	wait 1;
	logPrint( "leon: salvador trap test - a " + traps[0]._trap_type + " trap's kill at " + getTime() + ", health "
	          + self.health + "\n" );
	for ( i = 0; i < 2; i++ ) // (the second, the same run of it: no flinch again)
	{
		self thread maps\_zombiemode_traps::zombie_trap_death( traps[0], 50 );
		wait 3;
		logPrint( "leon: salvador trap test - alive " + isAlive( self ) + ", health " + self.health + ", marked "
		          + isDefined( self.marked_for_death ) + "\n" );
	}
}

salvador_where_log()
{
	self endon( "death" );
	for ( ;; )
	{
		wait 1;
		logPrint( "leon: salvador where " + getTime() + " " + self.origin + " busy " + isDefined( self.leon_busy ) + "\n" );
	}
}

// test: the first round over at once (its zombies to come none, those there killed) and the next a dog round
// (_zombiemode_ai_dogs dog_round_tracker: level.next_dog_round), then the dogs and the enemies there logged every 2 s
salvador_dog_round_test()
{
	wait 2;
	level.next_dog_round = level.round_number + 1;
	for ( t = 0; t < 40 && !flag( "dog_round" ); t++ )
	{
		level.zombie_total = 0;
		zombies = GetAiSpeciesArray( "axis", "all" );
		for ( i = 0; i < zombies.size; i++ )
			zombies[i] doDamage( zombies[i].health + 100, zombies[i].origin );
		wait 0.5;
	}
	for ( ;; )
	{
		dogs = GetAiSpeciesArray( "axis", "dog" );
		logPrint( "leon: salvador dogtest - dogs " + dogs.size + ", enemies " + get_enemy_count() + ", to come "
		          + level.zombie_total + ", dog round " + flag( "dog_round" ) + ", round " + level.round_number + "\n" );
		wait 2;
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
	model playSound( "salvador_sawstart" );
	ai thread salvador_pace();
	ai thread salvador_counted();
	model thread salvador_saw( ai );
	model thread salvador_saw_rev( ai );
	ai thread salvador_swings( model );
	ai thread salvador_voice( model );
	ai thread salvador_attacks();
	ai thread salvador_died();
	model thread maps\leon\_rtenemy::corpse( ai, "salvador_death" );
	logPrint( "leon: salvador at " + spot.origin + " health " + health + " round " + level.round_number + "\n" );
	return ai;
}

// 20 % faster than a zombie's run (kept: a zombie's scripts may set its pace back; at 70 %, then 40 %, he was on the
// player at once)
salvador_pace()
{
	self endon( "death" );
	for ( ;; )
	{
		self.moveplaybackrate = 1.2;
		wait 0.5;
	}
}

// not counted (_zombiemode_utility get_enemy_count: ignore_enemy_count) while the round has enemies still to come: the
// dogs' spawning waits while there are two enemies a player (_zombiemode_ai_dogs dog_round_spawning), and with him one
// of them, one dog came at a time - none while one lived. Counted once they all have come, so the last dog's death
// (dog_death: no enemy left) does not end the round under him: his own does (salvador_died)
salvador_counted()
{
	self endon( "death" );
	for ( ;; )
	{
		self.ignore_enemy_count = level.zombie_total > 0;
		wait 0.05;
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

// self: his model - the chainsaw's rev once its start is over (2.6 s), then again as each ends (1.63 s: RE4's 1.96 s a
// fifth faster, tools/leon-converter/leon_sounds.py SPEEDS), to his death
salvador_saw_rev( ai )
{
	ai endon( "death" );
	self endon( "death" );
	wait 2.6;
	for ( ;; )
	{
		self playSound( "salvador_sawrev" );
		wait 1.6;
	}
}

// the chainsaw's swish on each sweep of his walk (em10.cpp em10_R1_CSawWalkAtk: SndCall(8, 0x3D) on the frames its motion
// flags 0x20 - 3, 28, 46 and 67 of 85), the walk followed as the engine plays it (r_runtime_xmodel.cpp
// RtUpdateAnimState): at his ground speed over the walk's own (99.7 in/s: RE4's a quarter over, his size), at half to
// twice its pace, from its top again once he has stood or been busy (his kill, a stagger: their own motions)
salvador_swings( model )
{
	self endon( "death" );
	swings = array( 3, 28, 46, 67 );
	phase = 0.0;
	last = self.origin;
	for ( ;; )
	{
		wait 0.05;
		moved = self.origin - last;
		last = self.origin;
		speed = length( ( moved[0], moved[1], 0 ) ) / 0.05;
		if ( isDefined( self.leon_busy ) || speed < 12 )
		{
			phase = 0.0;
			continue;
		}
		rate = speed / 99.7;
		if ( rate < 0.5 )
			rate = 0.5;
		if ( rate > 2 )
			rate = 2;
		next = phase + 0.05 * 30 * rate;
		for ( i = 0; i < swings.size; i++ )
		{
			if ( ( phase < swings[i] && next >= swings[i] ) || ( phase < swings[i] + 85 && next >= swings[i] + 85 ) )
				model playSound( "salvador_sawswing" );
		}
		phase = next;
		if ( phase >= 85 )
			phase = phase - 85;
	}
}

// his voice every 3 to 6 s as he comes (em10.cpp em10BreathSe's laugh, 120 to 240 frames, and his chainsaw walk's
// shouts - em10_R1_CSawWalkAtk; his walk here never ends, so they come in turn with the laugh): the laugh, "Te voy a
// matar!" and the walk's two shouts (salvador_chase)
salvador_voice( model )
{
	self endon( "death" );
	lines = array( "salvador_laugh", "salvador_spawn", "salvador_chase", "salvador_chase" );
	wait 3;
	for ( ;; )
	{
		wait randomFloatRange( 3, 6 );
		if ( !isDefined( self.leon_busy ) )
			model playSound( lines[ randomInt( lines.size ) ] );
	}
}

// the Mercenaries' chainsaw walk sweeps the saw round as he goes (em10.cpp CSawWalkAtk: its sweeps catch whoever they
// touch): a player in its reach is down at once, whatever his health, and he goes on sawing - the spin is his kill, he
// stops for none (a sawing kill motion over the player was the village chainsaw man's, not his). Not while he flinches,
// nor once the game is over; a player it caught (the invulnerable tests') not again for a second
salvador_attacks()
{
	self endon( "death" );
	for ( ;; )
	{
		wait 0.05;
		if ( isDefined( self.leon_busy ) || isDefined( level.leon_game_over ) || level.intermission )
			continue;
		// the saw's reach, a quarter over RE4's (his size): 70 in front, 60 degrees either side of his facing
		target = self maps\leon\_rtenemy::in_reach( 70, 0.5 );
		if ( !isDefined( target ) || ( isDefined( target.leon_sawn ) && getTime() < target.leon_sawn ) )
			continue;
		target.leon_sawn = getTime() + 1000;
		self maps\leon\_rtenemy::hit( target, 0, true );
		logPrint( "leon: salvador caught player " + target getEntityNumber() + " at " + getTime() + "\n" );
	}
}

// maps\leon\_kick.gsc's stagger on him (self), and the Thundergun's blast (maps\leon\_rtenemy.gsc flung / knocked): his
// chainsaw flinch (Dm_Small's, 76 frames: 2.5 s), stepping back as RE4's does (its root's travel, a quarter over for his
// size)
stagger()
{
	self endon( "death" );
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
