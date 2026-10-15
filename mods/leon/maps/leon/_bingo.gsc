#include maps\_utility;
#include common_scripts\utility;

// LEON MOD - not part of the original game. Leon's "Bingo?" (the runtime sound leon_bingo, from the player's own RE4:
// tools/leon-converter/leon_sounds.py) as a special round starts: the Hellhound rounds (_zombiemode_ai_dogs' flag
// "dog_round"), "Five"'s thief (flag "thief_round"), the monkey rounds (flag "monkey_round") and Moon's crawler rounds
// (level notify "crawler_round_starting"). Once a round, 6.5 s in: after the round's own start has played (the dog
// round's stinger, zmb_dog_round_start, runs 5.7 s, its announcer from 1 s in under it); no line when the sound is
// not there.
//   leon_bingo 0     off
//   leon_bingotest 1 test: the line as if a special round started, 3 s into the level
//   leon_soundlen "<alias> ..."  test: each alias played in turn, its length logged (headless: bo1_snd_nodevice_length)

init()
{
	if ( getDvar( "leon_bingo" ) == "0" )
		return;
	level thread leon_bingo_flags();
	level thread leon_bingo_notify( "crawler_round_starting" );
	if ( getDvar( "leon_bingotest" ) == "1" )
		level thread leon_bingo_test();
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

leon_bingo_test()
{
	wait 3;
	level thread leon_bingo_say( "test" );
}

leon_bingo_say( why )
{
	wait 6.5;
	if ( !soundExists( "leon_bingo" ) )
		return;
	players = getEntArray( "player", "classname" );
	for ( i = 0; i < players.size; i++ )
		players[i] playLocalSound( "leon_bingo" );
	logPrint( "leon: bingo (" + why + ")\n" );
}
