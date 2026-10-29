#include maps\_utility;
#include common_scripts\utility;

// LEON MOD - not part of the original game. The knife under RE4's controls. A knife bought off the wall (the Bowie:
// _zombiemode_bowie) is his at once, with no flourish: the engine leaves the flourish's switches out (G_LeonSkipSwitch),
// as a perk bottle's, so his gun stays up, aimed or not, and his client plays RE4's knife slash once with the new knife
// as it becomes his melee weapon (cg_pose_mp.cpp CG_LeonBoughtKnife); BO1's own buy still runs - its cost, its line,
// the knife given as the melee weapon. Here only its test:
//   leon_bowietest <s>   test: the player buys the Bowie off the wall that many seconds into the level, once aimed in
//                        (or 20 s later)

init()
{
	if ( getDvar( "leon_bowietest" ) != "" )
		level thread leon_bowietest( getDvarFloat( "leon_bowietest" ) );
}

// Test: the wall knife's use by the first player, 5000 points richer (its trigger's own "trigger", as his use key gives
// it), at `at` seconds once he is aimed in (or 20 s later); then each 50 ms whether he still holds his gun and is aimed,
// and whether the Bowie is his melee weapon, the knife gone and the buy over (he is drinking no more)
leon_bowietest( at )
{
	while ( getTime() < at * 1000 )
		wait 0.05;
	players = getEntArray( "player", "classname" );
	triggers = getEntArray( "bowie_upgrade", "targetname" );
	if ( !players.size || !triggers.size )
	{
		logPrint( "leon: bowietest no wall knife\n" );
		return;
	}
	player = players[0];
	player.score = player.score + 5000;
	for ( t = 0; t < 400 && player playerADS() < 1; t++ )
		wait 0.05;
	logPrint( "leon: bowietest buys at " + getTime() + " holding " + player getCurrentWeapon() + ", aimed "
		+ player playerADS() + "\n" );
	triggers[0] notify( "trigger", player );
	for ( t = 0; t < 30; t++ )
	{
		wait 0.05;
		logPrint( "leon: bowietest +" + ( t + 1 ) * 50 + " ms holding " + player getCurrentWeapon() + ", aimed "
			+ player playerADS() + ", bowie " + player hasWeapon( "bowie_knife_zm" ) + ", knife "
			+ player hasWeapon( "knife_zm" ) + "\n" );
	}
	drinking = 0;
	if ( isDefined( player.is_drinking ) )
		drinking = player.is_drinking;
	logPrint( "leon: bowietest had bowie " + player hasWeapon( "bowie_knife_zm" ) + ", drinking " + drinking + ", holding "
		+ player getCurrentWeapon() + ", score " + player.score + "\n" );
}

