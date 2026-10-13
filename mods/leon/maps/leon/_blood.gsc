#include maps\_utility;
#include common_scripts\utility;

// LEON MOD - not part of the original game. More blood when Leon hits a zombie, on top of the game's own impact
// puffs (gameplay feedback): the effects all zombie maps carry (common_zombie). Every bullet hit sprays out of the
// wound along the bullet, a head shot more, a kill a large spray and a splat on the ground; a knife hit its flesh hit.
// One extra effect a zombie every 80 ms at most (an automatic weapon would flood the effects).
//   leon_bloodtest 1   tests: the nearest zombie bleeds as if shot, every 1.5 s

init()
{
	level._effect["leon_blood_exit"] = loadfx( "impacts/fx_flesh_hit_body_fatal_exit" );
	level._effect["leon_blood_exit_lg"] = loadfx( "impacts/fx_flesh_hit_body_fatal_lg_exit" );
	level._effect["leon_blood_head"] = loadfx( "impacts/fx_flesh_hit_neck_fatal" );
	level._effect["leon_blood_splat"] = loadfx( "misc/fx_zombie_bloodsplat" );
	level._effect["leon_blood_knife"] = loadfx( "impacts/fx_flesh_hit_knife" );
	level thread leon_blood_zombies();
	if ( getDvar( "leon_bloodtest" ) == "1" )
		level thread leon_blood_test();
}

leon_blood_zombies()
{
	for ( ;; )
	{
		zombies = getAiArray( "axis" );
		for ( i = 0; i < zombies.size; i++ )
		{
			if ( !isDefined( zombies[i].leon_blood_watched ) )
			{
				zombies[i].leon_blood_watched = true;
				zombies[i] thread leon_blood_watch();
			}
		}
		wait 0.2;
	}
}

leon_blood_watch()
{
	self endon( "death" );
	for ( ;; )
	{
		self waittill( "damage", amount, attacker, dir, point, mod, modelName, tagName, partName );
		if ( !isDefined( attacker ) || !isPlayer( attacker ) || !isDefined( point ) || !isDefined( mod ) )
			continue;
		part = "";
		if ( isDefined( partName ) )
			part = partName;
		self leon_bleed( point, dir, mod, part, self.health <= 0 );
	}
}

// the blood for one hit (also the test's)
leon_bleed( point, dir, mod, part, killed )
{
	if ( isDefined( self.leon_blood_time ) && getTime() < self.leon_blood_time + 80 )
		return;
	self.leon_blood_time = getTime();
	if ( !isDefined( dir ) || lengthSquared( dir ) < 0.01 )
		dir = anglesToForward( self.angles ) * -1;
	dir = vectorNormalize( dir );
	if ( mod == "MOD_MELEE" )
	{
		playFx( level._effect["leon_blood_knife"], point, dir );
		return;
	}
	if ( mod != "MOD_PISTOL_BULLET" && mod != "MOD_RIFLE_BULLET" && mod != "MOD_HEAD_SHOT" )
		return;
	head = isSubStr( part, "head" ) || isSubStr( part, "neck" ) || mod == "MOD_HEAD_SHOT";
	if ( killed )
	{
		playFx( level._effect["leon_blood_exit_lg"], point, dir );
		ground = groundpos( point );
		playFx( level._effect["leon_blood_splat"], ground + ( 0, 0, 1 ), ( 0, 0, 1 ) );
	}
	else if ( head )
		playFx( level._effect["leon_blood_head"], point, dir );
	else
		playFx( level._effect["leon_blood_exit"], point, dir );
}

// tests: the nearest zombie bleeds every 1.5 s (a hit; every third a kill's spray), the hit point on its chest
leon_blood_test()
{
	for ( n = 0; ; n++ )
	{
		wait 1.5;
		players = getEntArray( "player", "classname" );
		zombies = getAiArray( "axis" );
		if ( players.size == 0 || zombies.size == 0 )
			continue;
		z = getClosest( players[0].origin, zombies );
		dir = vectorNormalize( z.origin - players[0].origin );
		z.leon_blood_time = undefined;
		z leon_bleed( z.origin + ( 0, 0, 50 ), dir, "MOD_RIFLE_BULLET", "torso", n % 3 == 2 );
		logPrint( "leon: bloodtest at " + z.origin + "\n" );
	}
}
