#include maps\_utility;
#include common_scripts\utility;

// LEON MOD - not part of the original game. RE4's item pickup sound (the runtime sound "leon_pickup", from the
// player's own RE4) when Leon gets a gun (a wall weapon bought, the box's taken) or ammo (a wall weapon's ammo bought,
// Max Ammo). From his weapons: a new one in his list, or a weapon's stock going up (a reload only lowers it);
// grenades (refilled every round), perk bottles, Pack-a-Punch's knuckle crack, equipment and the last stand pistol
// left out.

init( player )
{
	player endon( "disconnect" );
	prevList = player getWeaponsList();
	prevStock = [];
	for ( i = 0; i < prevList.size; i++ )
		prevStock[prevList[i]] = player getWeaponAmmoStock( prevList[i] );
	upTime = 0;
	for ( ;; )
	{
		wait 0.05;
		list = player getWeaponsList();
		// down, and for a second after (the last stand hands his guns back): no sound, the lists just follow
		if ( isDefined( player.revivetrigger ) )
			upTime = getTime() + 1000;
		if ( getTime() < upTime )
		{
			prevList = list;
			prevStock = [];
			for ( i = 0; i < list.size; i++ )
				prevStock[list[i]] = player getWeaponAmmoStock( list[i] );
			continue;
		}
		stock = [];
		got = false;
		for ( i = 0; i < list.size; i++ )
		{
			w = list[i];
			stock[w] = player getWeaponAmmoStock( w );
			if ( !leon_pickup_counts( player, w ) )
				continue;
			if ( !leon_in_list( prevList, w ) )
				got = true; // a new gun
			else if ( isDefined( prevStock[w] ) && stock[w] > prevStock[w] )
				got = true; // more ammo
		}
		if ( got )
			player playLocalSound( "leon_pickup" );
		prevList = list;
		prevStock = stock;
	}
}

// a gun he picked up (not a grenade, a bottle, the knuckle crack, equipment, nor anything while down)
leon_pickup_counts( player, w )
{
	if ( isDefined( player.revivetrigger ) )
		return false; // down: the last stand's pistol
	if ( WeaponClass( w ) == "grenade" )
		return false;
	if ( isSubStr( w, "bottle" ) || isSubStr( w, "knuckle" ) || isSubStr( w, "equip_" ) || isSubStr( w, "syrette" ) )
		return false;
	return true;
}

leon_in_list( list, w )
{
	for ( i = 0; i < list.size; i++ )
	{
		if ( list[i] == w )
			return true;
	}
	return false;
}
