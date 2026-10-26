#pragma once

// mod (the Leon mod): its kick's state per player, in two playerState eFlags2 bits the engine leaves unused - networked
// to the player for his own prediction, and on his entity to the others (BG_PlayerToEntitySetFlags copies eFlags2).
// The script sets them (maps\leon\_kick.gsc: <player> leonsetkickflags( ready, kicking )); the server's pmove, the
// player's prediction and his use key all read his own.
#define LEON_EF2_KICK_READY 0x20000u  // the kick offered: the use key kicks (and does nothing else), no reload
#define LEON_EF2_KICKING 0x80000000u  // the kick under way: no aim, shot, knife or reload
#define LEON_EF2_KICK2 0x4000u        // this kick is the character's second roundhouse (the others' clients pose it)
// On his entity (what the other players see: BG_PlayerToEntitySetFlags) the kick's offer, his own business, is in its
// place whether he is down - BO1's last stand (pm_type 6 / 7) - so their clients lay his model down in RE4's death
#define LEON_EF2_ENT_DOWN LEON_EF2_KICK_READY
// HUNK's handling, his character's perk (maps\leon\_character.gsc: <player> leonsethandy( on )): he aims, reloads,
// switches guns and throws grenades faster, as with fast ADS / reload / switch (BG_LeonHandy) - not his bought perks,
// so Speed Cola is still his to buy and a down takes nothing of it
#define LEON_EF2_HANDY 0x2000u
#define BG_LeonHandy(ps) ((((unsigned int)(ps)->eFlags2) & LEON_EF2_HANDY) != 0)
// his times' multiplier where he has not bought the perk (the bought ones' is theirs: 0.5, twice as fast) - milder
// than a perk's, so Speed Cola is still worth his points
#define LEON_HANDY_RATE 0.7f
