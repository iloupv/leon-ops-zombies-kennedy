#pragma once

// mod (the Leon mod): its kick's state per player, in two playerState eFlags2 bits the engine leaves unused - networked
// to the player for his own prediction, and on his entity to the others (BG_PlayerToEntitySetFlags copies eFlags2).
// The script sets them (maps\leon\_kick.gsc: <player> leonsetkickflags( ready, kicking )); the server's pmove, the
// player's prediction and his use key all read his own.
#define LEON_EF2_KICK_READY 0x20000u  // the kick offered: the use key kicks (and does nothing else), no reload
#define LEON_EF2_KICKING 0x80000000u  // the kick under way: no aim, shot, knife or reload
// On his entity (what the other players see: BG_PlayerToEntitySetFlags) the kick's offer, his own business, is in its
// place whether he is down - BO1's last stand (pm_type 6 / 7) - so their clients lay his model down in RE4's death
#define LEON_EF2_ENT_DOWN LEON_EF2_KICK_READY
// The character's second roundhouse (maps\leon\_kick.gsc's variant, for the others' clients to pose): the kick's two bits
// both set - on his player state the offer and the kick (never both else: the kick takes the offer), on his entity the
// down bit and the kick (a downed player's entity carries no kick: BG_PlayerToEntitySetFlags). No bit of its own: the
// high half has none free, and the low half is the scripts' client flags (each change runs their client_flag_callback;
// it sat on flag 14)
#define LEON_EF2_KICK2 (LEON_EF2_KICK_READY | LEON_EF2_KICKING)
#define BG_LeonKickOffered(flags2) ((((unsigned int)(flags2)) & LEON_EF2_KICK2) == LEON_EF2_KICK_READY)
#define BG_LeonKick2(flags2) ((((unsigned int)(flags2)) & LEON_EF2_KICK2) == LEON_EF2_KICK2)
#define BG_LeonEntDown(flags2) ((((unsigned int)(flags2)) & LEON_EF2_KICK2) == LEON_EF2_ENT_DOWN)
// HUNK's handling, his character's perk (maps\leon\_character.gsc: <player> leonsethandy( on )): he aims, reloads,
// switches guns and throws grenades faster, as with fast ADS / reload / switch (BG_LeonHandy) - not his bought perks,
// so Speed Cola is still his to buy and a down takes nothing of it. In his player state's otherFlags (sent to him only,
// 6 bits: 0x20 the one the engine leaves unused), not eFlags2's low half: the scripts' client flags (it sat on flag 13)
#define LEON_POF_HANDY 0x20
#define BG_LeonHandy(ps) (((ps)->otherFlags & LEON_POF_HANDY) != 0)
// his times' multiplier where he has not bought the perk (the bought ones' is theirs: 0.5, twice as fast) - milder
// than a perk's, so Speed Cola is still worth his points
#define LEON_HANDY_RATE 0.7f
// his run (PM_TankControls: the run key going forward, not aimed), his own and on his entity for the others: his model's
// cycle is the run's or else the walk's by it (R_RuntimeXModel_SetRunning) - by his speed alone a quick character's walk
// (Leon's, 26 % over) stayed past the run's cut, and from a run to a walk he played the run slowed down. In the one bit
// of the high half no player uses (the low half is the scripts' client flags: each change runs client_flag_callback)
#define LEON_EF2_RUNNING 0x80000u
// The knife's reach under RE4's controls (BG_PlayerMeleeRange / Width / Height: bg_misc.cpp): BO1's melee trace 16"
// longer (64 to 80) and its rays' spread half as wide again (10 to 15 either way, across and up / down)
#define LEON_MELEE_RANGE_ADD 16.0f
#define LEON_MELEE_SPREAD 1.5f
