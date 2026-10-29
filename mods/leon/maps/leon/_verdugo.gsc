#include maps\_utility;
#include common_scripts\utility;
#include maps\_zombiemode_utility;

// LEON MOD - not part of the original game. Verdugo, Salazar's executioner - RE4's castle boss (em2c, the "INSECT BOSS")
// - comes in Five's special round in the thief's place. The round runs as the stock one (_zombiemode_ai_thief's
// thief_round_tracker: its alarms, its lights out, its music, its wait), and 3 s in he comes near the players with a
// scream, and roars at them. A hidden zombie carries him and his runtime model (maps\leon\_rtenemy.gsc). He attacks two
// ways, RE4's: the claw slash (em2c.cpp Atk: its hit frames 16 to 25), 95 damage, and the tail attack (TailAtk):
// through its first second the players it is aimed at are offered RE4's action button - the kick key's prompt, DODGE
// (maps\leon\_kick.gsc) - and one who takes it is left alone, Verdugo stopping to roar at him (his Threat, his taunt);
// one who does not is struck as it lands (its hit frames 44 to 78) - one hit, down (or the game over). Head and leg
// shots stagger him (his flinch). His kill ends the round as the thief's does, a Fire Sale and a Max Ammo where he fell;
// after four minutes he leaves, as the thief does at its end, a Max Ammo at the thief's start. (The thief's script is
// Five's alone, so none of its functions is called here: on another map this file must still load.)
//   leon_verdugo 0        off
//   leon_verdugotest 1    test: he comes 5 s into the level, on any map (2: and the player cannot be hurt; 3: and he
//                         attacks with his tail alone, every other one dodged; 4: with his claws alone; round: on
//                         Five, the special round next, in it as the players would have it)

init()
{
	if ( getDvar( "leon_verdugo" ) == "0" || !maps\leon\_rtenemy::built( "verdugo" ) )
		return;
	five = getDvar( "mapname" ) == "zombie_pentagon";
	if ( !five && getDvar( "leon_verdugotest" ) == "" )
		return;
	precacheModel( "verdugo" );
	if ( five )
	{
		// (thief_round_start runs it, the round's start; the map's own one still runs)
		level.leon_thief_round_start = level.thief_round_start;
		level.thief_round_start = ::verdugo_round_start;
	}
	if ( getDvar( "leon_verdugotest" ) != "" )
		level thread verdugo_test();
}

// Five's special round starting (thief_round_tracker -> thief_round_start): the round's spawning his, not the thief's
verdugo_round_start()
{
	if ( isDefined( level.leon_thief_round_start ) )
		level thread [[ level.leon_thief_round_start ]]();
	waittillframeend; // (the tracker sets the thief's spawning just after it calls this)
	level.round_spawn_func = ::verdugo_round_spawning;
}

// The round's spawning (in thief_round_spawning's place): the round holds till "last_thief_down" (thief_round_wait
// waits on level.thief_intermission), then ends as thief_round_aftermath ends it.
verdugo_round_spawning()
{
	level endon( "intermission" );
	level endon( "end_of_round" );
	level endon( "restart_round" );
	if ( level.intermission )
		return;
	level.thief_intermission = true;
	level thread verdugo_round_aftermath();
	level.zombie_total = 0;
	wait 3;
	if ( !isDefined( verdugo_come() ) )
		flag_set( "last_thief_down" ); // (no Verdugo: the round ends)
}

// _zombiemode_ai_thief thief_round_aftermath's: the round's own spawning back, its end 6 s later
verdugo_round_aftermath()
{
	flag_wait( "last_thief_down" );
	level thread maps\_zombiemode_audio::change_zombie_music( "dog_end" );
	level notify( "stop_thief_alarms" );
	level.round_spawn_func = level.thief_save_spawn_func;
	level.round_wait_func = level.thief_save_wait_func;
	wait 6;
	level.thief_intermission = false;
}

verdugo_test()
{
	if ( getDvar( "leon_verdugotest" ) == "round" )
	{
		verdugo_test_round();
		return;
	}
	players = getEntArray( "player", "classname" );
	while ( players.size == 0 )
	{
		wait 0.5;
		players = getEntArray( "player", "classname" );
	}
	if ( getDvar( "leon_verdugotest" ) == "2" || getDvar( "leon_verdugotest" ) == "3" )
	{
		players[0] enableInvulnerability();
		level.leon_boss_testgod = true;
	}
	flag_wait( "begin_spawning" ); // (the match under way: the intro's invulnerability over)
	wait 5;
	verdugo_come();
}

