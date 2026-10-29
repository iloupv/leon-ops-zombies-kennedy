#include maps\_utility;
#include common_scripts\utility;

// LEON MOD - not part of the original game. The knife under RE4's controls. A knife bought off the wall (the Bowie:
// _zombiemode_bowie) is his at once, with no flourish: the engine leaves the flourish's switches out (G_LeonSkipSwitch),
// as a perk bottle's, so his gun stays up, aimed or not, and his client plays RE4's knife slash once with the new knife
// as it becomes his melee weapon (cg_pose_mp.cpp CG_LeonBoughtKnife); BO1's own buy still runs - its cost, its line,
// the knife given as the melee weapon. The slash reaches farther and wider than BO1's (BG_PlayerMeleeRange: bg_leon.h's
// LEON_MELEE_RANGE_ADD, LEON_MELEE_SPREAD). Here only their tests:
//   leon_bowietest <s>   test: the player buys the Bowie off the wall that many seconds into the level, once aimed in
//                        (or 20 s later)
//   leon_knifetest <in>  test: from 25 s into the level the nearest zombie is held that many inches in front of the
//                        player, facing him; each hit on it logged (the key timetable presses the knife: hold 2)

init()
{
	if ( getDvar( "leon_bowietest" ) != "" )
		level thread leon_bowietest( getDvarFloat( "leon_bowietest" ) );
	if ( getDvar( "leon_knifetest" ) != "" )
		level thread leon_knifetest( getDvarFloat( "leon_knifetest" ) );
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

// Test: the nearest zombie held `dist` inches in front of the first player (origin to origin), facing him, from 25 s on -
// put back every frame, too tough to die; each damage it takes logged (a knife's: MOD_MELEE). The player cannot be hurt
// meanwhile.
leon_knifetest( dist )
{
	players = getEntArray( "player", "classname" );
	while ( !players.size )
	{
		wait 0.5;
		players = getEntArray( "player", "classname" );
	}
	player = players[0];
	player enableInvulnerability();
	while ( getTime() < 25000 )
		wait 0.05;
	for ( ;; )
	{
		wait 0.05;
		zombies = getAiArray( "axis" );
		if ( !zombies.size )
			continue;
		z = getClosest( player.origin, zombies );
		if ( !isDefined( z.leon_knifetest ) )
		{
			z.leon_knifetest = true;
			z.health = 100000; // (each slash a hit logged, none a kill: the same zombie held for every one)
			z thread leon_knifetest_hits( dist );
			logPrint( "leon: knifetest a zombie held at " + dist + " at " + getTime() + "\n" );
		}
		forward = anglesToForward( ( 0, player.angles[1], 0 ) );
		z forceTeleport( player.origin + forward * dist, ( 0, player.angles[1] + 180, 0 ) );
	}
}

leon_knifetest_hits( dist )
{
	self thread leon_knifetest_death( dist );
	self endon( "death" );
	for ( ;; )
	{
		self waittill( "damage", amount, attacker, dir, point, mod );
		logPrint( "leon: knifetest hit at " + dist + " at " + getTime() + ": " + amount + " " + mod + ", health "
			+ self.health + "\n" );
	}
}

// (a killing blow: its "damage" ends with the death that ends the thread above)
leon_knifetest_death( dist )
{
	self waittill( "death" );
	mod = "?";
	if ( isDefined( self ) && isDefined( self.damagemod ) )
		mod = self.damagemod;
	logPrint( "leon: knifetest killed at " + dist + " at " + getTime() + " by " + mod + "\n" );
}
