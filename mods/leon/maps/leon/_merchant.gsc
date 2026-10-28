#include maps\_utility;
#include common_scripts\utility;

// LEON MOD - not part of the original game. RE4's Merchant at the Mystery Box: his shop lines (the runtime sounds
// merchant_*, from the player's own RE4: tools/leon-converter/leon_sounds.py) to the player who uses it - "What're ya
// buyin'?" as it opens, a line by the weapon's tier once it shows it (_zombiemode_weapons' chest_origin notifies
// "randomization_done", its weapon in weapon_string), "Come back any time." for the teddy bear (no weapon_string: the
// box flies off - RE4's Merchant as he closes his coat after the shop), and "Not enough cash, stranger." on a
// use he cannot pay for - at the box, the doors and debris, the wall guns, the perk machines and Pack-a-Punch (BO1's
// own line for it is the characters' voice, silent under Leon; its deny sounds also play for a perk he has or a
// fourth perk, so the price is checked here instead, as each one's script checks it).
//   amazing  the wonder weapons: "Stranger, stranger, now that's a weapon!"
//   strong   the heavy guns: "...you'll need guts to buy that weapon." / "...Going huntin' an elephant?"
//   weak     the Cymbal Monkey, the Ballistic Knife, the pistols: "Is that all, stranger?"
//   good     the rest: "A choice of an avid gun collector..." / "A wise choice, mate..." / "Hehehe, thank you!"
//   leon_merchant 0  off
//   leon_boxtest 1   test: 5000 points and the player in front of the box (its use: the use key)
//   leon_boxtest teddy  the same, its next pull the teddy bear
//   leon_poortest 1  test: the player down to 10 points

init()
{
	if ( getDvar( "leon_merchant" ) == "0" )
		return;
	while ( !isDefined( level.chests ) )
		wait 0.5;
	for ( i = 0; i < level.chests.size; i++ )
		level.chests[i] thread leon_merchant_chest();
	level thread leon_merchant_buyables();
	if ( getDvar( "leon_boxtest" ) == "1" || getDvar( "leon_boxtest" ) == "teddy" )
		level thread leon_merchant_boxtest();
}

// Test: the first player, 5000 points richer, in front of the box (where the box stands: level.chest_index)
leon_merchant_boxtest()
{
	wait 2;
	players = getEntArray( "player", "classname" );
	if ( !players.size || !isDefined( level.chest_index ) )
		return;
	player = players[0];
	player.score = player.score + 5000;
	chest = level.chests[ level.chest_index ];
	forward = anglesToRight( chest.angles );
	spot = groundpos( chest.origin + forward * 50 + ( 0, 0, 20 ) );
	player setOrigin( spot );
	player setPlayerAngles( vectorToAngles( chest.origin - spot ) );
	if ( getDvar( "leon_boxtest" ) == "teddy" )
		level.chest_accessed = 8; // (_zombiemode_weapons: the eighth pull of a box never moved is the teddy bear)
	logPrint( "leon: boxtest at the box " + chest.origin + "\n" );
}

// (heard by every player, as the bingo lines: co-op heard only the buyer's)
leon_merchant_say( player, alias )
{
	players = getEntArray( "player", "classname" );
	for ( i = 0; i < players.size; i++ )
		players[i] playLocalSound( alias );
	logPrint( "leon: merchant " + alias + "\n" );
}

leon_merchant_chest()
{
	self thread leon_merchant_open();
	self thread leon_merchant_nocash();
	while ( !isDefined( self.chest_origin ) )
		wait 0.5;
	for ( ;; )
	{
		self.chest_origin waittill( "randomization_done" );
		// the teddy bear, as it flies off with the box (the box's own think, on the same notify, has let its user go
		// already: no wait for a grab)
		if ( !isDefined( self.chest_origin.weapon_string ) )
			leon_merchant_say( self.chest_user, "merchant_comeback" );
		else if ( isDefined( self.chest_user ) )
			leon_merchant_say( self.chest_user, "merchant_" + leon_merchant_tier( self.chest_origin.weapon_string ) );
	}
}

// "What're ya buyin'?" as a paid use opens the box
leon_merchant_open()
{
	wasOpen = false;
	for ( ;; )
	{
		wait 0.05;
		open = isDefined( self._box_open ) && self._box_open;
		if ( open && !wasOpen && isDefined( self.chest_user ) )
			leon_merchant_say( self.chest_user, "merchant_buyin" );
		wasOpen = open;
	}
}

// "Not enough cash, stranger." on a use he cannot pay for
leon_merchant_nocash()
{
	for ( ;; )
	{
		self waittill( "trigger", user );
		if ( !isPlayer( user ) || ( isDefined( self._box_open ) && self._box_open ) || !isDefined( self.zombie_cost ) )
			continue;
		if ( user leon_merchant_score() < self.zombie_cost )
			user leon_merchant_broke();
	}
}

// self: a player coming in (_leon's setup) - his score followed
player_init()
{
	if ( getDvar( "leon_merchant" ) == "0" )
		return;
	self thread leon_merchant_score_track();
}

