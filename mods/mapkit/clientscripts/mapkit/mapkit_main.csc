#include clientscripts\_utility;
#include clientscripts\_music;
#include clientscripts\_zombiemode_weapons;

// MAPKIT MOD - the client level main of an "empty" layout (the engine loads it instead of clientscripts\<mapname>).
// Retail clientscripts\zombie_pentagon::main without the parts that need Five's world (placed fx, teleporter,
// box screens, office lights, vision volumes, ambient sound emitters).

main()
{
	level._uses_crossbow = true;
	level._power_on = false;
	clientscripts\zombie_pentagon::include_weapons(); // before _zombiemode::main (retail order)
	clientscripts\_zombiemode::main();
	// retail zombie_pentagon_fx::main minus the createfx placements: the zombie fx the shared client scripts use
	// (eye_glow for _zombiemode.csc createZombieEyes, headshot/blood/gib fx for the _callbacks.csc gib callback)
	// and the perk machine fx; without them every zombie spawn is a script error on the client
	clientscripts\zombie_pentagon_fx::precache_util_fx();
	clientscripts\zombie_pentagon_fx::precache_createfx_fx();
	clientscripts\zombie_pentagon_fx::precache_scripted_fx();
	level thread clientscripts\_zombiemode::init_perk_machines_fx();
	clientscripts\_zombiemode_deathcard::init();
	thread waitforclient( 0 );
	clientscripts\zombie_pentagon::register_zombie_types();
}