// Test (Five, round): the power on and the next round the special one (thief_round_tracker's), the first round's
// zombies killed as they come; in it, Verdugo killed 15 s after he comes - the round's end, then the next one's start
verdugo_test_round()
{
	flag_wait( "begin_spawning" );
	players = getEntArray( "player", "classname" );
	players[0] enableInvulnerability();
	level.leon_boss_testgod = true;
	flag_set( "power_on" );
	wait 1;
	level.next_thief_round = level.round_number + 1;
	logPrint( "leon: verdugo round test - the special round is " + level.next_thief_round + "\n" );
	level thread verdugo_test_rounds();
	for ( ;; )
	{
		wait 0.5;
		zombies = getAiArray( "axis" );
		for ( i = 0; i < zombies.size; i++ )
		{
			if ( isDefined( zombies[i].leon_boss ) )
			{
				if ( !isDefined( zombies[i].leon_test_kill ) )
				{
					zombies[i].leon_test_kill = true;
					zombies[i] thread verdugo_test_kill( players[0] );
				}
			}
			else if ( !flag( "thief_round" ) )
				zombies[i] DoDamage( zombies[i].health + 666, zombies[i].origin );
		}
	}
}

verdugo_test_kill( player )
{
	self endon( "death" );
	wait 15;
	self DoDamage( self.health + 666, self.origin, player );
}

verdugo_test_rounds()
{
	for ( ;; )
	{
		level waittill( "between_round_over" );
		logPrint( "leon: verdugo round test - round " + level.round_number + " starts, thief round " + flag( "thief_round" )
		          + "\n" );
	}
}

verdugo_come()
{
	target = maps\leon\_rtenemy::target();
	if ( !isDefined( target ) )
		return undefined;
	// by the players (where one comes back from spectating), else at the thief's start
	origin = maps\_zombiemode::check_for_valid_spawn_near_team( target );
	if ( !isDefined( origin ) )
	{
		start = getstruct( "thief_start", "targetname" );
		if ( isDefined( start ) )
			origin = start.origin;
	}
	if ( !isDefined( origin ) )
	{
		logPrint( "leon: verdugo - no spot to come from\n" );
		return undefined;
	}
	// the thief's health (15000, a share by the players: _zombiemode_ai_thief thief_scale_health), or eight zombies' of
	// the round where that is more
	players = get_players();
	health = 15000;
	if ( players.size == 3 )
		health = 12000;
	else if ( players.size == 2 )
		health = 9000;
	else if ( players.size == 1 )
		health = 4500;
	if ( health < int( level.zombie_health * 8 ) )
		health = int( level.zombie_health * 8 );
	yaw = vectorToAngles( target.origin - origin );
	ai = maps\leon\_rtenemy::spawn_enemy( origin, yaw[1], "verdugo", health, ::stagger );
	if ( !isDefined( ai ) )
	{
		logPrint( "leon: verdugo - the spawn failed\n" );
		return undefined;
	}
	ai maps\_zombiemode_spawner::set_zombie_run_cycle( "sprint" );
	model = ai.leon_model;
	if ( isDefined( level._effect ) && isDefined( level._effect["ape_spawn"] ) )
		playFx( level._effect["ape_spawn"], origin ); // (the thief's coming, where Five loaded it)
	model playSound( "verdugo_scream" );
	ai thread verdugo_taunt( model, yaw[1] ); // (he roars at them first)
	ai thread verdugo_growls( model );
	ai thread verdugo_attacks( model );
	ai thread verdugo_unstick();
	ai thread verdugo_died();
	ai thread verdugo_leaves( model, 240 );
	model thread maps\leon\_rtenemy::corpse( ai, "verdugo_death" );
	logPrint( "leon: verdugo at " + origin + " health " + health + " round " + level.round_number + "\n" );
	return ai;
}

// his growls every 4 to 8 s as he comes (his wind-up's)
verdugo_growls( model )
{
	self endon( "death" );
	for ( ;; )
	{
		wait randomFloatRange( 4, 8 );
		if ( !isDefined( self.leon_busy ) )
			model playSound( "verdugo_growl" );
	}
}