// the other buyables: each one's use against its price, as its own script reckons it
leon_merchant_buyables()
{
	players = getEntArray( "player", "classname" );
	while ( players.size == 0 )
	{
		wait 0.5;
		players = getEntArray( "player", "classname" );
	}
	if ( getDvar( "leon_poortest" ) == "1" )
		players[0].score = 10;
	kinds = strTok( "zombie_door zombie_debris weapon_upgrade zombie_vending zombie_vending_upgrade", " " );
	for ( k = 0; k < kinds.size; k++ )
	{
		triggers = getEntArray( kinds[k], "targetname" );
		for ( i = 0; i < triggers.size; i++ )
			triggers[i] thread leon_merchant_buyable( kinds[k] );
	}
}

leon_merchant_buyable( kind )
{
	if ( kind == "zombie_vending_upgrade" )
		self thread leon_merchant_pap_holding();
	for ( ;; )
	{
		self waittill( "trigger", who );
		if ( !isPlayer( who ) )
			continue;
		// Pack-a-Punch: its use takes the finished gun too (not a buy), and a gun already upgraded is refused
		// without a price (_zombiemode_perks vending_weapon_upgrade)
		if ( kind == "zombie_vending_upgrade" && ( ( isDefined( self.leon_pap_holding_until ) && getTime() <= self.leon_pap_holding_until )
			|| who maps\_zombiemode_weapons::is_weapon_upgraded( who getCurrentWeapon() ) ) )
			continue;
		price = self leon_merchant_price( kind, who );
		if ( isDefined( price ) && who leon_merchant_score() < price )
			who leon_merchant_broke();
	}
}

// self: Pack-a-Punch's trigger - while it holds a finished gun to take (its current_weapon), and a moment after:
// the take's own use ends the hold (current_weapon emptied) before or after this script's turn on that use
leon_merchant_pap_holding()
{
	for ( ;; )
	{
		if ( isDefined( self.current_weapon ) && self.current_weapon != "" )
			self.leon_pap_holding_until = getTime() + 250;
		wait 0.05;
	}
}

// what the buyable asks of him: a door's or debris' zombie_cost; a wall gun's price, its ammo's when he has it (4500
// with its upgrade); a perk's (_zombiemode_perks' table: Quick Revive 500 alone); Pack-a-Punch's
leon_merchant_price( kind, who )
{
	if ( kind == "zombie_door" || kind == "zombie_debris" )
		return self.zombie_cost;
	if ( kind == "zombie_vending_upgrade" )
		return self.cost;
	if ( kind == "weapon_upgrade" )
	{
		weapon = self.zombie_weapon_upgrade;
		if ( !isDefined( weapon ) || !isDefined( level.zombie_weapons ) || !isDefined( level.zombie_weapons[ weapon ] ) )
			return undefined;
		struct = level.zombie_weapons[ weapon ];
		if ( isDefined( struct.upgrade_name ) && who hasWeapon( struct.upgrade_name ) )
			return 4500;
		if ( who hasWeapon( weapon ) )
			return struct.ammo_cost;
		return struct.cost;
	}
	perk = self.script_noteworthy;
	if ( !isDefined( perk ) )
		return undefined;
	if ( isSubStr( perk, "armorvest" ) )
		return 2500;
	if ( isSubStr( perk, "quickrevive" ) )
	{
		if ( getEntArray( "player", "classname" ).size == 1 )
			return 500;
		return 1500;
	}
	if ( isSubStr( perk, "fastreload" ) )
		return 3000;
	if ( isSubStr( perk, "rof" ) || isSubStr( perk, "longersprint" ) || isSubStr( perk, "flakjacket" ) )
		return 2000;
	if ( isDefined( level.zombie_vars ) && isDefined( level.zombie_vars[ "zombie_perk_cost" ] ) )
		return level.zombie_vars[ "zombie_perk_cost" ];
	return undefined;
}

// his score as the frame began: a buy that went through takes its points on the use itself, which then read as too
// few (scripts waiting a frame resume before the triggers' notifies)
leon_merchant_score_track()
{
	self endon( "disconnect" );
	for ( ;; )
	{
		self.leon_merchant_score = self.score;
		wait 0.05;
	}
}

leon_merchant_score()
{
	if ( isDefined( self.leon_merchant_score ) )
		return self.leon_merchant_score;
	return self.score;
}

// "Not enough cash, stranger." (not again for 3 s)
leon_merchant_broke()
{
	if ( isDefined( self.leon_merchant_broke ) && getTime() - self.leon_merchant_broke < 3000 )
		return;
	self.leon_merchant_broke = getTime();
	leon_merchant_say( self, "merchant_nocash" );
}

leon_merchant_tier( weapon )
{
	if ( leon_merchant_in( "ray_gun_zm thundergun_zm tesla_gun_zm freezegun_zm microwavegundw_zm microwavegun_zm "
		+ "shrink_ray_zm humangun_zm sniper_explosive_zm", weapon ) )
		return "amazing";
	if ( leon_merchant_in( "hk21_zm rpk_zm spas_zm hs10_zm china_lake_zm m72_law_zm crossbow_explosive_zm python_zm "
		+ "l96a1_zm dragunov_zm", weapon ) )
		return "strong";
	if ( leon_merchant_in( "zombie_cymbal_monkey knife_ballistic_zm cz75_zm cz75dw_zm m1911_zm", weapon ) )
		return "weak";
	return "good";
}

// whether the weapon is one of the space-separated names
leon_merchant_in( names, weapon )
{
	list = strTok( names, " " );
	for ( i = 0; i < list.size; i++ )
		if ( list[i] == weapon )
			return true;
	return false;
}
