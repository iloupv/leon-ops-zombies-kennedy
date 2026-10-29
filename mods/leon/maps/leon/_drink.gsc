#include maps\_utility;
#include common_scripts\utility;

// LEON MOD - not part of the original game. A perk machine's perk is his at once, with no drink: the engine leaves the
// bottle's switches out (G_LeonSkipSwitch), so his gun stays up, aimed or not. RE4 has no drink - an item used from the
// case plays its heal sound: the runtime sound "leon_heal" (from the player's own RE4: tools/leon-converter/
// leon_sounds.py), to him as Juggernog is his (the one perk that is health; the others sound BO1's own). On
// _zombiemode_perks' level.perk_bought_func, a map's own one still run.
//   leon_perktest <perk>  test: the player buys it, aimed in (specialty_quickrevive: solo, no power needed)

init()
{
	level.leon_perk_bought_func = level.perk_bought_func;
	level.perk_bought_func = ::leon_perk_bought;
	if ( getDvar( "leon_perktest" ) != "" )
		level thread leon_perktest( getDvar( "leon_perktest" ) );
}

// self: the player who bought it
leon_perk_bought( perk )
{
	if ( perk == "specialty_armorvest" )
		self playLocalSound( "leon_heal" );
	logPrint( "leon: perk " + perk + " holding " + self getCurrentWeapon() + ", aimed " + self playerADS() + "\n" );
	if ( isDefined( level.leon_perk_bought_func ) )
		self [[ level.leon_perk_bought_func ]]( perk );
}

// Test: the machine's use by the first player, 5000 points richer (its trigger's own "trigger", as his use key gives
// it), once he is aimed in (or after 20 s); then whether he has the perk, still holds his gun and is drinking no more
// (another buy would be refused)
leon_perktest( perk )
{
	wait 2;
	players = getEntArray( "player", "classname" );
	triggers = getEntArray( "zombie_vending", "targetname" );
	for ( i = 0; i < triggers.size; i++ )
	{
		if ( !players.size || !isDefined( triggers[i].script_noteworthy ) || triggers[i].script_noteworthy != perk )
			continue;
		player = players[0];
		player.score = player.score + 5000;
		for ( t = 0; t < 400 && player playerADS() < 1; t++ )
			wait 0.05;
		logPrint( "leon: perktest buys " + perk + " holding " + player getCurrentWeapon() + ", aimed " + player playerADS()
			+ "\n" );
		player setOrigin( triggers[i].origin ); // (at the machine, as a buyer is: its jingle as loud as he hears it)
		triggers[i] notify( "trigger", player );
		for ( t = 0; t < 20; t++ )
		{
			wait 0.05;
			logPrint( "leon: perktest +" + ( t + 1 ) * 50 + " ms holding " + player getCurrentWeapon() + ", aimed "
				+ player playerADS() + ", perk " + player hasPerk( perk ) + "\n" );
		}
		wait 1;
		drinking = 0;
		if ( isDefined( player.is_drinking ) )
			drinking = player.is_drinking;
		logPrint( "leon: perktest " + perk + " had " + player hasPerk( perk ) + ", drinking " + drinking + ", holding "
			+ player getCurrentWeapon() + "\n" );
		return;
	}
}
