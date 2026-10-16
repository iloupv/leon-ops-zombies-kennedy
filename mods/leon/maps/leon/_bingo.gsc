#include maps\_utility;
#include common_scripts\utility;

// LEON MOD - not part of the original game. Leon's "Where's everyone going? Bingo?" (the runtime sound leon_bingo, from
// the player's own RE4: tools/leon-converter/leon_sounds.py) once at the start of a map, ~9.5 s in, and as a special
// round starts: the Hellhound rounds (_zombiemode_ai_dogs' flag
// "dog_round"), "Five"'s thief (flag "thief_round"), the monkey rounds (flag "monkey_round") and Moon's crawler rounds
// (level notify "crawler_round_starting"). Once a round, 6.5 s in: after the round's own start has played (the dog
// round's stinger, zmb_dog_round_start, runs 5.7 s, its announcer from 1 s in under it); no line when the sound is
// not there. The dog round has his "Hey, it's that dog." instead (leon_that_dog: the village square's dog, RE4's
// r119s10), and the Nuke his "Hasta luego!" (leon_hasta_luego: to Mendez in the barn, r11fs02) as he grabs it
// (_zombiemode_powerups' powerup_grab: "nuke_triggered" on the player who took it, when he is Leon). Leon's lines are
// heard by every player, while one of them plays Leon (maps\leon\_character).
//   leon_bingo 0     off (the special rounds' and the map start's lines)
//   leon_bingostart 0  not at the start of the map
//   leon_soundlen "<alias> ..."  test: each alias played in turn, its length logged (headless: bo1_snd_nodevice_length)

init()
{
	if ( getDvar( "leon_bingo" ) == "0" )
		return;
	level thread leon_bingo_flags();
	level thread leon_bingo_notify( "crawler_round_starting" );
	if ( getDvar( "leon_bingostart" ) != "0" )
		level thread leon_bingo_start();
	if ( getDvar( "leon_soundlen" ) != "" )
		level thread leon_sound_lengths( getDvar( "leon_soundlen" ) );
}

leon_sound_lengths( list )
{
	wait 3;
	players = getEntArray( "player", "classname" );
	aliases = strTok( list, " " );
	for ( i = 0; i < aliases.size; i++ )
	{
		start = getTime();
		players[0] playSound( aliases[i], "leon_soundlen_done" );
		players[0] waittill( "leon_soundlen_done" );
		logPrint( "leon: soundlen " + aliases[i] + " " + ( getTime() - start ) + " ms\n" );
	}
}

leon_bingo_flags()
{
	names = strTok( "dog_round thief_round monkey_round", " " );
	was = [];
	for ( i = 0; i < names.size; i++ )
		was[i] = false;
	for ( ;; )
	{
		wait 0.25;
		for ( i = 0; i < names.size; i++ )
		{
			on = isDefined( level.flag ) && isDefined( level.flag[ names[i] ] ) && level.flag[ names[i] ];
			if ( on && !was[i] )
				level thread leon_bingo_say( names[i] );
			was[i] = on;
		}
	}
}

leon_bingo_notify( name )
{
	for ( ;; )
	{
		level waittill( name );
		level thread leon_bingo_say( name );
	}
}

// at the start of the map, once its player is in: 3 s, then the special round's 6.5 s (leon_bingo_say)
leon_bingo_start()
{
	players = getEntArray( "player", "classname" );
	while ( players.size == 0 )
	{
		wait 0.5;
		players = getEntArray( "player", "classname" );
	}
	wait 3;
	level thread leon_bingo_say( "start" );
}

leon_bingo_say( why )
{
	wait 6.5;
	players = getEntArray( "player", "classname" );
	if ( !leon_played_by( players ) )
		return;
	alias = "leon_bingo";
	if ( why == "dog_round" && soundExists( "leon_that_dog" ) )
		alias = "leon_that_dog";
	if ( !soundExists( alias ) )
		return;
	for ( i = 0; i < players.size; i++ )
		players[i] playLocalSound( alias );
	logPrint( "leon: " + alias + " (" + why + ")\n" );
}

// whether one of the players plays Leon (his lines are heard by all)
leon_played_by( players )
{
	for ( i = 0; i < players.size; i++ )
		if ( players[i] maps\leon\_character::current() == "leon" )
			return true;
	return false;
}

// self: a player coming in (_leon's setup) - his "Hasta luego!" when he grabs a Nuke as Leon, heard by all
player_init()
{
	self endon( "disconnect" );
	for ( ;; )
	{
		self waittill( "nuke_triggered" );
		if ( self maps\leon\_character::current() != "leon" )
			continue;
		if ( soundExists( "leon_hasta_luego" ) )
		{
			players = getEntArray( "player", "classname" );
			for ( i = 0; i < players.size; i++ )
				players[i] playLocalSound( "leon_hasta_luego" );
		}
		logPrint( "leon: hasta luego\n" );
	}
}
