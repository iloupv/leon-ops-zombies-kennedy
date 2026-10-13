#include maps\_utility;
#include common_scripts\utility;

// LEON MOD - not part of the original game. Moon's astronaut helmet (its gas mask equipment) on Leon.
// Moon (maps\zombie_moon.gsc) wears the helmet by swapping the player's body for the pressure suit's and his head
// model for the helmet (level.zombiemode_gasmask_* callbacks, called by maps\_zombiemode_equip_gasmask.gsc). On Leon
// the body swap made the mod set Leon back (maps\leon\_leon.gsc leon_keep_model), the helmet with it. Here the
// callbacks keep Leon and put the helmet on his head (j_head) while the mask is on.
//   leon_helmettest 1   tests: the helmet given and put on (Moon)

init()
{
	// the map sets its callbacks in its main, after this hook: wait for them
	for ( i = 0; i < 100 && !isDefined( level.zombiemode_gasmask_change_player_headmodel ); i++ )
		wait 0.1;
	if ( !isDefined( level.zombiemode_gasmask_change_player_headmodel ) )
		return;
	level.zombiemode_gasmask_set_player_model = ::leon_gasmask_set_player_model;
	level.zombiemode_gasmask_change_player_headmodel = ::leon_gasmask_change_player_headmodel;
	level.zombiemode_gasmask_reset_player_model = ::leon_gasmask_reset_player_model;
	logPrint( "leon: helmet callbacks set\n" );
	if ( getDvar( "leon_helmettest" ) == "1" )
		level thread leon_helmet_test();
	if ( getDvar( "leon_helmettest" ) == "2" )
		level thread leon_helmet_fit_test();
}

// the suit picked up: Leon stays Leon (the helmet comes on with the mask)
leon_gasmask_set_player_model( entity_num )
{
}

// the mask on / off: the helmet on his head / off
leon_gasmask_change_player_headmodel( entity_num, gasmask_active )
{
	logPrint( "leon: helmet mask " + gasmask_active + "\n" );
	if ( gasmask_active )
	{
		if ( !isDefined( self.leon_helmet ) )
		{
			self.leon_helmet = "c_zom_moon_pressure_suit_helm";
			self attach( self.leon_helmet, "tag_helmet", true );
		}
	}
	else
		self leon_helmet_off();
}

// the equipment lost: no helmet
leon_gasmask_reset_player_model( entity_num )
{
	self leon_helmet_off();
}

leon_helmet_off()
{
	if ( isDefined( self.leon_helmet ) )
	{
		self detach( self.leon_helmet, "tag_helmet" );
		self.leon_helmet = undefined;
	}
}

// tests: the helmet straight on his head, him left alone (its fit, from the camera's angles)
leon_helmet_fit_test()
{
	players = getEntArray( "player", "classname" );
	while ( players.size == 0 )
	{
		wait 0.5;
		players = getEntArray( "player", "classname" );
	}
	player = players[0];
	player enableInvulnerability();
	player.ignoreme = true;
	wait 3;
	player leon_gasmask_change_player_headmodel( 0, true );
}

// tests: the helmet given, then put on (the equipment's weapon selected, as its key does)
leon_helmet_test()
{
	players = getEntArray( "player", "classname" );
	while ( players.size == 0 )
	{
		wait 0.5;
		players = getEntArray( "player", "classname" );
	}
	player = players[0];
	player enableInvulnerability();
	player.ignoreme = true; // (the zombies leave him be)
	wait 4;
	player maps\_zombiemode_equipment::equipment_give( "equip_gasmask_zm" );
	logPrint( "leon: helmet given\n" );
	wait 1;
	player switchToWeapon( "equip_gasmask_zm" );
	logPrint( "leon: helmet put on\n" );
	wait 6;
	player switchToWeapon( "equip_gasmask_zm" ); // again: off
	logPrint( "leon: helmet taken off\n" );
}
