// LEON MOD - not part of the original game. The character the player plays: Leon, or Ada, HUNK or Luis - each a
// runtime model the mod's setup converts from the player's own RE4 (rtmodels/<name>.rlm; leon.cfg's bo1_rtmodels lists
// them, and the engine's bo1_rtmodels_loaded those this install has). They all play RE4's player motions (Leon's for
// what a character has none of), the kick included, and say their own pain, death and kick (the engine's CG_LeonVoice
// reads leon_character, set on each player's own client; the others hear his from his model). Each player has his own
// (self.leon_character): the host the one he picked last (leon_character, saved in his config), a player who joins the
// first one nobody plays yet. At the start of a match "Press SWITCH WEAPON to change character" goes through them.

// From _leon's init, before the level's first frame: the characters' models precached.
precache()
{
	level.leon_characters = [];
	loaded = strTok( getDvar( "bo1_rtmodels_loaded" ), " " );
	order = strTok( "leon ada hunk luis", " " );
	for ( i = 0; i < order.size; i++ )
	{
		for ( j = 0; j < loaded.size; j++ )
		{
			if ( loaded[j] == order[i] )
				level.leon_characters[ level.leon_characters.size ] = order[i];
		}
	}
	if ( level.leon_characters.size == 0 )
		level.leon_characters[0] = "leon"; // (none built: precached as before, BO1's default model in its place)
	for ( i = 0; i < level.leon_characters.size; i++ )
		precacheModel( level.leon_characters[i] );
}

// the character this install's player picked last (leon_character), when it has it; else Leon
saved()
{
	name = getDvar( "leon_character" );
	for ( i = 0; i < level.leon_characters.size; i++ )
	{
		if ( level.leon_characters[i] == name )
			return name;
	}
	return level.leon_characters[0];
}

// self: the player - his character
current()
{
	if ( isDefined( self.leon_character ) )
		return self.leon_character;
	return saved();
}

// self: the player - his character as he comes in: the host's saved pick, another's the first nobody plays yet
first_pick()
{
	players = getEntArray( "player", "classname" );
	if ( players.size == 0 || players[0] == self )
		return saved();
	for ( i = 0; i < level.leon_characters.size; i++ )
	{
		taken = false;
		for ( j = 0; j < players.size; j++ )
		{
			if ( players[j] != self && isDefined( players[j].leon_character ) && players[j].leon_character == level.leon_characters[i] )
				taken = true;
		}
		if ( !taken )
			return level.leon_characters[i];
	}
	return level.leon_characters[0];
}

// self: the player, as the character (his own client told it: his voice, and his config keeps it)
apply( name )
{
	self.leon_character = name;
	self setClientDvar( "leon_character", name );
	self detachAll();
	self setModel( name );
	logPrint( "leon: " + self.playername + " plays " + name + "\n" );
}

// The characters' perks (each player's, from his character as it is now - a switch at the start changes them):
//   Leon: his shots stagger zombies more easily (maps\leon\_kick.gsc) and one more hit (50 more health over the game's
//         own: 100, Juggernog's 250)
//   HUNK: aims, reloads, switches guns and throws grenades faster (the engine's handling flag: leonsethandy)
//   Ada:  a second roll for a power-up drop on each kill, a wider, harder kick (maps\leon\_kick.gsc), and 15% back on
//         everything she buys (ada_discount)
//   Luis: a quarter more damage, and his grenades' and launchers' / wonder weapons' blasts reach further
// self: the player
perks()
{
	self endon( "disconnect" );
	self thread luis_blasts( "grenade_fire" );
	self thread luis_blasts( "missile_fire" );
	self thread ada_discount();
	for ( ;; )
	{
		name = self current();
		self leonsethandy( name == "hunk" );
		if ( isDefined( self.maxhealth ) && self.maxhealth > 0 )
		{
			bonus = 0;
			if ( name == "leon" )
				bonus = 50;
			// the game's own maximum (it sets it: a spawn, a revive, Juggernog), Leon's bonus over it
			if ( !isDefined( self.leon_max ) || self.maxhealth != self.leon_max )
				self.leon_base_health = self.maxhealth;
			target = self.leon_base_health + bonus;
			// through the engine's SetMaxHealth, as Juggernog's own: his health, his max and the session's max in one.
			// The field alone (self.maxhealth) left the session's max where it was, and _gameskill's regen - a ratio
			// of self.maxhealth, set back through setNormalHealth, which scales by the session's - pulled his health
			// down to it every tick: with Juggernog his 50 were gone, without it he sat at 100 (two hits)
			if ( self.maxhealth != target )
				self SetMaxHealth( target );
			self.leon_max = target;
		}
		wait 0.25;
	}
}

