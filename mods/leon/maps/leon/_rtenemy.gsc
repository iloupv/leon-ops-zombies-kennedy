#include maps\_utility;
#include common_scripts\utility;
#include maps\_zombiemode_utility;

#using_animtree( "generic_human" );

// LEON MOD - not part of the original game. What the RE4 enemies share (maps\leon\_salvador.gsc's Super Salvador,
// maps\leon\_verdugo.gsc's Verdugo). A zombie of the map's own spawners carries one's pathing, his hit locations and the
// round's count - hidden (zombie mode's hide only stops it being drawn: its hits and paths stay), its melee and its moans
// off - and his runtime model, built by the mod's setup from the player's own RE4 (rtmodels/<name>.rlm:
// tools/leon-converter/leon_rlm.py ENEMIES), is linked on it: the engine plays his idle, walk and run by how it moves,
// and each motion started here on every player's client (leon_rtactions: cgame_mp/cg_pose_mp.cpp). Through a motion his
// zombie is held where it stands and moved along the motion's root travel (RE4 moves the whole model by it; the engine
// plays the motion in place). Head and leg shots stagger him (maps\leon\_kick.gsc: leon_boss_stagger) with no halo: he
// is never kicked.

init()
{
	level.leon_rtactions = [];
}

// the setup built it (bo1_rtmodels_loaded: the runtime models this level has)
built( name )
{
	loaded = strTok( getDvar( "bo1_rtmodels_loaded" ), " " );
	for ( i = 0; i < loaded.size; i++ )
	{
		if ( loaded[i] == name )
			return true;
	}
	return false;
}

// self: a player coming in - the motions playing now
player_init()
{
	self setClientDvar( "leon_rtactions", rtactions_text() );
}

// a valid player at random, or undefined
target()
{
	players = get_players();
	valid = [];
	for ( i = 0; i < players.size; i++ )
	{
		if ( is_player_valid( players[i] ) )
			valid[ valid.size ] = players[i];
	}
	if ( valid.size == 0 )
		return undefined;
	return valid[ randomInt( valid.size ) ];
}

// His zombie at origin, facing yaw, carrying modelName's model (ai.leon_model), with health and his stagger
// (staggerFunc, self: his zombie), or undefined: one of the map's zombie spawners spawns it as a chaser (no window to
// come through: _zombiemode_spawner should_skip_teardown) from origin, and is put back as it was.
spawn_enemy( origin, yaw, modelName, health, staggerFunc )
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
	ai forceTeleport( origin, ( 0, yaw, 0 ) );
	ai hide();
	ai.no_eye_glow = true; // (_zombiemode_spawner delayed_zombie_eye_glow, half a second on)
	ai.no_gib = 1; // (no limbs of the hidden zombie flying)
	ai.shrinked = true; // (_zombiemode_audio do_zombies_playvocals plays nothing for him: his voice is RE4's)
	ai.disableMelee = true; // (animscripts\zombie_melee CanMeleeInternal: his attacks are this script's)
	// no pain on his zombie (animscripts\zombie_pain): a hit stood it still through a stumble, and the Thundergun's
	// knockdown (playThundergunPainAnim: its fall and getting up) held him in place, his model idle, for seconds
	ai disable_pain();
	ai.leon_boss = true;
	ai.leon_boss_stagger = staggerFunc;
	ai.custom_damage_func = ::hit_damage;
	ai.thundergun_fling_func = ::flung;
	ai.thundergun_disintegrate_func = ::flung;
	ai.thundergun_knockdown_func = ::knocked;
	ai.maxhealth = health;
	ai.health = health;
	model = spawn( "script_model", ai.origin );
	model.angles = ai.angles;
	model setModel( modelName );
	model linkTo( ai );
	ai.leon_model = model;
	ai thread hidden();
	return ai;
}

// (a zombie's scripts may show it again: its rise, its entrance)
hidden()
{
	self endon( "death" );
	for ( ;; )
	{
		self hide();
		wait 0.5;
	}
}

// a player he can reach (self: his zombie), or undefined: within dist of him, 60 in height, cosMin of his facing
in_reach( dist, cosMin )
{
	forward = anglesToForward( ( 0, self.angles[1], 0 ) );
	players = get_players();
	for ( i = 0; i < players.size; i++ )
	{
		if ( !is_player_valid( players[i] ) )
			continue;
		offset = players[i].origin - self.origin;
		flat = ( offset[0], offset[1], 0 );
		d = length( flat );
		if ( d > dist || abs( offset[2] ) > 60 )
			continue;
		if ( d > 1 && vectorDot( forward, vectorNormalize( flat ) ) < cosMin )
			continue;
		return players[i];
	}
	return undefined;
}

