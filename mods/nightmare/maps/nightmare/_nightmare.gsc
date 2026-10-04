#include maps\_utility;
#include common_scripts\utility;

// NIGHTMARE MOD - not part of the original game. Five with the special enemies of the other maps (mods/nightmare/README.md).
// nightmare_level 0 = off (the mod only adds files), 1 = Nightmare (one of each special), 2 = Hell (several at once).
// Each count is a dvar with a level preset; a value already set (command line, horde's launcher flags) wins.

// Five's hook at the start of maps\zombie_pentagon::main(): before the zone scripts (they run after the level main)
main_start()
{
	level.nightmare_level = GetDvarInt( #"nightmare_level" );
	if ( level.nightmare_level <= 0 )
		return;
	hell = level.nightmare_level >= 2;
	// L60b (user, round 2): every special from the first round (nightmare_start_round, default 1), then they keep coming
	// on respawn timers (seconds after the last death of that type) instead of retail's round cadence. Before the first
	// zombie is through a window (~45 s on Five's round 1) they spawn at path nodes near a player (_modenemy).
	nightmare_default( "nightmare_start_round", 1 );
	start = GetDvarInt( #"nightmare_start_round" );
	if ( start < 1 )
		start = 1;
	level.mod_special_node_spots = true;
	// Shangri-La (maps\_modenemies\_napalm / _sonic read the horde_enemy_* dvars)
	nightmare_force_on( "horde_enemy_napalm" );
	nightmare_force_on( "horde_enemy_sonic" );
	nightmare_default( "horde_enemy_napalm_max", nightmare_pick( hell, 1, 3 ) );
	nightmare_default( "horde_enemy_sonic_max", nightmare_pick( hell, 1, 3 ) );
	nightmare_default( "horde_enemy_napalm_round", start );
	nightmare_default( "horde_enemy_sonic_round", start );
	nightmare_default( "horde_enemy_napalm_respawn", nightmare_pick( hell, 60, 25 ) );
	nightmare_default( "horde_enemy_sonic_respawn", nightmare_pick( hell, 60, 25 ) );
	// Call of the Dead / Moon (maps\_modenemies\_director / _astro)
	nightmare_default( "nightmare_george_max", nightmare_pick( hell, 1, 3 ) );
	nightmare_default( "nightmare_george_round", start );
	nightmare_default( "nightmare_george_pace", nightmare_pick( hell, 10, 4 ) );
	nightmare_default( "nightmare_george_respawn", nightmare_pick( hell, 30, 20 ) );
	nightmare_default( "nightmare_astro_max", nightmare_pick( hell, 1, 3 ) );
	nightmare_default( "nightmare_astro_round", start );
	nightmare_default( "nightmare_astro_respawn", nightmare_pick( hell, 45, 20 ) );
	if ( IsSubStr( GetDvar( #"bo1_mod_zones" ), "zombie_coast" ) )
		maps\_modenemies\_director::main_start();
	// L61: hellhound packs from the first round on maps with native dogs (Kino), not only on dog rounds
	// (maps\_modenemies\_dogs); Five keeps its retail dogs (no dog spawner in BO1Zombies's Five, see below)
	if ( Tolower( GetDvar( #"mapname" ) ) != "zombie_pentagon" )
	{
		nightmare_default( "nightmare_dog_round", start );
		nightmare_default( "nightmare_dog_pack", nightmare_pick( hell, 2, 4 ) );
		nightmare_default( "nightmare_dog_interval", nightmare_pick( hell, 60, 30 ) );
		nightmare_default( "nightmare_dog_max", nightmare_pick( hell, 4, 8 ) );
		maps\_modenemies\_dogs::init();
	}
	// L61: Five's thief on other maps (maps\_modenemies\_thief; Five keeps its retail thief round): one at a time from the
	// start round, back nightmare_thief_respawn s after the last one died or escaped. His death drop: bonfire sale (only
	// from him, _zombiemode_powerups.gsc:413) included here, before _zombiemode_powerups::init
	if ( Tolower( GetDvar( #"mapname" ) ) != "zombie_pentagon" && IsSubStr( GetDvar( #"bo1_mod_zones" ), "zombie_pentagon" ) )
	{
		nightmare_default( "nightmare_thief_round", start );
		nightmare_default( "nightmare_thief_respawn", nightmare_pick( hell, 90, 40 ) );
		nightmare_default( "nightmare_thief_flee", 30 );
		maps\_zombiemode_powerups::include_zombie_powerup( "bonfire_sale" );
	}
	// L62: Ascension's space monkeys on other maps (maps\_modenemies\_monkey): packs from the start round going for perk
	// machines, at most nightmare_monkey_packs packs at once, the next nightmare_monkey_respawn s after one is down
	if ( Tolower( GetDvar( #"mapname" ) ) != "zombie_cosmodrome" && IsSubStr( GetDvar( #"bo1_mod_zones" ), "zombie_cosmodrome" ) )
	{
		nightmare_default( "nightmare_monkey_round", start );
		nightmare_default( "nightmare_monkey_packs", nightmare_pick( hell, 1, 2 ) );
		nightmare_default( "nightmare_monkey_per_pack", 3 );
		nightmare_default( "nightmare_monkey_respawn", nightmare_pick( hell, 60, 25 ) );
	}
	LogPrint( "nightmare: level " + level.nightmare_level + " napalm max " + GetDvarInt( #"horde_enemy_napalm_max" ) + " sonic max " + GetDvarInt( #"horde_enemy_sonic_max" ) + " george max " + GetDvarInt( #"nightmare_george_max" ) + " astro max " + GetDvarInt( #"nightmare_astro_max" ) + "\n" );
	// Five's thief and hellhounds stay retail. Thief: his round needs the power on (_zombiemode_ai_thief.gsc:430) and
	// replaces a whole zombie round. Hellhounds: an early pack (retail special_dog_spawn, _zombiemode_ai_dogs.gsc:854)
	// needs a dog spawner, and BO1Zombies's Five has none at level time 70 s (L60b r2h4: GetEntArray
	// "zombie_spawner_dog_init" targetname 0, "zombie_dog_spawner" script_noteworthy 0).
}


// L61: Kino (zombie_theater) and any map other than Five: from maps\_zombiemode_ffotd::main_start (_zombiemode.gsc:19,
// before _zombiemode_powerups::init at :105, so George's include_powerup( "free_perk" ) still counts)
main_start_any_map()
{
	// L62 measurement (any level, e.g. zombie_cosmodrome): the perk machines' monkey attack spots to the game log
	if ( GetDvarInt( #"nightmare_log_perk_spots" ) )
		level thread maps\_modenemies\_monkey_spots::log_perk_spots();
	if ( Tolower( GetDvar( #"mapname" ) ) == "zombie_pentagon" || IsDefined( level.nightmare_level ) )
		return;
	main_start();
}


temple_zone()
{
	if ( !IsDefined( level.nightmare_level ) || level.nightmare_level <= 0 )
		return;
	maps\_modenemies\_napalm::init();
	maps\_modenemies\_sonic::init();
}


coast_zone()
{
	if ( !IsDefined( level.nightmare_level ) || level.nightmare_level <= 0 )
		return;
	maps\_modenemies\_director::init();
}


moon_zone()
{
	if ( !IsDefined( level.nightmare_level ) || level.nightmare_level <= 0 )
		return;
	maps\_modenemies\_astro::init();
}


pentagon_zone()
{
	if ( !IsDefined( level.nightmare_level ) || level.nightmare_level <= 0 || Tolower( GetDvar( #"mapname" ) ) == "zombie_pentagon" )
		return;
	maps\_modenemies\_thief::init();
}


cosmodrome_zone()
{
	if ( !IsDefined( level.nightmare_level ) || level.nightmare_level <= 0 || Tolower( GetDvar( #"mapname" ) ) == "zombie_cosmodrome" )
		return;
	[[ level.mod_zone_enemy_init["monkey"] ]]();
}


nightmare_pick( hell, normalValue, hellValue )
{
	if ( hell )
		return hellValue;
	return normalValue;
}


nightmare_force_on( name )
{
	if ( !GetDvarInt( name ) )
		SetDvar( name, "1" );
}


nightmare_default( name, value )
{
	if ( GetDvar( name ) == "" )
		SetDvar( name, "" + value );
}