// Ada's discount: each purchase (a gun, the box, a perk, a door, Pack-a-Punch, a trap - _zombiemode_score's
// minus_to_player_score, the price taken whole: the triggers check it whole first) gives her 15% of it back at once,
// in tens as the prices are; not the down penalty (it notifies I_am_down first), nor while she is down or out
ada_discount()
{
	self endon( "disconnect" );
	self thread ada_discount_downs();
	last = self.score;
	for ( ;; )
	{
		wait 0.05;
		if ( !isDefined( self.score ) )
			continue;
		spent = 0;
		if ( isDefined( last ) )
			spent = last - self.score;
		penalty = isDefined( self.leon_no_refund_until ) && getTime() < self.leon_no_refund_until;
		if ( spent > 0 && !penalty && self current() == "ada" && self.sessionstate == "playing"
			&& !isDefined( self.revivetrigger ) )
		{
			refund = int( spent * 0.15 / 10 ) * 10;
			if ( refund > 0 )
			{
				self maps\_zombiemode_score::add_to_player_score( refund, false ); // (not toward the drops' total)
				logPrint( "leon: ada's discount " + refund + " of " + spent + "\n" );
			}
		}
		last = self.score;
	}
}

ada_discount_downs()
{
	self endon( "disconnect" );
	for ( ;; )
	{
		self waittill( "I_am_down" );
		self.leon_no_refund_until = getTime() + 500;
	}
}

// Luis's blasts: around each of his grenades' and projectiles' explosions, the zombies out to 360" take a second
// blast's damage (more the nearer), his own (scored as his)
luis_blasts( note )
{
	self endon( "disconnect" );
	for ( ;; )
	{
		self waittill( note, projectile, weapon );
		if ( isDefined( projectile ) && self current() == "luis" )
			projectile thread luis_blast( self );
	}
}

luis_blast( owner )
{
	self waittill( "explode", position );
	if ( !isDefined( owner ) )
		return;
	zombies = getAiArray( "axis" );
	for ( i = 0; i < zombies.size; i++ )
	{
		if ( !isAlive( zombies[i] ) )
			continue;
		d = distance( zombies[i].origin, position );
		if ( d > 360 )
			continue;
		zombies[i] DoDamage( int( 250 * ( 1 - d / 360 ) ) + 50, position, owner, -1, "grenadesplash" );
	}
}

// The level's damage and kill overrides (_zombiemode's), wrapped once they are set: Luis's damage, Ada's drops
perks_init()
{
	while ( !isDefined( level.overrideActorDamage ) || !isDefined( level.overrideActorKilled ) )
		wait 0.1;
	level.leon_actor_damage = level.overrideActorDamage;
	level.overrideActorDamage = ::actor_damage;
	level.leon_actor_killed = level.overrideActorKilled;
	level.overrideActorKilled = ::actor_killed;
}

