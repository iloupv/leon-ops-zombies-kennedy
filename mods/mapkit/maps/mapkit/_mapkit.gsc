#include maps\_utility;
#include common_scripts\utility;
#include maps\_zombiemode_utility;

// MAPKIT MOD runtime - applies the script side of the layout (bo1_mod_mapkit = mods/mapkit/layouts/<name>.json).
// The engine already built the layout's boxes/rooms (collision + drawing); this file does the edits scripts own:
//   perk   { perk, origin, snap:"wall" }  a vending machine + use trigger, placed before _zombiemode_perks::init
//   zone   { name, mins, maxs, join }     (logged; zone wiring comes next)
//   window { spawn, target, every, room } zombies of the round spawn outside a window (mapkit_window below)
//   powerswitch { name, origin, yaw, snap } the retail power switch; without one the empty base has power from the start
//   pap    { name, origin, yaw, snap }     the retail Pack-a-Punch machine (power-gated like the perks)
// Test aids (dvars, all off by default): mapkit_selftest 1 = trace probes across every box + player origin log
// + bo1_mapkit_dump("mapkit_dump.json") for tools/mapkit/topdown.mjs.

main_start()
{
	if ( bo1_mapkit_editcount() == 0 )
	{
		return;
	}
	level.mapkit_layout = bo1_mapkit_layout();
	LogPrint( "mapkit: runtime on layout " + level.mapkit_layout + ", " + bo1_mapkit_editcount() + " edits\n" );
	mapkit_precache();
	for ( i = 0; i < bo1_mapkit_editcount(); i++ )
	{
		kind = bo1_mapkit_get( i, "kind" );
		if ( kind == "perk" )
		{
			continue; // placed in main_end: Spawn() needs level.flag (_utility SpawnThrottleEnable)
		}
		else if ( kind == "room" || kind == "box" || kind == "stairs" )
		{
			bo1_mapkit_mark( kind, mapkit_name( i ), mapkit_center( i ), 0 );
		}
		else if ( kind == "zone" )
		{
			bo1_mapkit_mark( "zone", mapkit_name( i ), mapkit_center( i ), 0 );
		}
		else if ( kind == "door" || kind == "debris" )
		{
			clip = GetEnt( "mkdoor" + i, "targetname" );
			if ( IsDefined( clip ) )
			{
				clip Hide();
				if ( kind == "debris" )
				{
					clip DisconnectPaths();
				}
				bo1_mapkit_visible( i, true );
				bo1_mapkit_mark( kind, mapkit_name( i ), mapkit_center( i ), 0 );
			}
		}
		else if ( kind == "mysterybox" )
		{
			bo1_mapkit_mark( kind, mapkit_name( i ), mapkit_center( i ), 0 );
		}
	}
}