verdugo_attacks( model )
{
	self endon( "death" );
	lastTail = -100000;
	tails = 0;
	for ( ;; )
	{
		wait 0.05;
		if ( isDefined( self.leon_busy ) )
			continue;
		// a player in front within the tail's reach (140) - within the claws' (75) a slash, or the tail one time in three;
		// past them the tail, 5 s apart (the test: the tail alone)
		target = self maps\leon\_rtenemy::in_reach( 140, 0.6 );
		if ( !isDefined( target ) )
			continue;
		offset = target.origin - self.origin;
		d = length( ( offset[0], offset[1], 0 ) );
		tail = getTime() - lastTail > 5000 && ( d > 75 || randomInt( 3 ) == 0 );
		if ( getDvar( "leon_verdugotest" ) == "3" )
			tail = true;
		if ( getDvar( "leon_verdugotest" ) == "4" )
			tail = false;
		if ( tail )
		{
			lastTail = getTime();
			tails++;
			self thread verdugo_tail( model, target, tails );
		}
		else if ( d <= 75 )
			self thread verdugo_claw( model, target );
	}
}

// RE4's claw slash (Atk 0x015, 65 frames: 2.2 s): turned to the player, its hit frames 16 to 25 (0.53 to 0.83 s) - 95
// to a player in front of the claws then; its lunge 61" (the root's travel)
verdugo_claw( model, target )
{
	self endon( "death" );
	self endon( "leon_interrupt" );
	self.leon_busy = "claw";
	yaw = vectorToAngles( target.origin - self.origin );
	self maps\leon\_rtenemy::hold( yaw[1] );
	model maps\leon\_rtenemy::motion( "claw" );
	model playSound( "verdugo_swish" );
	self thread maps\leon\_rtenemy::travel( "0 1 6 13 19 27 36 48 52 50 51 53 56 57 57 54 54 54 56 58 59 61", yaw[1] );
	wait 0.5;
	struck = undefined;
	for ( i = 0; i < 7 && !isDefined( struck ); i++ )
	{
		struck = self maps\leon\_rtenemy::in_reach( 85, 0.5 );
		if ( isDefined( struck ) )
		{
			model playSound( "verdugo_hit" );
			self maps\leon\_rtenemy::hit( struck, 95, false );
			logPrint( "leon: verdugo clawed player " + struck getEntityNumber() + " at " + getTime() + ", health "
			          + struck.health + "\n" );
		}
		else
			wait 0.05;
	}
	wait 1.7 - i * 0.05;
	self maps\leon\_rtenemy::free();
}

// RE4's tail attack (TailAtk 0x01B, 135 frames: 4.5 s): through its first second (RE4's action button: frames 5 to 30)
// the players in its reach may dodge it with the kick key (leon_dodge_window: maps\leon\_kick.gsc's prompt, DODGE); as
// it lands (its hit frames 44 to 78: 1.47 to 2.6 s) one in its reach who did not is struck down at once. Dodged, he
// stops and roars at them (his taunt) in place of the rest of the sweep.
verdugo_tail( model, target, count )
{
	self endon( "death" );
	self endon( "leon_interrupt" );
	self.leon_busy = "tail";
	yaw = vectorToAngles( target.origin - self.origin );
	self maps\leon\_rtenemy::hold( yaw[1] );
	model maps\leon\_rtenemy::motion( "tail" );
	model playSound( "verdugo_swish" );
	self thread maps\leon\_rtenemy::travel( "0 1 4 6 8 9 11 13 17 22 27 35 44 52 52 49 49 56 59 56 50 47 53 59 60 60 55 47 "
	                                         + "38 31 25 19 12 6 3 1 -1 -1 -2 -2 -2 -1 -1 -1 0", yaw[1] );
	offered = [];
	players = get_players();
	for ( i = 0; i < players.size; i++ )
	{
		players[i].leon_dodged = undefined; // (a dodge counts for its own tail alone)
		if ( !is_player_valid( players[i] ) || !self verdugo_tail_reaches( players[i] ) )
			continue;
		players[i].leon_dodge_window = true;
		players[i] thread verdugo_dodge_watch();
		offered[ offered.size ] = players[i];
	}
	level thread verdugo_dodge_close( offered, 1.33 ); // (closed even when his flinch or his death cut the tail short)
	logPrint( "leon: verdugo tail at " + getTime() + ", " + offered.size + " offered the dodge\n" );
	if ( getDvar( "leon_verdugotest" ) == "3" && count % 2 == 0 )
		level thread verdugo_test_dodge( offered ); // (tests: every other one dodged, as the key would)
	wait 1.33;
	model playSound( "verdugo_tail" );
	wait 0.14;
	struck = [];
	for ( step = 0; step < 23; step++ )
	{
		players = get_players();
		for ( i = 0; i < players.size; i++ )
		{
			p = players[i];
			if ( !is_player_valid( p ) || isDefined( p.leon_dodged ) || verdugo_listed( struck, p ) )
				continue;
			if ( !self verdugo_tail_reaches( p ) )
				continue;
			struck[ struck.size ] = p;
			model playSound( "verdugo_hit" );
			self maps\leon\_rtenemy::hit( p, 0, true );
			logPrint( "leon: verdugo's tail struck player " + p getEntityNumber() + " at " + getTime() + "\n" );
		}
		wait 0.05;
	}
	dodged = false;
	for ( i = 0; i < offered.size; i++ )
	{
		if ( isDefined( offered[i] ) && isDefined( offered[i].leon_dodged ) )
			dodged = true;
	}
	if ( dodged )
	{
		self verdugo_taunt( model, yaw[1] );
		return;
	}
	wait 1.9;
	self maps\leon\_rtenemy::free();
}

