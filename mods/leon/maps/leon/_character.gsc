// LEON MOD - not part of the original game. The character the player plays: Leon, or Ada, HUNK or Luis - each a
// runtime model the mod's setup converts from the player's own RE4 (rtmodels/<name>.rlm; leon.cfg's bo1_rtmodels lists
// them, and the engine's bo1_rtmodels_loaded those this install has). They all play RE4's player motions (Leon's for
// what a character has none of), the kick included, and say their own pain, death and kick (the engine's CG_LeonVoice
// reads leon_character). At the start of a match "Press KNIFE to switch character" goes through them; the pick is kept
// (leon_character is saved in the player's config).

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

// the character picked last (leon_character), when this install has it; else Leon
current()
{
	name = getDvar( "leon_character" );
	for ( i = 0; i < level.leon_characters.size; i++ )
	{
		if ( level.leon_characters[i] == name )
			return name;
	}
	return level.leon_characters[0];
}

// self: the player, as the character
apply( name )
{
	setDvar( "leon_character", name );
	self detachAll();
	self setModel( name );
	logPrint( "leon: character " + name + "\n" );
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
	was = true; // (a knife already held at the spawn is no press)
	while ( getTime() < until )
	{
		pressed = self meleeButtonPressed();
		if ( pressed && !was )
		{
			name = current();
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
		text = "Press KNIFE to switch character: " + display_name( current() );
		if ( text != shown )
		{
			hud setText( text );
			shown = text;
		}
		wait 0.05;
	}
	hud fadeOverTime( 0.5 );
	hud.alpha = 0;
	wait 0.5;
	hud destroy();
}
