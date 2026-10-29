#include maps\_utility;
#include common_scripts\utility;
#include maps\_zombiemode_utility;

// LEON MOD - not part of the original game. A headless test (a headless client sends no input) of the corpses the
// Thundergun leaves, from maps\leon\_leon.gsc:
//   leon_corpsetest <n>    n zombies run at the player (who cannot be hurt; the round's own stop coming) from the dogs'
//                          spots; once three are near him he is turned to them and the Thundergun he holds (leon_give
//                          thundergun_zm) fired - _zombiemode_weap_thundergun's blast and fling on his "weapon_fired" -
//                          again 5 s on, over the corpses, and at those still coming until none is left (six shots at
//                          most); then nothing more, the corpses watched. Each death logged with its entity number (the
//                          engine's bo1_ragdolltrace names the bodies by it).

test( count )
{
	self endon( "disconnect" );
	self enableInvulnerability();
	flag_wait( "begin_spawning" ); // (the match under way)
	level.zombie_total = 0; // (the round's own: none more - only the test's)
	flag_clear( "spawn_zombies" ); // (nor the next round's: the corpses are watched with no zombie about)
	wait 1;
	spots = [];
	if ( isDefined( level.enemy_dog_locations ) )
		spots = level.enemy_dog_locations;
	if ( spots.size == 0 && isDefined( level.enemy_spawns ) )
		spots = level.enemy_spawns;
	near = [];
	for ( i = 0; i < spots.size; i++ )
	{
		d = distance( spots[i].origin, self.origin );
		if ( d > 300 && d < 1100 )
			near[ near.size ] = spots[i];
	}
	if ( near.size == 0 )
		near = spots;
	logPrint( "leon: corpsetest - " + count + " zombies from " + near.size + " spots, the player at " + self.origin + "\n" );
	for ( i = 0; i < count; i++ )
	{
		ai = spawn_chaser( near[ i % near.size ].origin );
		if ( isDefined( ai ) )
			ai thread death_log();
		wait 0.3;
	}
	aimed = ( 0, 0, 0 );
	for ( shot = 0; shot < 6; shot++ )
	{
		close = [];
		alive = 0;
		for ( t = 0; t < 120; t++ )
		{
			wait 0.25;
			zombies = getAiSpeciesArray( "axis", "all" );
			close = [];
			alive = 0;
			for ( i = 0; i < zombies.size; i++ )
			{
				if ( !isAlive( zombies[i] ) )
					continue;
				alive++;
				if ( distance( zombies[i].origin, self.origin ) < 260 )
					close[ close.size ] = zombies[i];
			}
			if ( close.size >= 3 || ( close.size > 0 && t > 12 ) || ( shot == 1 && t >= 20 ) || ( shot >= 2 && alive == 0 ) )
				break;
		}
		if ( shot >= 2 && alive == 0 )
			break;
		// turned to the near ones' middle (or kept as he was: the second shot goes where the first flung them)
		if ( close.size > 0 )
		{
			middle = ( 0, 0, 0 );
			for ( i = 0; i < close.size; i++ )
				middle = middle + close[i].origin;
			middle = vector_scale( middle, 1.0 / close.size );
			self setPlayerAngles( vectorToAngles( middle + ( 0, 0, 30 ) - self getEye() ) );
		}
		wait 0.05;
		logPrint( "leon: corpsetest - thundergun fired at " + getTime() + ", " + close.size + " near, holding "
		          + self getCurrentWeapon() + "\n" );
		self notify( "weapon_fired" );
		angles = self getPlayerAngles();
		aimed = aimed + vector_scale( anglesToForward( ( 0, angles[1], 0 ) ), close.size );
	}
	// then turned, looking a little down, the way most were flung (screenshots: the corpses in view)
	if ( length( aimed ) > 0 )
	{
		angles = vectorToAngles( aimed );
		self setPlayerAngles( ( 12, angles[1], 0 ) );
	}
	wait 1;
	zombies = getAiSpeciesArray( "axis", "all" );
	logPrint( "leon: corpsetest - done at " + getTime() + ", " + zombies.size + " left\n" );
}

// one of the map's zombie spawners spawns a zombie at origin, running straight at the player (a chaser: no window)
spawn_chaser( origin )
{
	if ( !isDefined( level.enemy_spawns ) || level.enemy_spawns.size == 0 )
		return undefined;
	spawner = level.enemy_spawns[ randomInt( level.enemy_spawns.size ) ];
	keptOrigin = spawner.origin;
	keptString = spawner.script_string;
	keptTarget = spawner.target;
	spawner.origin = origin;
	spawner.script_string = "zombie_chaser";
	spawner.target = "";
	ai = spawn_zombie( spawner );
	spawner.origin = keptOrigin;
	spawner.script_string = keptString;
	spawner.target = keptTarget;
	if ( !isDefined( ai ) )
		return undefined;
	ai forceTeleport( origin );
	ai maps\_zombiemode_spawner::set_zombie_run_cycle( "run" );
	return ai;
}

death_log()
{
	num = self getEntityNumber();
	self waittill( "death" );
	wait 0.05; // (the fling marks it after the damage that killed it)
	how = "";
	if ( isDefined( self ) && isDefined( self.thundergun_death ) )
		how = ", flung";
	if ( isDefined( self ) )
		how = how + " at " + self.origin;
	logPrint( "leon: corpsetest - zombie " + num + " died at " + getTime() + how + "\n" );
}