// the tail's reach (self: his zombie): 150 in front, a wide arc (its sweep)
verdugo_tail_reaches( player )
{
	offset = player.origin - self.origin;
	flat = ( offset[0], offset[1], 0 );
	d = length( flat );
	if ( d > 150 || abs( offset[2] ) > 60 )
		return false;
	return d < 1 || vectorDot( anglesToForward( ( 0, self.angles[1], 0 ) ), vectorNormalize( flat ) ) >= 0.3;
}

verdugo_listed( list, player )
{
	for ( i = 0; i < list.size; i++ )
	{
		if ( list[i] == player )
			return true;
	}
	return false;
}

// the tail's dodge window over, after seconds
verdugo_dodge_close( players, seconds )
{
	wait seconds;
	for ( i = 0; i < players.size; i++ )
	{
		if ( isDefined( players[i] ) )
		{
			players[i].leon_dodge_window = undefined;
			players[i] notify( "leon_dodge_closed" );
		}
	}
}

// self: a player offered the tail's dodge - the kick key taken in its window (maps\leon\_kick.gsc: leon_dodge)
verdugo_dodge_watch()
{
	self endon( "disconnect" );
	self endon( "leon_dodge_closed" );
	self waittill( "leon_dodge" );
	self.leon_dodged = true;
	self.leon_dodge_window = undefined;
	logPrint( "leon: verdugo - player " + self getEntityNumber() + " dodged at " + getTime() + "\n" );
}

verdugo_test_dodge( players )
{
	wait 0.4;
	for ( i = 0; i < players.size; i++ )
	{
		if ( isDefined( players[i] ) )
			players[i] notify( "leon_dodge" );
	}
}

// his roar at them (Threat 0x044, 100 frames: 3.3 s), his taunt: as he comes, and after a dodged tail
verdugo_taunt( model, yaw )
{
	self endon( "death" );
	self endon( "leon_interrupt" );
	self.leon_busy = "taunt";
	self maps\leon\_rtenemy::hold( yaw );
	model maps\leon\_rtenemy::motion( "taunt" );
	model playSound( "verdugo_roar" );
	logPrint( "leon: verdugo taunts at " + getTime() + "\n" );
	wait 3.3;
	self maps\leon\_rtenemy::free();
}