actor_damage( inflictor, attacker, damage, flags, meansofdeath, weapon, vpoint, vdir, sHitLoc, modelIndex, psOffsetTime )
{
	damage = self [[ level.leon_actor_damage ]]( inflictor, attacker, damage, flags, meansofdeath, weapon, vpoint, vdir,
		sHitLoc, modelIndex, psOffsetTime );
	if ( isDefined( damage ) && isDefined( attacker ) && isPlayer( attacker ) && attacker current() == "luis" )
		damage = int( damage * 1.25 );
	return damage;
}

actor_killed( eInflictor, attacker, iDamage, sMeansOfDeath, sWeapon, vDir, sHitLoc, psOffsetTime )
{
	self [[ level.leon_actor_killed ]]( eInflictor, attacker, iDamage, sMeansOfDeath, sWeapon, vDir, sHitLoc, psOffsetTime );
	// Ada's kill: one more of _zombiemode's own drop rolls (its 3%, its round's cap)
	if ( isDefined( attacker ) && isPlayer( attacker ) && attacker current() == "ada" && isDefined( self.origin ) )
		level thread maps\_zombiemode_powerups::powerup_drop( self.origin );
}

display_name( name )
{
	switch ( name )
	{
	case "ada":
		return "Ada";
	case "hunk":
		return "HUNK";
	case "luis":
		return "Luis";
	}
	return "Leon";
}

// self: the player. The match's first seconds: the knife goes to the next character (the prompt stays a while after
// each switch).
choose()
{
	self endon( "disconnect" );
	if ( level.leon_characters.size < 2 )
		return;
	hud = newClientHudElem( self );
	hud.alignX = "center";
	hud.alignY = "middle";
	hud.horzAlign = "center";
	hud.vertAlign = "middle";
	hud.y = 110;
	hud.fontScale = 1.5;
	hud.foreground = true;
	hud.hidewheninmenu = true;
	// and under it the character's perk (perks)
	line = newClientHudElem( self );
	line.alignX = "center";
	line.alignY = "middle";
	line.horzAlign = "center";
	line.vertAlign = "middle";
	line.y = 132;
	line.fontScale = 1.2;
	line.foreground = true;
	line.hidewheninmenu = true;
	until = getTime() + 15000;
	shown = "";
	shownPerk = "";
	was = true; // (a press already held at the spawn is none)
	// his weapon switch (weapnext: the mouse wheel, the pad's Y) presses the change-seat button meanwhile, not the weapon
	// (the engine's CG_NextWeapon_f reads leon_charprompt on his client)
	self setClientDvar( "leon_charprompt", "1" );
	while ( getTime() < until )
	{
		pressed = self changeSeatButtonPressed();
		if ( pressed && !was )
		{
			name = self current();
			next = level.leon_characters[0];
			for ( i = 0; i < level.leon_characters.size - 1; i++ )
			{
				if ( level.leon_characters[i] == name )
					next = level.leon_characters[ i + 1 ];
			}
			self apply( next );
			if ( until < getTime() + 6000 )
				until = getTime() + 6000;
		}
		was = pressed;
		text = "Press SWITCH WEAPON to change character"; // (no name: his model shows who he is)
		if ( text != shown )
		{
			hud setText( text );
			shown = text;
		}
		perk = perk_line( self current() );
		if ( perk != shownPerk )
		{
			line setText( perk );
			shownPerk = perk;
		}
		wait 0.05;
	}
	self setClientDvar( "leon_charprompt", "0" );
	hud fadeOverTime( 0.5 );
	hud.alpha = 0;
	line fadeOverTime( 0.5 );
	line.alpha = 0;
	wait 0.5;
	hud destroy();
	line destroy();
}

// the character's perk in a line (the prompt's second: choose)
perk_line( name )
{
	switch ( name )
	{
	case "ada":
		return "Ada: 15% discount, more power-ups, stronger kick";
	case "hunk":
		return "HUNK: faster aim, reload, weapon switch and grenades";
	case "luis":
		return "Luis: more damage, bigger explosions";
	}
	return "Leon: staggers zombies more easily, takes an extra hit, a little faster";
}
