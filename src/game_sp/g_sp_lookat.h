#pragma once

struct gentity_s;

// zombies: SP 0x00418d00, the player look-at update (lookatent / crosshair flags), from ClientEndFrame.
void Player_UpdateLookAtEntity_SP(gentity_s *ent);