// maps\leon\_kick.gsc's stagger on him (self): his flinch (Dm_Normal 0x01D, 82 frames: 2.7 s), stepping back as RE4's
// does - an attack it cuts short
stagger()
{
	self endon( "death" );
	model = self.leon_model;
	self notify( "leon_interrupt" );
	self.leon_busy = "stagger";
	self.leon_stagger_end = getTime() + 2700;
	yaw = self.angles[1];
	self maps\leon\_rtenemy::hold( yaw );
	model maps\leon\_rtenemy::motion( "stagger" );
	model playSound( "verdugo_pain" );
	self thread maps\leon\_rtenemy::travel( "0 2 -2 -13 -19 -26 -31 -34 -36 -38 -39 -40 -41 -41 -42 -41 -41 -40 -39 -38 "
	                                         + "-38 -36 -35 -35 -34 -34 -34 -33", yaw );
	logPrint( "leon: verdugo staggered at " + getTime() + "\n" );
	wait 2.7;
	if ( isDefined( self.leon_busy ) && self.leon_busy == "stagger" )
		self maps\leon\_rtenemy::free();
}

// Five's floors: a zombie keeps to its own, and the players take the elevators - out of their reach (12 s with no 50" of
// ground made on the nearest, more than 350 off), he comes again by them
verdugo_unstick()
{
	self endon( "death" );
	best = undefined;
	since = getTime();
	for ( ;; )
	{
		wait 1;
		if ( isDefined( self.leon_busy ) )
		{
			since = getTime();
			continue;
		}
		players = get_players();
		nearest = undefined;
		d = 100000;
		for ( i = 0; i < players.size; i++ )
		{
			if ( is_player_valid( players[i] ) && distance( players[i].origin, self.origin ) < d )
			{
				nearest = players[i];
				d = distance( players[i].origin, self.origin );
			}
		}
		if ( !isDefined( nearest ) || d < 350 || !isDefined( best ) || d < best - 50 )
		{
			best = d;
			since = getTime();
			continue;
		}
		if ( getTime() - since < 12000 )
			continue;
		origin = maps\_zombiemode::check_for_valid_spawn_near_team( nearest );
		if ( isDefined( origin ) )
		{
			yaw = vectorToAngles( nearest.origin - origin );
			self forceTeleport( origin, ( 0, yaw[1], 0 ) );
			if ( isDefined( level._effect ) && isDefined( level._effect["ape_spawn"] ) )
				playFx( level._effect["ape_spawn"], origin );
			self.leon_model playSound( "verdugo_scream" );
			logPrint( "leon: verdugo came again by player " + nearest getEntityNumber() + " at " + getTime() + "\n" );
		}
		best = undefined;
		since = getTime();
	}
}

// his kill ends Five's special round as the thief's does (_zombiemode_ai_thief thief_zombie_die): a Fire Sale where he
// fell and a Max Ammo behind it, its announcer
verdugo_died()
{
	self endon( "leon_left" );
	self waittill( "death", attacker );
	origin = self.origin;
	forward = anglesToForward( ( 0, self.angles[1], 0 ) );
	by = "nobody";
	if ( isDefined( attacker ) && isPlayer( attacker ) )
		by = "player " + attacker getEntityNumber();
	else if ( isDefined( attacker ) && isDefined( attacker.classname ) )
		by = attacker.classname;
	logPrint( "leon: verdugo died at " + getTime() + ", killed by " + by + "\n" );
	if ( isDefined( level.zombie_powerups ) && isDefined( level.zombie_powerups["fire_sale"] ) )
		level thread maps\_zombiemode_powerups::specific_powerup_drop( "fire_sale", origin );
	level thread maps\_zombiemode_powerups::specific_powerup_drop( "full_ammo", origin - forward * 32 );
	if ( isDefined( level.flag ) && isDefined( level.flag["last_thief_down"] ) )
	{
		flag_set( "last_thief_down" );
		play_sound_2D( "zmb_vox_pentann_thiefend_good" );
	}
}

// still alive after seconds he leaves, as the thief leaves at its round's end (_zombiemode_ai_thief thief_exit_level):
// a Max Ammo at the thief's start, its announcer
verdugo_leaves( model, seconds )
{
	self endon( "death" );
	wait seconds;
	self notify( "leon_left" );
	logPrint( "leon: verdugo left at " + getTime() + "\n" );
	start = getstruct( "thief_start", "targetname" );
	if ( isDefined( start ) )
		level thread maps\_zombiemode_powerups::specific_powerup_drop( "full_ammo", start.origin );
	if ( isDefined( level.flag ) && isDefined( level.flag["last_thief_down"] ) )
	{
		flag_set( "last_thief_down" );
		play_sound_2D( "zmb_vox_pentann_thiefend_bad" );
	}
	model.leon_left = true;
	self delete();
}