main_end()
{
	if ( bo1_mapkit_editcount() == 0 )
	{
		return;
	}
	if ( IsDefined( level.mapkit_empty ) )
	{
		mapkit_label_spawners();
	}
	for ( i = 0; i < bo1_mapkit_editcount(); i++ )
	{
		kind = bo1_mapkit_get( i, "kind" );
		if ( ( kind == "door" || kind == "debris" ) && IsDefined( level.mapkit_empty ) )
		{
			level thread mapkit_blocker_open( i );
		}
		if ( bo1_mapkit_get( i, "kind" ) == "perk" )
		{
			// mod: noperks 1 (mods/noperks stacked with fs_mods) - no perk machine, as noperks removes retail ones before
			// _zombiemode_perks::init; the engine adds no clip for it either (src/qcommon/cm_mapkit.cpp)
			if ( GetDvarInt( #"noperks" ) == 1 )
			{
				LogPrint( "mapkit: perk edit " + i + " (" + bo1_mapkit_get( i, "perk" ) + ") not placed: noperks 1\n" );
			}
			else
			{
				mapkit_place_perk( i );
			}
		}
		else if ( kind == "pap" )
		{
			mapkit_place_pap( i );
		}
		else if ( kind == "powerswitch" )
		{
			mapkit_place_powerswitch( i );
		}
		else if ( kind == "riser" && IsDefined( level.mapkit_empty ) && !IsDefined( level.mapkit_riser_watch ) )
		{
			level.mapkit_riser_watch = true;
			level thread mapkit_riser_watch();
		}
		else if ( bo1_mapkit_get( i, "kind" ) == "window" )
		{
			if ( IsDefined( level.mapkit_empty ) )
			{
				if ( mapkit_window_open( i ) )
				{
					mapkit_open_goals();
				}
				level thread mapkit_empty_window_watch( i );
			}
			else
			{
				level thread mapkit_window( i );
			}
		}
	}
	if ( IsDefined( level.mapkit_empty ) && IsDefined( level.flag ) && IsDefined( level.flag[ "power_on" ] ) && !IsDefined( level.mapkit_switch ) )
	{
		// empty base without a powerswitch edit: the power is on from the start (perks and doors that wait for it work)
		flag_set( "power_on" );
	}
	if ( IsDefined( level.mapkit_paps ) && !IsDefined( level.mapkit_switch ) )
	{
		level thread mapkit_pap_power();
	}
	level thread mapkit_round_log();
	// asset catalog (node tools/mapkit/catalog.mjs --refresh): +set mapkit_catalog 1 prints every loaded weapon/xmodel/material
	if ( GetDvar( "mapkit_catalog" ) == "1" )
	{
		bo1_mapkit_catalog();
	}
	// L43: the instant stress level (layout stress_arena): +set stress_count N spawns N zombies at once
	if ( GetDvarInt( #"stress_count" ) > 0 )
	{
		level thread mapkit_stress();
	}
	if ( GetDvar( "mapkit_selftest" ) == "1" )
	{
		level thread mapkit_selftest();
		level thread mapkit_check();
		if ( GetDvar( "mapkit_doctor" ) == "1" )
		{
			level thread mapkit_doctor_observe();
		}
	}
	if ( GetDvar( "mapkit_perktest" ) == "1" )
	{
		level thread mapkit_perktest();
	}
	// mapkit-fix-2: +set mapkit_open_all 1 opens every door/debris at spawn (free), e.g. to photograph areas behind them
	if ( GetDvar( "mapkit_open_all" ) == "1" )
	{
		level thread mapkit_open_all();
	}
	// mapkit-fix-4a: +set mapkit_boardshots 1 (passive god test client, -Client 1x): one window photographed boarded,
	// partly torn by zombies and rebuilt by the player with the use button (log lines "mapkit: boardshots ...")
	if ( GetDvar( "mapkit_boardshots" ) == "1" )
	{
		level thread mapkit_boardshots();
	}
	// mapkit-fix-4b: +set mapkit_clipshots 1 (passive god test client): the player walks from 64 in front into every
	// machine's centre and must stop outside its clip; zombies are sampled for standing inside a clip ("mapkit: clipshots")
	if ( GetDvar( "mapkit_clipshots" ) == "1" && GetDvar( "mapkit_boardshots" ) != "1" ) // with boardshots: after its END
	{
		level thread mapkit_clipshots();
	}
}

// The retail devgui open_sesame route (_zombiemode_devgui.gsc): with zombie_unlock_all set, a forced "trigger" notify
// makes door_buy / debris_think open without a use press or a charge; the flag mkdoor<i>_open then fires as in play.
mapkit_open_all()
{
	flag_wait( "all_players_spawned" );
	wait( 1 );
	players = get_players();
	SetDvar( "zombie_unlock_all", 1 );
	opened = 0;
	for ( i = 0; i < bo1_mapkit_editcount(); i++ )
	{
		kind = bo1_mapkit_get( i, "kind" );
		if ( kind != "door" && kind != "debris" )
		{
			continue;
		}
		trigs = GetEntArray( "zombie_" + kind, "targetname" );
		for ( t = 0; t < trigs.size; t++ )
		{
			if ( IsDefined( trigs[t].target ) && trigs[t].target == "mkdoor" + i )
			{
				trigs[t] notify( "trigger", players[0], true );
				opened++;
			}
		}
	}
	wait( 1 );
	SetDvar( "zombie_unlock_all", 0 );
	LogPrint( "mapkit: open_all triggered " + opened + " blockers\n" );
}

// L43: stress_count N. Retail spawning is paused (flag spawn_zombies) and N zombies are spawned at once, one per
// zombie spawner per frame (the engine refuses a second spawn from a spawner in the same level frame), each spawner
// moved onto the next pathnode at least 256 u from the player, as zombie_chaser (straight to find_flesh, no window).
// Logs "stress: spawned S of N in T ms" and then "stress: alive A t T" every 5 s (games_mp.log).
mapkit_stress()
{
	n = GetDvarInt( #"stress_count" );
	flag_wait( "all_players_spawned" );
	wait( 0.05 );
	flag_clear( "spawn_zombies" );
	level.zombie_ai_limit = n;
	SetAILimit( n );
	level.zombie_vars[ "zombie_use_failsafe" ] = false;
	// zombie_spawner's init reads level.zombie_health, which retail round_spawning sets at the round start
	if ( !IsDefined( level.zombie_health ) )
	{
		maps\_zombiemode::ai_calculate_health( level.round_number );
	}
	player = get_players()[ 0 ];
	all = GetAllNodes();
	nodes = [];
	for ( i = 0; i < all.size; i++ )
	{
		if ( DistanceSquared( all[ i ].origin, player.origin ) > 256 * 256 )
		{
			nodes[ nodes.size ] = all[ i ];
		}
	}
	spawners = GetEntArray( "zombie_spawner", "script_noteworthy" );
	LogPrint( "stress: count " + n + " nodes " + nodes.size + " spawners " + spawners.size + " time " + GetTime() + "\n" );
	if ( nodes.size == 0 || spawners.size == 0 )
	{
		return;
	}
	start = GetTime();
	spawned = 0;
	refused = 0;
	frames = 0;
	idle = 0;
	k = 0;
	while ( spawned < n && idle < 40 )
	{
		before = spawned;
		for ( s = 0; s < spawners.size && spawned < n; s++ )
		{
			sp = spawners[ s ];
			node = nodes[ k % nodes.size ];
			k++;
			old = sp.origin;
			sp.origin = node.origin + ( RandomIntRange( -40, 40 ), RandomIntRange( -40, 40 ), 0 );
			sp.script_forcespawn = 1;
			sp.script_moveoverride = true;
			sp.script_string = "zombie_chaser";
			sp.count = 9999;
			ai = sp StalingradSpawn();
			sp.origin = old;
			if ( IsDefined( ai ) )
			{
				spawned++;
			}
			else
			{
				refused++;
			}
		}
		frames++;
		if ( spawned == before )
		{
			idle++;
		}
		else
		{
			idle = 0;
		}
		wait( 0.05 );
	}
	LogPrint( "stress: spawned " + spawned + " of " + n + " in " + ( GetTime() - start ) + " ms frames " + frames + " refused " + refused + " alive " + GetAiSpeciesArray( "axis", "all" ).size + "\n" );
	for ( ;; )
	{
		LogPrint( "stress: alive " + GetAiSpeciesArray( "axis", "all" ).size + " t " + GetTime() + "\n" );
		wait( 5 );
	}
}

mapkit_name( i )
{
	name = bo1_mapkit_get( i, "name" );
	if ( !IsDefined( name ) )
	{
		name = "edit" + i;
	}
	return name;
}

mapkit_center( i )
{
	o = bo1_mapkit_get( i, "origin" );
	if ( IsDefined( o ) )
	{
		return o;
	}
	mins = bo1_mapkit_get( i, "mins" );
	maxs = bo1_mapkit_get( i, "maxs" );
	return ( mins + maxs ) * 0.5;
}

mapkit_floor_z( o )
{
	t = BulletTrace( ( o[0], o[1], o[2] + 48 ), ( o[0], o[1], o[2] - 256 ), false, undefined );
	return t["position"][2];
}

// the nearest wall around the anchor (8 directions at knee height), backed off by depth; undefined if none.
// mapkit-fix-4b: the engine resolves the same rule at map load for machine edits (edit i) and puts the machine's clip
// there (cm_mapkit.cpp CM_Mapkit_AddMachineClips); its spot wins so the machine stands inside its clip
mapkit_wall_spot( anchor, depth, i )
{
	if ( IsDefined( i ) && IsDefined( bo1_mapkit_get( i, "_spot" ) ) )
	{
		s = SpawnStruct();
		s.origin = bo1_mapkit_get( i, "_spot" );
		s.yaw = bo1_mapkit_get( i, "_spotyaw" );
		s.out = AnglesToForward( ( 0, s.yaw, 0 ) );
		return s;
	}
	start = ( anchor[0], anchor[1], mapkit_floor_z( anchor ) + 40 );
	best = undefined;
	best_d = 99999;
	best_dir = undefined;
	for ( a = 0; a < 360; a += 45 )
	{
		dir = AnglesToForward( ( 0, a, 0 ) );
		t = BulletTrace( start, start + dir * 200, false, undefined );
		if ( t["fraction"] < 1 && t["fraction"] * 200 < best_d )
		{
			best_d = t["fraction"] * 200;
			best = t["position"];
			best_dir = dir;
		}
	}
	if ( !IsDefined( best ) )
	{
		return undefined;
	}
	s = SpawnStruct();
	s.out = ( 0, 0, 0 ) - best_dir;
	o = best + s.out * depth;
	s.origin = ( o[0], o[1], mapkit_floor_z( o ) );
	s.yaw = VectorToAngles( s.out )[1];
	return s;
}

mapkit_perk_model( perk )
{
	switch ( perk )
	{
	case "specialty_armorvest": return "zombie_vending_jugg";
	case "specialty_quickrevive": return "zombie_vending_revive";
	case "specialty_fastreload": return "zombie_vending_sleight";
	case "specialty_rof": return "zombie_vending_doubletap";
	}
	return undefined;
}

mapkit_perk_targetname( perk )
{
	switch ( perk )
	{
	case "specialty_armorvest": return "vending_jugg";
	case "specialty_quickrevive": return "vending_revive";
	case "specialty_fastreload": return "vending_sleight";
	case "specialty_rof": return "vending_doubletap";
	}
	return undefined;
}

// the pattern of _zombiemode_perks::place_additionalprimaryweapon_machine: a script_model + a trigger_radius_use
// with targetname zombie_vending. It runs in main_end, after _zombiemode_perks::init collected the map's machines,
// so the trigger gets init's retail vending_trigger_think thread here; the machine swaps to its _on model on power.
mapkit_place_perk( i )
{
	perk = bo1_mapkit_get( i, "perk" );
	origin = bo1_mapkit_get( i, "origin" );
	model = mapkit_perk_model( perk );
	if ( !IsDefined( model ) || !IsDefined( origin ) )
	{
		LogPrint( "mapkit: perk edit " + i + " skipped (perk " + perk + " has no Five machine model, or no origin)\n" );
		return;
	}
	yaw = bo1_mapkit_get( i, "yaw" );
	if ( !IsDefined( yaw ) )
	{
		yaw = 0;
	}
	out = AnglesToForward( ( 0, yaw, 0 ) );
	snap = bo1_mapkit_get( i, "snap" );
	if ( IsDefined( snap ) && snap == "wall" )
	{
		s = mapkit_wall_spot( origin, 20, i );
		if ( !IsDefined( s ) )
		{
			LogPrint( "mapkit: perk edit " + i + " (" + perk + "): no wall within 200 of " + origin + ", placed as given\n" );
		}
		else
		{
			origin = s.origin;
			yaw = s.yaw;
			out = s.out;
		}
	}
	machine = Spawn( "script_model", origin );
	machine.angles = ( 0, yaw + 90, 0 );
	machine SetModel( model );
	machine.targetname = mapkit_perk_targetname( perk );

	// vending_trigger_think sets UseTriggerRequireLookAt: the engine (Player_GetUseList, player_use_mp.cpp) takes the
	// trigger only when the eye -> bounds-centre direction is within dot 0.76 of the view. The zombies trigger_radius_use
	// spawned by script has its bounds centred on its origin (measured, run mk13: centre z = origin z). Put the centre on
	// the machine's front face at mid height, where a player at the machine looks (runs mk7a/mk8/mk10: a trigger 40 u
	// in front of the machine, or at origin + 30 like retail Mule Kick, left the centre outside the view cone)
	trig = Spawn( "trigger_radius_use", origin + out * 12 + ( 0, 0, 48 ), 0, 44, 70 );
	// the use list's sight trace (Player_GetUseList, eye -> trigger bounds centre) ends inside the solid machine model;
	// the trigger's ignore entity (s.otherEntityNum) makes that trace skip the machine
	trig SetIgnoreEntForTrigger( machine );
	trig.targetname = "zombie_vending";
	trig.target = machine.targetname;
	trig.script_noteworthy = perk;
	trig thread maps\_zombiemode_perks::vending_trigger_think();
	machine thread mapkit_perk_power( model, perk );
	trig thread mapkit_perk_trigger_log( perk );
	rec = SpawnStruct();
	rec.perk = perk;
	rec.edit = i;
	rec.origin = origin;
	rec.out = out;
	rec.uses = 0;
	trig.mapkit_rec = rec;
	if ( !IsDefined( level.mapkit_perks ) )
	{
		level.mapkit_perks = [];
	}
	level.mapkit_perks[ level.mapkit_perks.size ] = rec;
	LogPrint( "mapkit: perk " + perk + " at " + origin + " yaw " + yaw + "\n" );
	bo1_mapkit_mark( "perk", perk, origin, 20 );
	bo1_mapkit_mark( "trigger", perk, trig.origin, 44 );
}

mapkit_perk_power( model, perk )
{
	flag_wait( "power_on" );
	self SetModel( model + "_on" );
	// Retail vending_trigger_think waits on the perk-specific notify, not the power_on flag.
	// Quick Revive first waits for all_players_connected; let every placed trigger reach its power wait.
	flag_wait( "all_players_connected" );
	wait( 0.05 );
	level notify( perk + "_power_on" );
}

mapkit_has_kind( kind )
{
	for ( i = 0; i < bo1_mapkit_editcount(); i++ )
	{
		if ( bo1_mapkit_get( i, "kind" ) == kind )
		{
			return true;
		}
	}
	return false;
}

// mapkit-fix-3: assets of the power switch and Pack-a-Punch kinds (main_start runs before _zombiemode::main, while
// precaching is open). Retail _zombiemode_perks::init precaches the machines' assets only when the map has zombie_vending
// triggers at init (it returns early at "vending_triggers.size < 1"), which the empty base never has, so the kit calls
// the same retail default_vending_precaching (knuckle crack, packapunch_on model, packapunch_fx) itself.
mapkit_precache()
{
	if ( !IsDefined( level.mapkit_empty ) )
	{
		return;
	}
	if ( mapkit_has_kind( "powerswitch" ) )
	{
		PrecacheModel( "zombie_power_lever_short" );
		PrecacheModel( "zombie_power_lever_handle" );
	}
	if ( mapkit_has_kind( "pap" ) )
	{
		PrecacheModel( "zombie_vending_packapunch" );
		maps\_zombiemode_perks::default_vending_precaching();
	}
}

// origin / yaw / out of a machine-like edit by the perk rules: yaw defaults to 0, snap:"wall" backs it off the nearest wall
mapkit_edit_spot( i, depth )
{
	s = SpawnStruct();
	s.origin = bo1_mapkit_get( i, "origin" );
	s.yaw = bo1_mapkit_get( i, "yaw" );
	if ( !IsDefined( s.yaw ) )
	{
		s.yaw = 0;
	}
	s.out = AnglesToForward( ( 0, s.yaw, 0 ) );
	snap = bo1_mapkit_get( i, "snap" );
	if ( IsDefined( snap ) && snap == "wall" )
	{
		w = mapkit_wall_spot( s.origin, depth, i );
		if ( IsDefined( w ) )
		{
			return w;
		}
		LogPrint( "mapkit: " + bo1_mapkit_get( i, "kind" ) + " edit " + i + ": no wall within 200 of " + s.origin + ", placed as given\n" );
	}
	return s;
}

// a use trigger like the perks' (see mapkit_place_perk): centred on the machine's front face at mid height
mapkit_front_trigger( spot, machine )
{
	trig = Spawn( "trigger_radius_use", spot.origin + spot.out * 12 + ( 0, 0, 48 ), 0, 44, 70 );
	trig SetIgnoreEntForTrigger( machine );
	return trig;
}

mapkit_machine_rec( i, spot, trig )
{
	rec = SpawnStruct();
	rec.edit = i;
	rec.name = mapkit_name( i );
	rec.origin = spot.origin;
	rec.out = spot.out;
	rec.trig = trig;
	return rec;
}

// mapkit-fix-3: powerswitch { name, origin, yaw, snap:"wall" } - Five's power switch, maps\zombie_pentagon electric_switch
// (P:334: trigger use_elec_switch, hint ZOMBIE_ELECTRIC_SWITCH, delete on use, flag_set power_on) and wait_for_power
// (P:354: lever elec_switch rotateroll -90 in 0.3 s + zmb_switch_flip, the perk and Pack_A_Punch_on notifies, sparks and
// zmb_turn_on on rotatedone, the power-on announcer). Five-only parts are left out, the empty base has none of them:
// exploders 3500/2000/2001, regular_portal_fx_on, change_pentagon_vision, clientnotify ZPO, teleporter_init,
// Objective_State. Models from the catalog: zombie_power_lever_short (panel) + zombie_power_lever_handle (lever).
mapkit_place_powerswitch( i )
{
	if ( IsDefined( level.mapkit_switch ) )
	{
		LogPrint( "mapkit: powerswitch edit " + i + " skipped (one power switch per layout)\n" );
		return;
	}
	spot = mapkit_edit_spot( i, 4 );
	if ( !IsDefined( spot.origin ) )
	{
		LogPrint( "mapkit: powerswitch edit " + i + " skipped (no origin)\n" );
		return;
	}
	at = spot.origin + ( 0, 0, 48 );
	panel = Spawn( "script_model", at );
	panel.angles = ( 0, spot.yaw + 90, 0 );
	panel SetModel( "zombie_power_lever_short" );
	lever = Spawn( "script_model", at );
	// mapkit-fix-3c: retail Five's handle stands at roll 90 before the flip (entities.txt:7595 elec_switch angles "0 90 90",
	// wall facing yaw 0, so yaw + 90 like the panel); roll 0 hid it in the wall (1x photo sluice_lever_wide.jpg).
	lever.angles = ( 0, spot.yaw + 90, 90 );
	lever SetModel( "zombie_power_lever_handle" );
	lever.targetname = "elec_switch";
	trig = mapkit_front_trigger( spot, panel );
	trig.targetname = "use_elec_switch";
	level.mapkit_switch = mapkit_machine_rec( i, spot, trig );
	trig thread mapkit_electric_switch();
	level thread mapkit_wait_for_power( lever );
	LogPrint( "mapkit: powerswitch " + level.mapkit_switch.name + " at " + spot.origin + " yaw " + spot.yaw + "\n" );
	bo1_mapkit_mark( "powerswitch", level.mapkit_switch.name, spot.origin, 20 );
	bo1_mapkit_mark( "trigger", level.mapkit_switch.name, trig.origin, 44 );
}

// maps\zombie_pentagon electric_switch (SP script P:334)
mapkit_electric_switch()
{
	self SetHintString( &"ZOMBIE_ELECTRIC_SWITCH" );
	self SetCursorHint( "HINT_NOICON" );
	self waittill( "trigger", user );
	LogPrint( "mapkit: powerswitch " + level.mapkit_switch.name + " used by " + user.origin + "\n" );
	self Delete();
	flag_set( "power_on" );
}

// maps\zombie_pentagon wait_for_power (SP script P:354), Five-only parts left out (see mapkit_place_powerswitch)
mapkit_wait_for_power( master_switch )
{
	master_switch NotSolid();
	flag_wait( "power_on" );
	master_switch RotateRoll( -90, .3 );
	master_switch PlaySound( "zmb_switch_flip" );
	level notify( "revive_on" );
	level notify( "juggernog_on" );
	level notify( "sleight_on" );
	level notify( "doubletap_on" );
	level notify( "Pack_A_Punch_on" );
	LogPrint( "mapkit: power on at " + GetTime() + "\n" );
	master_switch waittill( "rotatedone" );
	if ( IsDefined( level._effect[ "switch_sparks" ] ) )
	{
		PlayFx( level._effect[ "switch_sparks" ], master_switch.origin );
	}
	master_switch PlaySound( "zmb_turn_on" );
	level thread maps\zombie_pentagon_amb::play_pentagon_announcer_vox( "zmb_vox_pentann_poweron" );
}

// mapkit-fix-3: pap { name, origin, yaw, snap:"wall" } - the retail Pack-a-Punch chain of _zombiemode_perks
// vending_weapon_upgrade (K:393): trigger zombie_vending_upgrade -> its target, the machine (unique targetname, as the
// retail note at turn_PackAPunch_on asks). Retail init threads vending_weapon_upgrade only when the map has zombie_vending
// triggers at init (K:30 returns early on the empty base), so the kit threads it here, like the perks; turn_PackAPunch_on
// (K:709) swaps the _on model when Pack_A_Punch_on comes (the power switch, or mapkit_pap_power without one).
// The "please wait" sign (machine.target -> wait_flag) is optional in retail and left out: no tag to hang it on is known.
mapkit_place_pap( i )
{
	spot = mapkit_edit_spot( i, 20 );
	if ( !IsDefined( spot.origin ) )
	{
		LogPrint( "mapkit: pap edit " + i + " skipped (no origin)\n" );
		return;
	}
	machine = Spawn( "script_model", spot.origin );
	machine.angles = ( 0, spot.yaw + 90, 0 );
	machine SetModel( "zombie_vending_packapunch" );
	machine.targetname = "mkpap" + i;
	trig = mapkit_front_trigger( spot, machine );
	trig.targetname = "zombie_vending_upgrade";
	trig.target = machine.targetname;
	if ( !IsDefined( level.mapkit_paps ) )
	{
		level.mapkit_paps = [];
		level thread maps\_zombiemode_perks::turn_PackAPunch_on();
	}
	trig thread maps\_zombiemode_perks::vending_weapon_upgrade();
	level.mapkit_paps[ level.mapkit_paps.size ] = mapkit_machine_rec( i, spot, trig );
	LogPrint( "mapkit: pap " + mapkit_name( i ) + " at " + spot.origin + " yaw " + spot.yaw + "\n" );
	bo1_mapkit_mark( "pap", mapkit_name( i ), spot.origin, 20 );
	bo1_mapkit_mark( "trigger", mapkit_name( i ), trig.origin, 44 );
}

// no power switch in the layout: Pack-a-Punch is powered from the start, like the perks (mapkit_perk_power)
mapkit_pap_power()
{
	flag_wait( "power_on" );
	flag_wait( "all_players_connected" );
	wait( 0.05 );
	level notify( "Pack_A_Punch_on" );
}

// mapkit-fix-3: one log line per round, so map makers read rounds from the log instead of spawn gaps.
// Retail round_think (_zombiemode.gsc) notifies start_of_round with level.round_number already the new round.
mapkit_round_log()
{
	for ( ;; )
	{
		level waittill( "start_of_round" );
		LogPrint( "mapkit: round " + level.round_number + " start at " + GetTime() + "\n" );
	}
}

// mapkit-fix-3 self-test of a powerswitch layout: with the power off a perk refuses (retail vending_trigger_think waits
// for <perk>_power_on; the press must reach the trigger), then the test client flips the switch like a player and
// power_on must follow. The perk checks after this one then buy the same perk with the power on.
mapkit_check_powerswitch( player )
{
	if ( !IsDefined( level.mapkit_switch ) )
	{
		return;
	}
	sw = level.mapkit_switch;
	refused = "no perk in the layout to try";
	// mapkit-fix-3c: solo Quick Revive is sold without power in retail (vending_trigger_think skips the _power_on wait when
	// one player is connected; retail research notes/weapons.md:304), so it cannot prove the power is off: try another perk.
	rec = undefined;
	if ( IsDefined( level.mapkit_perks ) )
	{
		for ( p = 0; p < level.mapkit_perks.size && !IsDefined( rec ); p++ )
		{
			if ( level.mapkit_perks[p].perk != "specialty_quickrevive" || get_players().size > 1 )
				rec = level.mapkit_perks[p];
		}
	}
	if ( IsDefined( rec ) )
	{
		mapkit_test_funds( player );
		mapkit_test_visit( rec.origin + rec.out * 40, rec.origin + rec.out * 12 + ( 0, 0, 44 ) );
		start = GetTime();
		while ( rec.uses < 3 && GetTime() - start < 25000 )
		{
			wait( 0.5 );
		}
		wait( 1 );
		if ( player HasPerk( rec.perk ) || flag( "power_on" ) )
		{
			LogPrint( "mapkit: check powerswitch " + sw.name + " FAIL perk " + rec.perk + " was sold with the power off (power_on " + flag( "power_on" ) + ")\n" );
			return;
		}
		if ( rec.uses == 0 )
		{
			LogPrint( "mapkit: check powerswitch " + sw.name + " FAIL the press never reached perk " + rec.perk + " (player at " + player.origin + ")\n" );
			return;
		}
		refused = "perk " + rec.perk + " refused " + rec.uses + " uses with the power off";
	}
	mapkit_test_visit( sw.origin + sw.out * 40, sw.origin + sw.out * 12 + ( 0, 0, 44 ) );
	start = GetTime();
	while ( !flag( "power_on" ) && GetTime() - start < 25000 )
	{
		wait( 0.5 );
	}
	SetDvar( "bo1_testclient_press", "" );
	if ( flag( "power_on" ) )
	{
		LogPrint( "mapkit: check powerswitch " + sw.name + " PASS " + refused + "; power_on " + ( GetTime() - start ) + " ms after the flip\n" );
	}
	else
	{
		LogPrint( "mapkit: check powerswitch " + sw.name + " FAIL switch never used (player at " + player.origin + ", switch front " + ( sw.origin + sw.out * 40 ) + ")\n" );
	}
}

// mapkit-fix-3 self-test of each light edit (mood lighting, empty base only): the engine resolves the area and its tint
// (bo1_mapkit_light at the area center, 1 1 1 = the kit's flat grey) and the renderer lit something with it
// (bo1_mapkit_lightuses = model/box lighting samples inside the area that took the tint).
mapkit_check_lights()
{
	for ( i = 0; i < bo1_mapkit_editcount(); i++ )
	{
		if ( bo1_mapkit_get( i, "kind" ) != "light" )
		{
			continue;
		}
		name = mapkit_name( i );
		if ( !IsDefined( level.mapkit_empty ) )
		{
			LogPrint( "mapkit: check light " + name + " SKIP (lights tint the empty base only; an overlay keeps the retail light grid)\n" );
			continue;
		}
		area = i;
		room = bo1_mapkit_get( i, "room" );
		for ( j = 0; IsDefined( room ) && j < bo1_mapkit_editcount(); j++ )
		{
			if ( j != i && mapkit_name( j ) == room && IsDefined( bo1_mapkit_get( j, "mins" ) ) )
			{
				area = j;
			}
		}
		if ( !IsDefined( bo1_mapkit_get( area, "mins" ) ) )
		{
			LogPrint( "mapkit: check light " + name + " FAIL no area (mins/maxs, or room naming an edit with bounds)\n" );
			continue;
		}
		c = mapkit_center( area );
		tint = bo1_mapkit_light( c );
		uses = bo1_mapkit_lightuses( i );
		if ( !IsDefined( tint ) || uses < 0 )
		{
			LogPrint( "mapkit: check light " + name + " FAIL the engine has no light at the area center " + c + "\n" );
		}
		else if ( uses == 0 )
		{
			LogPrint( "mapkit: check light " + name + " FAIL tint " + tint + " but no renderer sample inside the area took it\n" );
		}
		else
		{
			LogPrint( "mapkit: check light " + name + " PASS tint " + tint + " at " + c + ", " + uses + " renderer samples\n" );
		}
	}
}

// mapkit-fix-3 self-test of each pap edit: funded, the test client presses at the machine with its current gun, the
// retail machine upgrades it (knuckle crack, gun in the machine) and the next press takes it; the player must then hold
// level.zombie_weapons[gun].upgrade_name (the _upgraded_zm name from add_zombie_weapon).
mapkit_check_pap( player )
{
	for ( k = 0; IsDefined( level.mapkit_paps ) && k < level.mapkit_paps.size; k++ )
	{
		rec = level.mapkit_paps[k];
		if ( GetDvar( "mapkit_doctor" ) == "1" )
		{
			LogPrint( "mapkit_doctor purchase " + rec.edit + "\n" );
		}
		mapkit_test_funds( player );
		// the wall-buy check just before hands the player a new gun; read the gun once the switch to it is over
		// (power_example run 1: read m1911_zm mid-switch, the machine then upgraded the m14)
		wait( 1 );
		for ( w = 0; w < 20 && player IsSwitchingWeapons(); w++ )
		{
			wait( 0.25 );
		}
		gun = player GetCurrentWeapon();
		if ( !IsDefined( level.zombie_weapons[ gun ] ) || !IsDefined( level.zombie_weapons[ gun ].upgrade_name ) )
		{
			LogPrint( "mapkit: check pap " + rec.name + " FAIL the player's gun " + gun + " has no upgrade in the weapon table\n" );
			continue;
		}
		up = level.zombie_weapons[ gun ].upgrade_name;
		score = player.score;
		mapkit_test_visit( rec.origin + rec.out * 40, rec.origin + rec.out * 12 + ( 0, 0, 44 ) );
		start = GetTime();
		while ( !player HasWeapon( up ) && GetTime() - start < 40000 )
		{
			wait( 0.5 );
		}
		SetDvar( "bo1_testclient_press", "" );
		if ( player HasWeapon( up ) )
		{
			LogPrint( "mapkit: check pap " + rec.name + " PASS " + gun + " -> " + up + " after " + ( GetTime() - start ) + " ms, paid " + ( score - player.score ) + "\n" );
		}
		else
		{
			LogPrint( "mapkit: check pap " + rec.name + " FAIL no " + up + " within 40 s (holding " + player GetCurrentWeapon() + ", score " + player.score + " of " + score
				+ ", machine in use " + flag( "pack_machine_in_use" ) + ")\n" );
		}
	}
}

// test aid: bullet traces across every room's walls (they must hit), the player's origin every 2 s, and the dump
mapkit_selftest()
{
	flag_wait( "all_players_connected" );
	wait 2;
	for ( i = 0; i < bo1_mapkit_editcount(); i++ )
	{
		if ( bo1_mapkit_get( i, "kind" ) != "room" )
		{
			continue;
		}
		mins = bo1_mapkit_get( i, "mins" );
		maxs = bo1_mapkit_get( i, "maxs" );
		c = ( mins + maxs ) * 0.5;
		dirs = array( ( 1, 0, 0 ), ( -1, 0, 0 ), ( 0, 1, 0 ), ( 0, -1, 0 ), ( 0, 0, 1 ), ( 0, 0, -1 ) );
		for ( d = 0; d < dirs.size; d++ )
		{
			t = BulletTrace( c, c + dirs[d] * 400, false, undefined );
			LogPrint( "mapkit: selftest room " + mapkit_name( i ) + " dir " + dirs[d] + " hit " + t["position"] + " frac " + t["fraction"] + "\n" );
		}
	}
	bo1_mapkit_dump( "mapkit_dump.json" );
	players = get_players();
	for ( n = 0; n < 60; n++ )
	{
		for ( p = 0; p < players.size; p++ )
		{
			LogPrint( "mapkit: player " + p + " origin " + players[p].origin + "\n" );
		}
		wait 2;
	}
}

// window { spawn, target, every, room }: zombies of the running round come in from "spawn" (outside the window) while
// the round still has zombies to spawn. Reuses a live zone spawner, moved for one spawn and put back in the same frame
// (the horde napalm module's pattern); "target" = the targetname of a node next to the window, which the retail entrance
// logic uses to pick that window (_zombiemode_spawner: closest exterior goal to the spawner's target). Each spawn takes
// one from level.zombie_total, so the round's count is unchanged.
mapkit_window( i )
{
	name = mapkit_name( i );
	spawnOrigin = bo1_mapkit_get( i, "spawn" );
	target = bo1_mapkit_get( i, "target" );
	every = bo1_mapkit_get( i, "every" );
	if ( !IsDefined( every ) )
	{
		every = 4;
	}
	bo1_mapkit_mark( "spawner", name, spawnOrigin, 16 );
	flag_wait( "begin_spawning" );
	spawned = 0;
	while ( 1 )
	{
		wait( every );
		if ( !IsDefined( level.zombie_total ) || level.zombie_total <= 0 || get_enemy_count() >= level.zombie_ai_limit )
		{
			continue;
		}
		if ( !IsDefined( level.enemy_spawns ) || level.enemy_spawns.size == 0 )
		{
			continue;
		}
		spawner = level.enemy_spawns[ RandomInt( level.enemy_spawns.size ) ];
		oldOrigin = spawner.origin;
		oldTarget = spawner.target;
		oldForce = spawner.script_forcespawn;
		spawner.origin = spawnOrigin;
		spawner.target = target;
		spawner.script_forcespawn = 1;
		ai = spawn_zombie( spawner );
		spawner.origin = oldOrigin;
		spawner.target = oldTarget;
		spawner.script_forcespawn = oldForce;
		if ( spawn_failed( ai ) )
		{
			LogPrint( "mapkit: window " + name + " spawn failed\n" );
			continue;
		}
		ai ForceTeleport( spawnOrigin, ( 0, 270, 0 ) );
		// mapkit-fix-3d: the doctor attributes actor lines by source window (it printed -1: window-no-reach FAIL)
		ai.mapkit_source_window = i;
		level.zombie_total--;
		spawned++;
		LogPrint( "mapkit: window " + name + " spawned #" + spawned + " round " + level.round_number + " at " + spawnOrigin + " target " + target + "\n" );
		ai thread mapkit_window_track( i, name, spawned );
	}
}

// test log: when a window zombie gets inside the edit's "room" (inner mins/maxs of that room edit) and when it gets
// within 64 of a player
mapkit_window_track( i, name, id )
{
	self endon( "death" );
	room = bo1_mapkit_get( i, "room" );
	if ( !IsDefined( room ) && IsDefined( level.mapkit_empty ) )
	{
		room = mapkit_window_zone( i );
	}
	mins = undefined;
	maxs = undefined;
	for ( e = 0; IsDefined( room ) && e < bo1_mapkit_editcount(); e++ )
	{
		if ( mapkit_name( e ) == room )
		{
			mins = bo1_mapkit_get( e, "mins" );
			maxs = bo1_mapkit_get( e, "maxs" );
		}
	}
	start = GetTime();
	inside = false;
	if ( !IsDefined( level.mapkit_window_state ) )
	{
		level.mapkit_window_state = [];
	}
	while ( 1 )
	{
		wait( 0.25 );
		o = self.origin;
		if ( !inside && IsDefined( mins ) && o[0] > mins[0] && o[0] < maxs[0] && o[1] > mins[1] && o[1] < maxs[1] && o[2] > mins[2] - 8 && o[2] < maxs[2] )
		{
			inside = true;
			if ( !IsDefined( level.mapkit_window_state[ name ] ) )
			{
				level.mapkit_window_state[ name ] = "inside room " + room;
			}
			LogPrint( "mapkit: window " + name + " zombie #" + id + " inside room " + room + " at " + o + " after " + ( GetTime() - start ) + " ms\n" );
		}
		players = get_players();
		for ( p = 0; p < players.size; p++ )
		{
			if ( ( !IsDefined( level.mapkit_empty ) || inside ) && DistanceSquared( o, players[p].origin ) < 64 * 64 )
			{
				level.mapkit_window_state[ name ] = "reached";
				// Record the exact witness: the 0.5 s doctor sampler can miss a 0.25 s reach.
				if ( GetDvar( "mapkit_doctor" ) == "1" && IsDefined( self.mapkit_doctor_serial ) )
				{
					entnum = self GetEntityNumber();
					LogPrint( "mapkit_doctor reach " + GetTime() + " " + self.mapkit_doctor_serial + " " + entnum + " " + i + " " + o + "\n" );
				}
				LogPrint( "mapkit: window " + name + " zombie #" + id + " reached player " + p + " at " + o + " after " + ( GetTime() - start ) + " ms\n" );
				return;
			}
		}
	}
}

// self-test (mapkit_selftest 1 + bo1_testclient 1 + bo1_testclient_passive 1): checks every placement for use and
// prints one line per item, "mapkit: check <kind> <name> PASS|FAIL|SKIP <why>". Perks: power on + points, the test
// client walks to the machine front, looks at it and presses use; PASS when HasPerk. Windows: PASS when a zombie that
// came in through it reached a player (the player waits in the last perk's room). Kinds with no runtime yet: SKIP.
mapkit_check()
{
	mapkit_test_budget();
	flag_wait( "all_players_spawned" );
	wait( 3 );
	player = get_players()[0];
	if ( GetDvar( "bo1_testclient" ) != "1" )
	{
		LogPrint( "mapkit: check all SKIP (needs bo1_testclient 1 + bo1_testclient_passive 1)\n" );
		return;
	}
	// mapkit-fix-3c: the switch and the perk that must refuse can sit behind doors (sluice_works: switch in the control
	// room, the only reachable perk solo Quick Revive; live doctor "the press never reached perk"). Without electric doors
	// the progression opens everything with the power off first; electric doors need the power, so they keep the old order.
	powerDoors = false;
	doors = GetEntArray( "zombie_door", "targetname" );
	for ( d = 0; d < doors.size; d++ )
	{
		if ( IsDefined( doors[d].script_noteworthy ) && ( doors[d].script_noteworthy == "electric_buyable_door" || doors[d].script_noteworthy == "electric_door" ) )
			powerDoors = true;
	}
	if ( powerDoors )
		mapkit_check_powerswitch( player );
	else
		LogPrint( "mapkit: selftest powerswitch check after the progression (no electric doors)\n" );
	if ( powerDoors )
		mapkit_test_power( player );
	level.mapkit_test_purchases = true;
	player thread mapkit_test_purchase_defence();
	mapkit_check_progression( player );
	if ( !powerDoors )
	{
		mapkit_check_powerswitch( player );
		mapkit_test_power( player );
	}
	// test aid (mapkit_test_powerup <name>, e.g. minigun): drop a retail powerup on the player before the perk checks,
	// so the Death Machine case of mapkit_test_hands_free can be reproduced on purpose
	if ( GetDvar( "mapkit_test_powerup" ) != "" )
	{
		LogPrint( "mapkit: test powerup " + GetDvar( "mapkit_test_powerup" ) + " dropped at " + player.origin + "\n" );
		level thread maps\_zombiemode_powerups::specific_powerup_drop( GetDvar( "mapkit_test_powerup" ), player.origin );
		wait( 3 );
	}
	for ( k = 0; IsDefined( level.mapkit_perks ) && k < level.mapkit_perks.size; k++ )
	{
		rec = level.mapkit_perks[k];
		if ( GetDvar( "mapkit_doctor" ) == "1" )
		{
			LogPrint( "mapkit_doctor purchase " + rec.edit + "\n" );
		}
		mapkit_test_funds( player );
		stand = rec.origin + rec.out * 40;
		look = rec.origin + rec.out * 12 + ( 0, 0, 44 );
		SetDvar( "bo1_testclient_goto", stand[0] + "," + stand[1] + "," + stand[2] );
		SetDvar( "bo1_testclient_look", look[0] + "," + look[1] + "," + look[2] );
		SetDvar( "bo1_testclient_press", "3," + GetTime() + ",1500,200" );
		start = GetTime();
		while ( !player HasPerk( rec.perk ) && GetTime() - start < 25000 )
		{
			wait( 0.5 );
		}
		if ( player HasPerk( rec.perk ) )
		{
			LogPrint( "mapkit: check perk " + rec.perk + " PASS bought after " + ( GetTime() - start ) + " ms at " + rec.origin + "\n" );
		}
		else
		{
			why = "trigger never used (player at " + player.origin + ", machine front " + stand + ")";
			if ( rec.uses > 0 )
			{
				why = "trigger used " + rec.uses + "x but no perk (score " + player.score + ")";
			}
			LogPrint( "mapkit: check perk " + rec.perk + " FAIL " + why + "\n" );
		}
	}
	mapkit_check_zones( player );
	mapkit_check_wallbuys( player );
	mapkit_check_pap( player );
	mapkit_check_lights();
	level.mapkit_test_purchases = false;
	SetDvar( "bo1_testclient_press", "" );
	for ( i = 0; i < bo1_mapkit_editcount(); i++ )
	{
		kind = bo1_mapkit_get( i, "kind" );
		if ( kind != "window" && kind != "perk" && kind != "room" && kind != "box" && kind != "stairs" && kind != "node" && kind != "zone"
			&& kind != "wallbuy" && kind != "spawner" && kind != "spawn" && kind != "door" && kind != "debris" && kind != "mysterybox"
			&& kind != "pap" && kind != "powerswitch" && kind != "light" && kind != "riser" )
		{
			LogPrint( "mapkit: check " + kind + " " + mapkit_name( i ) + " SKIP (no runtime check for this kind yet)\n" );
		}
	}
	for ( i = 0; i < bo1_mapkit_editcount(); i++ )
	{
		if ( bo1_mapkit_get( i, "kind" ) != "window" )
		{
			continue;
		}
		name = mapkit_name( i );
		// Visit each empty-base entrance's room; the retail zone manager activates its spawners by occupancy.
		wo = bo1_mapkit_get( i, "origin" );
		if ( IsDefined( wo ) )
		{
			if ( IsDefined( level.mapkit_empty ) )
			{
				yaw = bo1_mapkit_get( i, "yaw" );
				if ( !IsDefined( yaw ) )
				{
					yaw = 90;
				}
				stand = wo + AnglesToForward( ( 0, yaw, 0 ) ) * 160;
				mapkit_test_visit( stand, wo + ( 0, 0, 48 ) );
				SetDvar( "bo1_testclient_press", "" );
			}
			SetDvar( "bo1_testclient_look", wo[0] + "," + wo[1] + "," + ( wo[2] + 48 ) );
		}
		start = GetTime();
		while ( GetTime() - start < 90000 && !( IsDefined( level.mapkit_window_state ) && IsDefined( level.mapkit_window_state[ name ] ) && level.mapkit_window_state[ name ] == "reached" ) )
		{
			if ( IsDefined( level.mapkit_empty ) && GetTime() - start > 5000 )
			{
				// A passive client otherwise leaves the round's population stranded at previous windows.
				// Keep this entrance's zombies alive; replacements still come from the retail round spawner.
				zombies = GetAiSpeciesArray( "axis", "all" );
				for ( z = 0; z < zombies.size; z++ )
				{
					if ( IsDefined( zombies[z].mapkit_source_window ) && zombies[z].mapkit_source_window != i )
					{
						mapkit_test_kill( zombies[z], player, "other_window" );
					}
					// mapkit-fix-4e: riser zombies (retail script_string "riser") have no source window and were never
					// culled: two filter_bed risers stood on the god player for the whole 90 s (sluice_works: check window
					// pump_south and control_north FAIL "spawned outside only"). The risers are checked after the windows.
					else if ( !IsDefined( zombies[z].mapkit_source_window ) && IsDefined( zombies[z].script_string ) && zombies[z].script_string == "riser" )
					{
						mapkit_test_kill( zombies[z], player, "riser_during_window" );
					}
				}
			}
			wait( 1 );
		}
		if ( IsDefined( level.mapkit_window_state ) && IsDefined( level.mapkit_window_state[ name ] ) && level.mapkit_window_state[ name ] == "reached" )
		{
			LogPrint( "mapkit: check window " + name + " PASS a zombie from it reached the player\n" );
		}
		else
		{
			got = "no zombie spawned there";
			if ( IsDefined( level.mapkit_window_state ) && IsDefined( level.mapkit_window_state[ name ] ) )
			{
				got = "zombies got " + level.mapkit_window_state[ name ] + " only";
			}
			LogPrint( "mapkit: check window " + name + " FAIL within 90 s: " + got + "\n" );
		}
		if ( IsDefined( level.mapkit_empty ) && mapkit_window_open( i ) )
		{
			LogPrint( "mapkit: window " + name + " is an open entrance (no boards, no rebuild)\n" );
		}
		else if ( IsDefined( level.mapkit_empty ) )
		{
			torn = mapkit_boards_torn( i );
			total = mapkit_boards( i ).size;
			if ( torn > 0 )
			{
				LogPrint( "mapkit: check window " + name + " boards PASS " + torn + "/" + total + " torn by zombies\n" );
			}
			else
			{
				LogPrint( "mapkit: check window " + name + " boards FAIL 0/" + total + " torn (no zombie reached the window)\n" );
			}
		}
	}
	mapkit_check_risers( player );
	// mapkit-fix-3d: overlays too (retail round spawning; the doctor always requires the round line)
	mapkit_check_round( player );
	bo1_mapkit_selftestdone();
	if ( GetDvar( "mapkit_doctor" ) == "1" )
	{
		level.mapkit_doctor_done = true;
		bo1_mapkit_dump( "mapkit_doctor_dump.json" );
		mapkit_doctor_photos( player );
	}
}

// Worst-case waits in mapkit_check, including every destination-window wait after each blocker.
// tools/mapkit/selftest_budget.mjs computes this before launch and adds a real-time startup allowance.
mapkit_test_budget()
{
	seconds = 5;
	windows = 0;
	blockers = 0;
	for ( i = 0; i < bo1_mapkit_editcount(); i++ )
	{
		kind = bo1_mapkit_get( i, "kind" );
		if ( kind == "perk" || kind == "wallbuy" )
		{
			seconds += 25;
		}
		if ( kind == "window" )
		{
			windows++;
		}
		if ( kind == "riser" && IsDefined( level.mapkit_empty ) )
		{
			seconds += 90;
		}
		if ( IsDefined( level.mapkit_empty ) && ( kind == "door" || kind == "debris" ) )
		{
			blockers++;
		}
		if ( IsDefined( level.mapkit_empty ) && kind == "mysterybox" )
		{
			seconds += 30;
		}
	}
	seconds += windows * 90 + blockers * ( 29 + windows * 90 );
	seconds += 120; // mapkit_check_round, empty base and overlay
	LogPrint( "mapkit_selftest BUDGET level_seconds " + seconds + " timescale " + GetDvar( "timescale" ) + "\n" );
}

// every zone: its info_volume exists, the zone manager enabled it, and whether the test client stands in it
mapkit_check_zones( player )
{
	for ( i = 0; i < bo1_mapkit_editcount(); i++ )
	{
		if ( bo1_mapkit_get( i, "kind" ) != "zone" )
		{
			continue;
		}
		name = mapkit_name( i );
		vol = GetEnt( name, "targetname" );
		if ( !IsDefined( vol ) )
		{
			LogPrint( "mapkit: check zone " + name + " FAIL no info_volume (the zone needs mins and maxs)\n" );
			continue;
		}
		inside = player IsTouching( vol );
		if ( IsDefined( level.zones ) && IsDefined( level.zones[ name ] ) && level.zones[ name ].is_enabled )
		{
			LogPrint( "mapkit: check zone " + name + " PASS enabled, test client inside " + inside + "\n" );
		}
		else
		{
			LogPrint( "mapkit: check zone " + name + " FAIL not enabled by the zone manager (test client inside " + inside + ")\n" );
		}
	}
}

// every wall buy: the test client walks in front of it, looks at the weapon, presses use and must get the weapon
mapkit_check_wallbuys( player )
{
	for ( i = 0; i < bo1_mapkit_editcount(); i++ )
	{
		if ( bo1_mapkit_get( i, "kind" ) != "wallbuy" )
		{
			continue;
		}
		if ( GetDvar( "bo1_testclient_fight" ) != "1" )
		{
			LogPrint( "mapkit: check wallbuy " + mapkit_name( i ) + " SKIP (the test client walks only with bo1_testclient_fight 1)\n" );
			continue;
		}
		o = bo1_mapkit_get( i, "origin" );
		yaw = bo1_mapkit_get( i, "yaw" );
		weapon = bo1_mapkit_get( i, "weapon" );
		if ( GetDvar( "mapkit_doctor" ) == "1" )
		{
			LogPrint( "mapkit_doctor purchase " + i + "\n" );
		}
		if ( !IsDefined( yaw ) )
		{
			yaw = 0;
		}
		if ( !IsDefined( weapon ) )
		{
			weapon = "m14_zm";
		}
		mapkit_test_funds( player );
		// Each placement must sell its weapon, even when multiple wall buys offer the same gun.
		if ( player HasWeapon( weapon ) )
		{
			player TakeWeapon( weapon );
		}
		f = AnglesToForward( ( 0, yaw, 0 ) );
		stand = o + f * 40;
		SetDvar( "bo1_testclient_goto", stand[0] + "," + stand[1] + "," + player.origin[2] );
		SetDvar( "bo1_testclient_look", o[0] + "," + o[1] + "," + o[2] );
		SetDvar( "bo1_testclient_press", "3," + GetTime() + ",1500,200" );
		start = GetTime();
		while ( !player HasWeapon( weapon ) && GetTime() - start < 25000 )
		{
			wait( 0.5 );
		}
		if ( player HasWeapon( weapon ) )
		{
			LogPrint( "mapkit: check wallbuy " + weapon + " PASS bought after " + ( GetTime() - start ) + " ms, score now " + player.score + "\n" );
		}
		else
		{
			LogPrint( "mapkit: check wallbuy " + weapon + " FAIL not bought in 25 s (player at " + player.origin + ", stand " + stand + ", score " + player.score + ")\n" );
		}
		SetDvar( "bo1_testclient_press", "" );
	}
}

// rounds advance: a passive test client never kills, so the check kills every zombie until round 2 starts
mapkit_check_round( player )
{
	start = GetTime();
	first = level.round_number;
	while ( level.round_number == first && GetTime() - start < 120000 )
	{
		zombies = GetAiSpeciesArray( "axis", "all" );
		for ( z = 0; z < zombies.size; z++ )
		{
			mapkit_test_kill( zombies[z], player, "round_cleanup" );
		}
		wait( 1 );
	}
	if ( level.round_number > first )
	{
		LogPrint( "mapkit: check round PASS round " + first + " -> " + level.round_number + " after " + ( GetTime() - start ) + " ms\n" );
	}
	else
	{
		LogPrint( "mapkit: check round FAIL still round " + first + " after 120 s (zombies left " + level.zombie_total + ")\n" );
	}
}

mapkit_boards( i )
{
	boards = [];
	ents = GetEntArray( "mkwin" + i, "targetname" );
	for ( b = 0; b < ents.size; b++ )
	{
		if ( IsDefined( ents[b].script_parameters ) && ents[b].script_parameters == "repair_board" )
		{
			boards[boards.size] = ents[b];
		}
	}
	return boards;
}

mapkit_boards_torn( i )
{
	boards = mapkit_boards( i );
	torn = 0;
	for ( b = 0; b < boards.size; b++ )
	{
		if ( IsDefined( boards[b].mapkit_torn ) )
		{
			torn++;
		}
	}
	return torn;
}

// empty base: the window's zombies come from the retail spawner logic (a spawner edit; the closest exterior_goal).
// Track only the actor's retail exterior_goal, following zombie_assure_node reassignments. A board counts as torn
// once it left its place. Never infer ownership from watcher scheduling or proximity after the zombie moves.
// SpawnActor copies script fields from the spawner (SP 0x00526e50). Label the source before any
// round starts; first_node is unsuitable because retail zombie_assure_node can replace it in flight.
mapkit_label_spawners()
{
	for ( e = 0; e < bo1_mapkit_editcount(); e++ )
	{
		if ( bo1_mapkit_get( e, "kind" ) != "zone" )
		{
			continue;
		}
		spawners = GetEntArray( mapkit_name( e ) + "_spawners", "targetname" );
		for ( s = 0; s < spawners.size; s++ )
		{
			if ( IsDefined( spawners[s].script_string ) && spawners[s].script_string == "riser" )
			{
				continue;
			}
			nearest = undefined;
			best = 999999999;
			for ( w = 0; w < bo1_mapkit_editcount(); w++ )
			{
				if ( bo1_mapkit_get( w, "kind" ) != "window" || mapkit_window_zone( w ) != mapkit_name( e ) )
				{
					continue;
				}
				dist = DistanceSquared( spawners[s].origin, bo1_mapkit_get( w, "origin" ) );
				if ( dist < best )
				{
					best = dist;
					nearest = w;
				}
			}
			if ( IsDefined( nearest ) )
			{
				spawners[s].mapkit_source_window = nearest;
				LogPrint( "mapkit: spawner " + spawners[s].origin + " source window " + mapkit_name( nearest ) + "\n" );
			}
		}
	}
}

mapkit_empty_window_watch( i )
{
	name = mapkit_name( i );
	boards = mapkit_boards( i );
	for ( b = 0; b < boards.size; b++ )
	{
		boards[b].mapkit_start = boards[b].origin;
	}
	while ( 1 )
	{
		for ( b = 0; b < boards.size; b++ )
		{
			if ( !IsDefined( boards[b].mapkit_torn ) && DistanceSquared( boards[b].origin, boards[b].mapkit_start ) > 4 )
			{
				boards[b].mapkit_torn = true;
				LogPrint( "mapkit: window " + name + " board " + boards[b].script_noteworthy + " torn at " + GetTime() + "\n" );
			}
		}
		zombies = GetAiSpeciesArray( "axis", "all" );
		for ( z = 0; z < zombies.size; z++ )
		{
			if ( IsDefined( zombies[z].mapkit_source_window ) && zombies[z].mapkit_source_window == i
				&& !IsDefined( zombies[z].mapkit_tracked ) )
			{
				zombies[z].mapkit_tracked = true;
				id = zombies[z] GetEntityNumber();
				if ( !IsDefined( level.mapkit_window_state ) )
				{
					level.mapkit_window_state = [];
				}
				if ( !IsDefined( level.mapkit_window_state[ name ] ) )
				{
					level.mapkit_window_state[ name ] = "spawned outside";
				}
				LogPrint( "mapkit: window " + name + " actor " + id + " source mkwin" + i + " at " + zombies[z].origin + "\n" );
				zombies[z] thread mapkit_window_track( i, name, id );
				if ( GetDvar( "mapkit_selftest" ) == "1" )
				{
					zombies[z] thread mapkit_window_path_failure( name, id );
				}
			}
		}
		wait( 0.25 );
	}
}

mapkit_window_path_failure( name, id )
{
	self endon( "death" );
	while ( 1 )
	{
		self waittill( "bad_path", goal );
		LogPrint( "mapkit: window " + name + " actor " + id + " bad path from " + self.origin + " to " + goal + "\n" );
		if ( IsDefined( self.first_node ) )
		{
			entrance_target = "none"; // an open entrance's exterior_goal has no target
			if ( IsDefined( self.first_node.target ) )
			{
				entrance_target = self.first_node.target;
			}
			LogPrint( "mapkit: path detail entrance " + self.first_node.origin + " target " + entrance_target + " got " +self.got_to_entrance + " chunks " + self.first_node.barrier_chunks.size + "\n" );
		}
		if ( GetDvar( "mapkit_doctor" ) == "1" )
		{
			LogPrint( "mapkit_doctor path " + GetTime() + " " + id + " " + self.mapkit_source_window + " " + self.origin + " goal " + goal + "\n" );
		}
		else
		{
			return;
		}
	}
}

// Doctor observations never move/kill actors or change the test route. The serial is
// attached to the actor, so recycled entity slots cannot inherit an earlier life.
mapkit_doctor_observe()
{
	flag_wait( "all_players_spawned" );
	serial = 0;
	LogPrint( "mapkit_doctor BEGIN " + level.mapkit_layout + "\n" );
	while ( !IsDefined( level.mapkit_doctor_done ) )
	{
		players = get_players();
		if ( players.size )
		{
			LogPrint( "mapkit_doctor player " + GetTime() + " " + players[0].origin + "\n" );
		}
		zombies = GetAiSpeciesArray( "axis", "all" );
		for ( a = 0; a < zombies.size; a++ )
		{
			z = zombies[a];
			if ( !IsDefined( z.mapkit_doctor_serial ) )
			{
				serial++;
				z.mapkit_doctor_serial = serial;
			}
			source = -1;
			if ( IsDefined( z.mapkit_source_window ) )
			{
				source = z.mapkit_source_window;
			}
			got = 0;
			if ( IsDefined( z.got_to_entrance ) && z.got_to_entrance )
			{
				got = 1;
			}
			tore = -1;
			// Retail zombie_tear_notetracks stamps this only when destroying a chunk.
			if ( IsDefined( z.lastchunk_destroy_time ) )
			{
				tore = z.lastchunk_destroy_time;
			}
			emerged = 0;
			if ( IsDefined( z.completed_emerging_into_playable_area ) && z.completed_emerging_into_playable_area )
			{
				emerged = 1;
			}
			id = z GetEntityNumber();
			target = "none";
			if ( IsDefined( z.first_node ) && IsDefined( z.first_node.target ) )
			{
				target = z.first_node.target;
			}
			// mapkit-fix-4c: a riser under the floor (retail do_zombie_rise starts it 45 below the spot) is not a fall
			ground = "";
			if ( IsDefined( z.in_the_ground ) && z.in_the_ground )
			{
				ground = " ground 1";
			}
			LogPrint( "mapkit_doctor actor " + GetTime() + " " + z.mapkit_doctor_serial + " " + id + " " + source + " " + z.origin + " got " + got + " tore " + tore + " emerged " + emerged + " target " + target + ground + "\n" );
		}
		wait( 0.5 );
	}
}

// One headless Client run: 4x functional checks, then 1x back-buffer evidence.
// Camera placement occurs only AFTER DONE; it cannot make reachability checks pass.
mapkit_doctor_photos( player )
{
	SetDvar( "timescale", "1" );
	SetDvar( "bo1_testclient_press", "" );
	shots = "";
	shot = 0;
	for ( i = 0; i < bo1_mapkit_editcount(); i++ )
	{
		if ( bo1_mapkit_get( i, "kind" ) != "window" )
		{
			continue;
		}
		o = bo1_mapkit_get( i, "origin" );
		if ( !IsDefined( o ) )
		{
			continue;
		}
		yaw = bo1_mapkit_get( i, "yaw" );
		if ( !IsDefined( yaw ) )
		{
			yaw = 90;
		}
		stand = o + AnglesToForward( ( 0, yaw, 0 ) ) * 160;
		player SetOrigin( stand );
		mapkit_test_visit( stand, o + ( 0, 0, 48 ) );
		SetDvar( "bo1_testclient_press", "" );
		wait( 3 );
		shot++;
		shots = shots + shot + " ";
		LogPrint( "mapkit_doctor photo " + i + " " + shot + " " + player.origin + " timescale " + GetDvar( "timescale" ) + "\n" );
		SetDvar( "bo1_shots", shots );
		wait( 2 );
	}
	LogPrint( "mapkit_doctor END\n" );
	SetDvar( "bo1_autoquit", "1" );
}

// mapkit-fix-4a test aid (1x -Client, passive god test client): every window photographed at spawn (boarded), then the
// first window zombies tear two boards off is photographed from the same camera (partly torn), zombies are culled and
// the player holds use there until every board is back (rebuilt; score before/after logged).
mapkit_boardshots_cam( player, i, press )
{
	o = bo1_mapkit_get( i, "origin" );
	yaw = bo1_mapkit_get( i, "yaw" );
	if ( !IsDefined( yaw ) )
	{
		yaw = 90;
	}
	dist = 160;
	if ( press )
	{
		dist = 48;
	}
	stand = o + AnglesToForward( ( 0, yaw, 0 ) ) * dist;
	look = o + ( 0, 0, 48 );
	player SetOrigin( stand );
	SetDvar( "bo1_testclient_goto", stand[0] + "," + stand[1] + "," + stand[2] );
	SetDvar( "bo1_testclient_look", look[0] + "," + look[1] + "," + look[2] );
	if ( press )
	{
		SetDvar( "bo1_testclient_press", "3," + GetTime() + ",1000,1000" );
	}
	else
	{
		SetDvar( "bo1_testclient_press", "" );
	}
}

mapkit_boardshots_shot( i, what )
{
	level.mapkit_shot++;
	level.mapkit_shots = level.mapkit_shots + level.mapkit_shot + " ";
	LogPrint( "mapkit: boardshots shot " + level.mapkit_shot + " window " + i + " " + what + " torn " + mapkit_boards_moved( i ) + "/" + mapkit_boards( i ).size + " panels " + mapkit_panels( i ) + "\n" );
	SetDvar( "bo1_shots", level.mapkit_shots );
	wait( 2 );
}

mapkit_panels( i )
{
	n = 0;
	for ( s = 2116; s <= 2121; s++ )
	{
		n += GetEntArray( "mkwin" + i + "_sec" + s, "targetname" ).size;
	}
	return n;
}

mapkit_boards_moved( i )
{
	boards = mapkit_boards( i );
	n = 0;
	for ( b = 0; b < boards.size; b++ )
	{
		if ( !IsDefined( boards[b].mapkit_start ) )
		{
			boards[b].mapkit_start = boards[b].origin;
		}
		if ( DistanceSquared( boards[b].origin, boards[b].mapkit_start ) > 4 )
		{
			n++;
		}
	}
	return n;
}

mapkit_boardshots_cull()
{
	zombies = GetAiSpeciesArray( "axis", "all" );
	for ( z = 0; z < zombies.size; z++ )
	{
		zombies[z] DoDamage( zombies[z].health + 666, zombies[z].origin );
	}
}

mapkit_boardshots()
{
	flag_wait( "all_players_spawned" );
	wait( 2 );
	player = get_players()[0];
	level.mapkit_spawnpos = player.origin;
	level.mapkit_shot = 0;
	level.mapkit_shots = "";
	windows = [];
	for ( i = 0; i < bo1_mapkit_editcount(); i++ )
	{
		if ( bo1_mapkit_get( i, "kind" ) == "window" && IsDefined( bo1_mapkit_get( i, "origin" ) ) && !mapkit_window_open( i ) )
		{
			windows[windows.size] = i;
			mapkit_boards_moved( i );
		}
	}
	for ( w = 0; w < windows.size; w++ )
	{
		mapkit_boardshots_cam( player, windows[w], false );
		wait( 3 );
		mapkit_boardshots_shot( windows[w], "boarded" );
	}
	torn = -1;
	start = GetTime();
	while ( torn < 0 && GetTime() - start < 240000 )
	{
		wait( 0.1 );
		for ( w = 0; w < windows.size && torn < 0; w++ )
		{
			if ( mapkit_boards_moved( windows[w] ) >= 2 )
			{
				torn = windows[w];
			}
		}
	}
	if ( torn < 0 )
	{
		LogPrint( "mapkit: boardshots FAIL no window lost two boards in 240 s\n" );
		SetDvar( "bo1_autoquit", "1" );
		return;
	}
	mapkit_boardshots_cull();
	mapkit_boardshots_cam( player, torn, false );
	for ( t = 0; t < 12; t++ )
	{
		mapkit_boardshots_cull();
		wait( 0.25 );
	}
	mapkit_boardshots_shot( torn, "torn" );
	score = player.score;
	mapkit_boardshots_cam( player, torn, true );
	start = GetTime();
	while ( mapkit_boards_moved( torn ) > 0 && GetTime() - start < 40000 )
	{
		mapkit_boardshots_cull();
		wait( 0.5 );
	}
	LogPrint( "mapkit: boardshots rebuild " + ( mapkit_boards_moved( torn ) == 0 ) + " after " + ( GetTime() - start ) + " ms score " + score + " -> " + player.score + "\n" );
	mapkit_boardshots_cam( player, torn, false );
	wait( 3 );
	mapkit_boardshots_shot( torn, "rebuilt" );
	// The client draws/marks what the player shoots: a crash (r_scene "699 not in [0, 690)") was a closed kit door.
	for ( i = 0; i < bo1_mapkit_editcount(); i++ )
	{
		kind = bo1_mapkit_get( i, "kind" );
		if ( kind != "door" && kind != "debris" )
		{
			continue;
		}
		mins = bo1_mapkit_get( i, "mins" );
		maxs = bo1_mapkit_get( i, "maxs" );
		c = ( ( mins[0] + maxs[0] ) * 0.5, ( mins[1] + maxs[1] ) * 0.5, mins[2] );
		n = ( 0, 1, 0 );
		if ( maxs[0] - mins[0] < maxs[1] - mins[1] )
		{
			n = ( 1, 0, 0 );
		}
		if ( VectorDot( level.mapkit_spawnpos - c, n ) < 0 )
		{
			n = n * -1;
		}
		stand = c + n * 96;
		look = c + ( 0, 0, 48 );
		player SetOrigin( stand );
		SetDvar( "bo1_testclient_goto", stand[0] + "," + stand[1] + "," + stand[2] );
		SetDvar( "bo1_testclient_look", look[0] + "," + look[1] + "," + look[2] );
		SetDvar( "bo1_testclient_press", "0," + ( GetTime() + 1500 ) + ",400,200" );
		for ( t = 0; t < 16; t++ )
		{
			mapkit_boardshots_cull();
			wait( 0.25 );
		}
		SetDvar( "bo1_testclient_press", "" );
		LogPrint( "mapkit: boardshots door " + mapkit_name( i ) + " fired at " + look + " from " + player.origin + "\n" );
		mapkit_boardshots_shot( i, "door" );
	}
	LogPrint( "mapkit: boardshots END\n" );
	if ( GetDvar( "mapkit_clipshots" ) == "1" )
	{
		mapkit_clipshots(); // mapkit-fix-4b: then the machine clips (it quits)
		return;
	}
	SetDvar( "bo1_autoquit", "1" );
}

// mapkit-fix-4b: machine clip self-test (see main_end). gap = 2D distance of the stopped player from the clip box
mapkit_clipshots()
{
	flag_wait( "all_players_spawned" );
	wait( 2 );
	player = get_players()[0];
	level.mapkit_shot = 0;
	level.mapkit_shots = "";
	level.mapkit_clipinside = 0;
	level thread mapkit_clip_zombiewatch();
	pass = 0;
	fail = 0;
	for ( i = 0; i < bo1_mapkit_editcount(); i++ )
	{
		mins = bo1_mapkit_get( i, "_clipmins" );
		if ( !IsDefined( mins ) )
		{
			continue;
		}
		maxs = bo1_mapkit_get( i, "_clipmaxs" );
		o = bo1_mapkit_get( i, "_spot" );
		yaw = bo1_mapkit_get( i, "_spotyaw" );
		if ( !IsDefined( o ) )
		{
			o = bo1_mapkit_get( i, "origin" );
			yaw = bo1_mapkit_get( i, "yaw" );
		}
		if ( !IsDefined( yaw ) )
		{
			yaw = 0;
		}
		c = ( ( mins[0] + maxs[0] ) * 0.5, ( mins[1] + maxs[1] ) * 0.5, o[2] );
		stand = c + AnglesToForward( ( 0, yaw, 0 ) ) * 64 + ( 0, 0, 4 );
		// face the machine and walk straight at it (slowwalk: full forwardmove, reversing every 1.5 s; no path planner,
		// which would route around it); the closest approach over 1.4 s is the player stopped at the clip
		player SetOrigin( stand );
		SetDvar( "bo1_testclient_look", c[0] + "," + c[1] + "," + ( c[2] + 56 ) );
		// slowwalk walks forward while Int( level.time / 1500 ) is even: start at a forward phase
		while ( GetTime() % 3000 > 100 )
		{
			wait( 0.05 );
		}
		SetDvar( "bo1_testclient_slowwalk", 127 );
		gap = 9999;
		p = player.origin;
		for ( t = 0; t < 14; t++ )
		{
			q = player.origin;
			dx = max( max( mins[0] - q[0], q[0] - maxs[0] ), 0 );
			dy = max( max( mins[1] - q[1], q[1] - maxs[1] ), 0 );
			if ( Sqrt( dx * dx + dy * dy ) < gap )
			{
				gap = Sqrt( dx * dx + dy * dy );
				p = q;
			}
			wait( 0.1 );
		}
		SetDvar( "bo1_testclient_slowwalk", 0 );
		player SetOrigin( p );
		verdict = "PASS";
		if ( gap < 12 )
		{
			verdict = "FAIL";
			fail++;
		}
		else
		{
			pass++;
		}
		LogPrint( "mapkit: clipshots " + bo1_mapkit_get( i, "kind" ) + " edit " + i + " from " + stand + " stopped at " + p + " gap " + gap + " " + verdict + "\n" );
		// the test client only turns to bo1_testclient_look once it stops walking
		SetDvar( "bo1_testclient_goto", "" );
		wait( 0.5 );
		level.mapkit_shot++;
		level.mapkit_shots = level.mapkit_shots + level.mapkit_shot + " ";
		SetDvar( "bo1_shots", level.mapkit_shots );
		wait( 2 );
	}
	SetDvar( "bo1_testclient_goto", "" );
	LogPrint( "mapkit: clipshots END machines " + ( pass + fail ) + " pass " + pass + " fail " + fail + " zombie samples inside a clip " + level.mapkit_clipinside + "\n" );
	SetDvar( "bo1_autoquit", "1" );
}

mapkit_clip_zombiewatch()
{
	level endon( "end_game" );
	while ( 1 )
	{
		zombies = GetAiSpeciesArray( "axis", "all" );
		for ( i = 0; i < bo1_mapkit_editcount(); i++ )
		{
			mins = bo1_mapkit_get( i, "_clipmins" );
			if ( !IsDefined( mins ) )
			{
				continue;
			}
			maxs = bo1_mapkit_get( i, "_clipmaxs" );
			for ( z = 0; z < zombies.size; z++ )
			{
				q = zombies[z].origin;
				if ( q[0] > mins[0] && q[0] < maxs[0] && q[1] > mins[1] && q[1] < maxs[1] && q[2] + 8 > mins[2] && q[2] < maxs[2] )
				{
					level.mapkit_clipinside++;
				}
			}
		}
		wait( 0.25 );
	}
}

// test aid: the retail power switch is triggered and the player gets 20000 points
mapkit_test_power( player )
{
	mapkit_test_funds( player );
	if ( flag( "power_on" ) )
	{
		return;
	}
	sw = GetEnt( "use_elec_switch", "targetname" );
	if ( IsDefined( sw ) )
	{
		sw notify( "trigger", player );
	}
	wait( 2 );
	LogPrint( "mapkit: power_on " + flag( "power_on" ) + " score " + player.score + "\n" );
}

// A random Death Machine drop (30 s) sets the retail is_drinking count (_zombiemode_powerups minigun_weapon_powerup),
// and perk and wall-buy triggers refuse while is_drinking (_zombiemode_perks vending_trigger_think): a perk attempt
// during it logs "trigger used Nx but no perk" (sluice doctor run 13224, Speed Cola). Wait it out first.
mapkit_test_hands_free( player )
{
	start = GetTime();
	while ( player maps\_zombiemode_utility::is_drinking() && GetTime() - start < 35000 )
	{
		wait( 0.5 );
	}
	if ( GetTime() - start > 0 )
	{
		LogPrint( "mapkit: test waited " + ( GetTime() - start ) + " ms for is_drinking (Death Machine) to end before a purchase\n" );
		wait( 1 );
	}
}

mapkit_test_funds( player )
{
	mapkit_test_hands_free( player );
	if ( player.score < 20000 )
	{
		player maps\_zombiemode_score::add_to_player_score( 20000 - player.score );
	}
}

// test aid (mapkit_perktest 1): logs every use of a placed perk trigger with the user's origin and score
mapkit_perk_trigger_log( perk )
{
	for ( ;; )
	{
		self waittill( "trigger", who );
		self.mapkit_rec.uses++;
		LogPrint( "mapkit: perk trigger " + perk + " used by " + who.origin + " score " + who.score + " dist "
			+ Distance( who.origin, self.origin ) + "\n" );
		wait( 0.5 );
	}
}

// test aid mapkit_perktest 1: the retail power switch is triggered and the player gets 20000 points; the buy itself is
// the test client holding use at the machine (bo1_testclient_use); logs the layout perks the player has every 2 s
mapkit_perktest()
{
	flag_wait( "all_players_spawned" );
	wait( 3 );
	player = get_players()[0];
	sw = GetEnt( "use_elec_switch", "targetname" );
	if ( IsDefined( sw ) )
	{
		sw notify( "trigger", player );
	}
	wait( 2 );
	player maps\_zombiemode_score::add_to_player_score( 20000 );
	LogPrint( "mapkit: perktest power_on " + flag( "power_on" ) + " score " + player.score + "\n" );
	for ( t = 0; t < 60; t++ )
	{
		wait( 2 );
		for ( i = 0; i < bo1_mapkit_editcount(); i++ )
		{
			if ( bo1_mapkit_get( i, "kind" ) == "perk" )
			{
				perk = bo1_mapkit_get( i, "perk" );
				LogPrint( "mapkit: perktest " + perk + " has " + player HasPerk( perk ) + " score " + player.score + "\n" );
			}
		}
	}
}

// empty base (maps\mapkit\mapkit_main): start the zone manager on the layout's zones that have a volume in the
// entity string (info_volume, targetname = the zone name); _zombiemode::main has already reset level.zones
start_zones()
{
	level.zone_manager_init_func = ::mapkit_zone_connections;
	// Zone labels are authored into the generated window structs before retail struct_class_init.
	zones = [];
	for ( i = 0; i < bo1_mapkit_editcount(); i++ )
	{
		if ( bo1_mapkit_get( i, "kind" ) != "zone" )
		{
			continue;
		}
		name = mapkit_name( i );
		if ( !IsDefined( GetEnt( name, "targetname" ) ) )
		{
			LogPrint( "mapkit: zone " + name + " has no info_volume in the entity string: not started\n" );
			continue;
		}
		// A door's destination starts locked. Existing layouts without doors keep all their starting zones.
		locked = false;
		for ( d = 0; d < bo1_mapkit_editcount(); d++ )
		{
			kind = bo1_mapkit_get( d, "kind" );
			if ( ( kind == "door" || kind == "debris" ) && bo1_mapkit_get( d, "to" ) == name )
			{
				locked = true;
			}
		}
		// A loop can lead back into the player start; that zone must still be enabled at load.
		for ( s = 0; s < bo1_mapkit_editcount(); s++ )
		{
			if ( bo1_mapkit_get( s, "kind" ) == "spawn" && bo1_mapkit_get( s, "zone" ) == name )
			{
				locked = false;
			}
		}
		if ( !locked )
		{
			zones[zones.size] = name;
		}
	}
	LogPrint( "mapkit: empty base level main, " + zones.size + " zones started\n" );
	if ( zones.size > 0 )
	{
		level thread maps\_zombiemode_zone_manager::manage_zones( zones );
	}
}

// Adapter to the retail zone graph: the blocker purchase sets the same flag the manager waits on.
mapkit_zone_connections()
{
	for ( i = 0; i < bo1_mapkit_editcount(); i++ )
	{
		kind = bo1_mapkit_get( i, "kind" );
		if ( kind == "door" || kind == "debris" )
		{
			maps\_zombiemode_zone_manager::add_adjacent_zone( bo1_mapkit_get( i, "from" ), bo1_mapkit_get( i, "to" ), "mkdoor" + i + "_open" );
		}
	}
}

mapkit_blocker_open( i )
{
	flag_wait( "mkdoor" + i + "_open" );
	bo1_mapkit_visible( i, false );
	LogPrint( "mapkit: opened " + mapkit_name( i ) + " zone " + bo1_mapkit_get( i, "to" ) + "\n" );
}

// The passive use-button client cannot shoot while buying. Clear only nearby attackers
// during purchases so a functioning zombie graph cannot crowd it away from every trigger.
// Walking, collision, payment and grants are still checked; window trials keep actors alive.
mapkit_test_purchase_defence()
{
	while ( level.mapkit_test_purchases )
	{
		zombies = GetAiSpeciesArray( "axis", "all" );
		for ( z = 0; z < zombies.size; z++ )
		{
			if ( DistanceSquared( self.origin, zombies[z].origin ) < 96 * 96 )
			{
				mapkit_test_kill( zombies[z], self, "purchase_defence" );
			}
		}
		wait( 0.25 );
	}
}

mapkit_test_kill( zombie, player, reason )
{
	if ( GetDvar( "mapkit_doctor" ) == "1" )
	{
		id = zombie GetEntityNumber();
		LogPrint( "mapkit_doctor cull " + GetTime() + " " + id + " " + zombie.origin + " " + reason + "\n" );
	}
	// a test cull is not a player kill: no retail powerup drop (_zombiemode_spawner.gsc zombie_can_drop_powerups reads
	// no_powerups). Without this the culls left nukes / insta-kills floating at the riser photos and a nuke killed
	// the zombie under test.
	zombie.no_powerups = true;
	zombie DoDamage( zombie.health + 100, zombie.origin, player );
}

mapkit_test_visit( stand, look )
{
	SetDvar( "bo1_testclient_goto", stand[0] + "," + stand[1] + "," + stand[2] );
	SetDvar( "bo1_testclient_look", look[0] + "," + look[1] + "," + look[2] );
	SetDvar( "bo1_testclient_press", "3," + GetTime() + ",1500,200" );
}

mapkit_window_zone( i )
{
	o = bo1_mapkit_get( i, "origin" );
	yaw = bo1_mapkit_get( i, "yaw" );
	if ( !IsDefined( o ) )
	{
		return "";
	}
	if ( !IsDefined( yaw ) )
	{
		yaw = 90;
	}
	o += AnglesToForward( ( 0, yaw, 0 ) ) * 48 + ( 0, 0, 32 );
	for ( z = 0; z < bo1_mapkit_editcount(); z++ )
	{
		if ( bo1_mapkit_get( z, "kind" ) != "zone" )
		{
			continue;
		}
		lo = bo1_mapkit_get( z, "mins" );
		hi = bo1_mapkit_get( z, "maxs" );
		if ( o[0] >= lo[0] && o[0] <= hi[0] && o[1] >= lo[1] && o[1] <= hi[1] && o[2] >= lo[2] && o[2] <= hi[2] )
		{
			return mapkit_name( z );
		}
	}
	return "";
}

// Exercise actual use-button purchases, then require the retail zone manager and new-window spawning.
mapkit_check_progression( player )
{
	for ( i = 0; i < bo1_mapkit_editcount(); i++ )
	{
		kind = bo1_mapkit_get( i, "kind" );
		if ( kind != "door" && kind != "debris" )
		{
			continue;
		}
		to = bo1_mapkit_get( i, "to" );
		if ( !IsDefined( GetEnt( "mkdoor" + i, "targetname" ) ) )
		{
			LogPrint( "mapkit: check " + kind + " " + mapkit_name( i ) + " FAIL missing blocker (requires empty base)\n" );
			continue;
		}
		// A blocker whose `to` zone is already enabled closes a loop: check only the path through its own doorway.
		was_locked = !maps\_zombiemode_zone_manager::zone_is_enabled( to );
		bo1_mapkit_graphcheck( i, false, !was_locked );
		lo = bo1_mapkit_get( i, "mins" );
		o = mapkit_center( i );
		yaw = bo1_mapkit_get( i, "yaw" );
		if ( !IsDefined( yaw ) )
		{
			yaw = 0;
		}
		f = AnglesToForward( ( 0, yaw, 0 ) );
		stand = ( o[0], o[1], lo[2] ) - f * 48;
		player maps\_zombiemode_score::add_to_player_score( 5000 );
		before = player.score;
		mapkit_test_visit( stand, o );
		start = GetTime();
		while ( !flag( "mkdoor" + i + "_open" ) && GetTime() - start < 25000 )
		{
			wait( 0.25 );
		}
		SetDvar( "bo1_testclient_press", "" );
		if ( flag( "mkdoor" + i + "_open" ) && player.score < before )
		{
			LogPrint( "mapkit: check " + kind + " " + mapkit_name( i ) + " PASS buyable, charged " + ( before - player.score ) + "\n" );
		}
		else
		{
			LogPrint( "mapkit: check " + kind + " " + mapkit_name( i ) + " FAIL not purchased within 25 s\n" );
			continue;
		}
		// Walk through the cleared opening; occupancy activates the destination's spawners.
		mapkit_test_visit( ( o[0], o[1], lo[2] ) + f * 80, o + f * 150 );
		SetDvar( "bo1_testclient_press", "" );
		wait( 4 );
		bo1_mapkit_graphcheck( i, true, !was_locked );
		if ( was_locked && maps\_zombiemode_zone_manager::zone_is_enabled( to ) && level.zones[to].is_active )
		{
			LogPrint( "mapkit: check unlock " + to + " PASS locked -> enabled and active\n" );
		}
		else if ( !was_locked && maps\_zombiemode_zone_manager::zone_is_enabled( to ) && level.zones[to].is_active )
		{
			// Loop-closer: its windows were already checked when the first blocker unlocked this zone.
			LogPrint( "mapkit: check unlock " + to + " PASS loop-closer: already enabled through another blocker, still active\n" );
			continue;
		}
		else
		{
			LogPrint( "mapkit: check unlock " + to + " FAIL zone did not become active after purchase\n" );
		}
		for ( w = 0; w < bo1_mapkit_editcount(); w++ )
		{
			if ( bo1_mapkit_get( w, "kind" ) != "window" || mapkit_window_zone( w ) != to )
			{
				continue;
			}
			found = false;
			start = GetTime();
			while ( !found && GetTime() - start < 90000 )
			{
				zombies = GetAiSpeciesArray( "axis", "all" );
				for ( z = 0; z < zombies.size; z++ )
				{
					if ( !IsDefined( zombies[z].first_node ) )
					{
						continue;
					}
					// mapkit-fix-4e: an open entrance's exterior_goal has no target (sluice_works sluice_gate: 33 script
					// errors "pair 'undefined' and 'mkwinN'" here); its zombies are known by their source window.
					if ( IsDefined( zombies[z].first_node.target ) )
					{
						mine = zombies[z].first_node.target == "mkwin" + w;
					}
					else
					{
						mine = IsDefined( zombies[z].mapkit_source_window ) && zombies[z].mapkit_source_window == w;
					}
					if ( mine )
					{
						found = true;
					}
					// Free the round's population so a passive test can exercise newly opened zones.
					// Do not kill a newly spawned actor before retail script assigns its entrance.
					if ( GetTime() - start > 10000 && !found && !mine
						&& ( IsDefined( zombies[z].first_node.target ) || IsDefined( zombies[z].mapkit_source_window ) ) )
					{
						mapkit_test_kill( zombies[z], player, "unlock_other_window" );
					}
				}
				wait( 0.25 );
			}
			if ( found )
			{
				LogPrint( "mapkit: check unlock_window " + mapkit_name( w ) + " PASS zombie selected this entrance after opening " + to + "\n" );
			}
			else
			{
				LogPrint( "mapkit: check unlock_window " + mapkit_name( w ) + " FAIL no spawn within 90 s\n" );
			}
		}
	}
	for ( i = 0; i < bo1_mapkit_editcount(); i++ )
	{
		if ( bo1_mapkit_get( i, "kind" ) != "mysterybox" )
		{
			continue;
		}
		trigs = GetEntArray( "mkchest" + i + "_lid", "target" );
		if ( !trigs.size )
		{
			LogPrint( "mapkit: check mysterybox " + mapkit_name( i ) + " FAIL missing trigger\n" );
			continue;
		}
		chest = trigs[0];
		if ( IsDefined( chest.disabled ) && chest.disabled )
		{
			continue; // retail selects one starting location; the others are relocation destinations
		}
		mapkit_test_hands_free( player );
		o = bo1_mapkit_get( i, "origin" );
		yaw = bo1_mapkit_get( i, "yaw" );
		if ( !IsDefined( yaw ) )
		{
			yaw = 270;
		}
		player maps\_zombiemode_score::add_to_player_score( 5000 );
		before = player.score;
		start = GetTime();
		mapkit_test_visit( o + AnglesToForward( ( 0, yaw, 0 ) ) * 44, o + ( 0, 0, 32 ) );
		weapon = undefined;
		while ( GetTime() - start < 30000 )
		{
			// Retail clears weapon_string when the offer is taken; retain it before that notify.
			if ( IsDefined( chest.chest_origin.weapon_string ) )
			{
				weapon = chest.chest_origin.weapon_string;
			}
			if ( IsDefined( player.last_box_weapon ) && player.last_box_weapon >= start )
			{
				// Retail stamps last_box_weapon before its grant thread finishes. Wait for inventory too.
				SetDvar( "bo1_testclient_press", "" );
				if ( IsDefined( weapon ) && player HasWeapon( weapon ) )
				{
					break;
				}
			}
			wait( 0.25 );
		}
		SetDvar( "bo1_testclient_press", "" );
		if ( IsDefined( player.last_box_weapon ) && player.last_box_weapon >= start && player.score < before
			&& IsDefined( weapon ) && player HasWeapon( weapon ) )
		{
			LogPrint( "mapkit: check mysterybox " + mapkit_name( i ) + " PASS bought weapon " + weapon + "\n" );
		}
		else
		{
			LogPrint( "mapkit: check mysterybox " + mapkit_name( i ) + " FAIL no weapon within 30 s\n" );
		}
	}
}

// mapkit-fix-4c: window "open": true (empty base) - an unboardable entrance. The C++ gives its exterior_goal no target
// (retail blocker_init/tear_into_building: no barrier). Retail code that walks every goal's chunks (carpenter,
// get_num_window_destroyed, all_chunks_destroyed) reads barrier_chunks, so a goal with no target gets an empty list.
mapkit_window_open( i )
{
	o = bo1_mapkit_get( i, "open" );
	return IsDefined( o ) && o != 0;
}

mapkit_open_goals()
{
	goals = GetStructArray( "exterior_goal", "targetname" );
	for ( g = 0; g < goals.size; g++ )
	{
		if ( !IsDefined( goals[g].target ) && !IsDefined( goals[g].barrier_chunks ) )
		{
			goals[g].barrier_chunks = [];
		}
	}
}

// mapkit-fix-4c: riser edits (empty base). Retail does the rise (spawner.gsc do_zombie_rise: script_string "riser",
// a random spot of zone.rise_locations, climb-out anim + rise fx, "risen" -> find_flesh). This only watches: every
// riser actor is attributed to the riser edit nearest the spot it climbed out at, then tracked to the player.
mapkit_riser_watch()
{
	level.mapkit_riser_state = [];
	while ( 1 )
	{
		zombies = GetAiSpeciesArray( "axis", "all" );
		for ( z = 0; z < zombies.size; z++ )
		{
			if ( IsDefined( zombies[z].script_string ) && zombies[z].script_string == "riser" && !IsDefined( zombies[z].mapkit_riser_seen ) )
			{
				zombies[z].mapkit_riser_seen = true;
				zombies[z] thread mapkit_riser_track();
			}
		}
		wait( 0.25 );
	}
}

mapkit_riser_track()
{
	self endon( "death" );
	id = self GetEntityNumber();
	start = GetTime();
	while ( !IsDefined( self.in_the_ground ) && GetTime() - start < 2000 )
	{
		wait( 0.05 );
	}
	underground = IsDefined( self.in_the_ground ) && self.in_the_ground;
	// +set mapkit_risershots 1 (-Client 1x): the first climb-out seen while the self-test client stands at a riser is
	// photographed twice (mid-climb, out), from the client's view of the spot
	if ( underground && GetDvar( "mapkit_risershots" ) == "1" && IsDefined( level.mapkit_riser_checking ) && !IsDefined( level.mapkit_risershot ) )
	{
		level.mapkit_risershot = true;
		self thread mapkit_risershot( id );
	}
	while ( IsDefined( self.in_the_ground ) && self.in_the_ground && GetTime() - start < 20000 )
	{
		wait( 0.1 );
	}
	o = self.origin;
	r = undefined;
	best = 128 * 128;
	for ( e = 0; e < bo1_mapkit_editcount(); e++ )
	{
		if ( bo1_mapkit_get( e, "kind" ) == "riser" && DistanceSquared( o, bo1_mapkit_get( e, "origin" ) ) < best )
		{
			best = DistanceSquared( o, bo1_mapkit_get( e, "origin" ) );
			r = e;
		}
	}
	if ( !IsDefined( r ) )
	{
		LogPrint( "mapkit: riser actor " + id + " climbed out at " + o + " near no riser edit (underground " + underground + ")\n" );
		return;
	}
	name = mapkit_name( r );
	self.mapkit_source_riser = r;
	if ( !IsDefined( level.mapkit_riser_state[ name ] ) )
	{
		level.mapkit_riser_state[ name ] = "rose";
	}
	LogPrint( "mapkit: riser " + name + " zombie #" + id + " rose at " + o + " after " + ( GetTime() - start ) + " ms (underground " + underground + ")\n" );
	while ( 1 )
	{
		wait( 0.25 );
		players = get_players();
		for ( p = 0; p < players.size; p++ )
		{
			if ( DistanceSquared( self.origin, players[p].origin ) < 64 * 64 )
			{
				level.mapkit_riser_state[ name ] = "reached";
				LogPrint( "mapkit: riser " + name + " zombie #" + id + " reached player " + p + " at " + self.origin + " after " + ( GetTime() - start ) + " ms\n" );
				return;
			}
		}
	}
}

mapkit_riser_state( name )
{
	if ( !IsDefined( level.mapkit_riser_state ) || !IsDefined( level.mapkit_riser_state[ name ] ) )
	{
		return "";
	}
	return level.mapkit_riser_state[ name ];
}

// self-test: stand 192 in front of each riser (its zone active by occupancy) and wait for a zombie that climbed out
// of it to reach the player. Zombies from windows and other risers are culled after 5 s so the round keeps spawning.
mapkit_check_risers( player )
{
	for ( i = 0; i < bo1_mapkit_editcount(); i++ )
	{
		if ( bo1_mapkit_get( i, "kind" ) != "riser" || !IsDefined( level.mapkit_empty ) )
		{
			continue;
		}
		name = mapkit_name( i );
		ro = bo1_mapkit_get( i, "origin" );
		yaw = bo1_mapkit_get( i, "yaw" );
		if ( !IsDefined( yaw ) )
		{
			yaw = 0;
		}
		mapkit_test_visit( ro + AnglesToForward( ( 0, yaw, 0 ) ) * 192, ro + ( 0, 0, 32 ) );
		SetDvar( "bo1_testclient_press", "" );
		level.mapkit_riser_checking = i;
		start = GetTime();
		while ( GetTime() - start < 90000 && mapkit_riser_state( name ) != "reached" )
		{
			if ( GetTime() - start > 5000 )
			{
				zombies = GetAiSpeciesArray( "axis", "all" );
				for ( z = 0; z < zombies.size; z++ )
				{
					if ( !IsDefined( zombies[z].mapkit_riser_seen ) || ( IsDefined( zombies[z].mapkit_source_riser ) && zombies[z].mapkit_source_riser != i ) )
					{
						mapkit_test_kill( zombies[z], player, "other_entrance" );
					}
				}
			}
			wait( 1 );
		}
		if ( mapkit_riser_state( name ) == "reached" )
		{
			LogPrint( "mapkit: check riser " + name + " PASS a zombie climbed out of it and reached the player\n" );
		}
		else if ( mapkit_riser_state( name ) != "" )
		{
			LogPrint( "mapkit: check riser " + name + " FAIL within 90 s: zombies rose there but none reached the player\n" );
		}
		else
		{
			LogPrint( "mapkit: check riser " + name + " FAIL within 90 s: no zombie rose there\n" );
		}
	}
}

mapkit_risershot( id )
{
	self endon( "death" );
	if ( !IsDefined( level.mapkit_shot ) )
	{
		level.mapkit_shot = 0;
		level.mapkit_shots = "";
	}
	for ( k = 0; k < 2; k++ )
	{
		wait( 1.5 );
		level.mapkit_shot++;
		level.mapkit_shots = level.mapkit_shots + level.mapkit_shot + " ";
		LogPrint( "mapkit: risershot shot " + level.mapkit_shot + " zombie #" + id + " at " + self.origin + " in_the_ground " + self.in_the_ground + "\n" );
		SetDvar( "bo1_shots", level.mapkit_shots );
	}
}