// His hit on player (self: his zombie): damage, or lethal - down at once, whatever the player's health: through the
// player's death invulnerability after a hit, as _zombiemode's own failsafe kill (it held off a hit on a player hurt a
// moment before), and through a kick's invulnerability (not the invulnerable tests': level.leon_boss_testgod). Solo with
// no Quick Revive's life the game ends: _zombiemode's damage then returns 0, his health shown unchanged.
hit( player, damage, lethal )
{
	if ( lethal )
	{
		if ( !isDefined( level.leon_boss_testgod ) )
			player disableInvulnerability();
		damage = player.health + 1000;
		setSavedDvar( "player_deathInvulnerableTime", 0 );
	}
	self.leon_damage = damage;
	player DoDamage( damage, self.origin, self );
	self.leon_damage = undefined;
	if ( lethal && isDefined( level.startInvulnerableTime ) )
		setSavedDvar( "player_deathInvulnerableTime", level.startInvulnerableTime );
}

// _zombiemode player_damage_override's damage from him (self: his zombie): his hit's (hit); nothing else (his zombie's
// own - none: its melee is off)
hit_damage( player )
{
	if ( isDefined( self.leon_damage ) )
		return self.leon_damage;
	return 0;
}

// the Thundergun's blast on him (_zombiemode_weap_thundergun: in place of its fling, its knockdown and its
// disintegration of a zombie): a share of his health, and his own stagger (spawn_enemy's staggerFunc)
flung( player )
{
	self DoDamage( int( self.maxhealth * 0.2 ), player.origin, player );
	if ( isAlive( self ) && isDefined( self.leon_boss_stagger ) )
		self thread [[ self.leon_boss_stagger ]]();
}

knocked( player, gib )
{
	self DoDamage( int( self.maxhealth * 0.1 ), player.origin, player );
	if ( isAlive( self ) && isDefined( self.leon_boss_stagger ) )
		self thread [[ self.leon_boss_stagger ]]();
}

// his zombie held where it stands, turned to yaw, while a motion plays on him (his own path off) - till free
hold( yaw )
{
	self.leon_hold_yaw = yaw;
	self animcustom( ::hold_anim );
}

hold_anim()
{
	self endon( "death" );
	self AnimMode( "zonly_physics" );
	self OrientMode( "face angle", self.leon_hold_yaw );
	self ClearAnim( %body, 0.2 );
	self SetFlaggedAnimKnobAllRestart( "leon_hold", %ai_zombie_idle_v1, %body, 1, 0.2, 1 );
	self waittill( "leon_free" );
}

// his motion over (self: his zombie): his path back, his model back on its cycles
free()
{
	self.leon_busy = undefined;
	self notify( "leon_free" );
	self.leon_model motion_end();
}

// his zombie moved along a motion's root travel: curve, inches along yaw every 0.1 s (3 of RE4's frames, sampled from
// the motion), stopping at whatever it cannot move into; till free, or an interruption (leon_interrupt: his flinch)
travel( curve, yaw )
{
	self endon( "death" );
	self endon( "leon_free" );
	self endon( "leon_interrupt" );
	curve = strTok( curve, " " );
	dir = anglesToForward( ( 0, yaw, 0 ) );
	done = 0;
	last = curve.size - 1;
	for ( step = 1; step <= last * 2; step++ )
	{
		wait 0.05;
		k = step * 0.5;
		lo = int( k );
		hi = lo + 1;
		if ( hi > last )
			hi = last;
		d = int( curve[lo] ) + ( int( curve[hi] ) - int( curve[lo] ) ) * ( k - lo );
		move = d - done;
		if ( abs( move ) < 0.5 )
			continue;
		next = self.origin + dir * move;
		if ( !self mayMoveToPoint( next ) )
			return;
		self forceTeleport( next, ( 0, yaw, 0 ) );
		done = d;
	}
}

// self: his model - left where he fell, his fall played (deathSound with it) and its last frame held 10 s, then gone
// (his zombie's death or its removal); gone at once when he left (leon_left: his zombie removed alive)
corpse( ai, deathSound )
{
	while ( isDefined( ai ) && isAlive( ai ) )
		wait 0.05;
	if ( isDefined( self.leon_left ) )
	{
		self motion_end();
		self delete();
		return;
	}
	self unlink();
	self motion( "death" );
	self playSound( deathSound );
	wait 10;
	self motion_end();
	self delete();
}

// self: his model - the motion it plays from now on every player's client (leon_rtactions: "<entity> <motion> <ms>")
motion( name )
{
	level.leon_rtactions[ "" + self getEntityNumber() ] = name + " " + getTime();
	rtactions_publish();
}

motion_end()
{
	level.leon_rtactions[ "" + self getEntityNumber() ] = undefined;
	rtactions_publish();
}

rtactions_text()
{
	text = "";
	keys = getArrayKeys( level.leon_rtactions );
	for ( i = 0; i < keys.size; i++ )
		text = text + keys[i] + " " + level.leon_rtactions[ keys[i] ] + " ";
	return text;
}

rtactions_publish()
{
	text = rtactions_text();
	players = getEntArray( "player", "classname" );
	for ( i = 0; i < players.size; i++ )
		players[i] setClientDvar( "leon_rtactions", text );
}
