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
