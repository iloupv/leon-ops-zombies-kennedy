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
	until = getTime() + 15000;
	shown = "";
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
		text = "Press SWITCH WEAPON to change character: " + display_name( self current() );
		if ( text != shown )
		{
			hud setText( text );
			shown = text;
		}
		wait 0.05;
	}
	self setClientDvar( "leon_charprompt", "0" );
	hud fadeOverTime( 0.5 );
	hud.alpha = 0;
	wait 0.5;
	hud destroy();
}
