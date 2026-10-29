#pragma once

// mod (the Leon mod): RE4's HUD - the life meter, the rounds, the character's name - in place of BO1's ammo counter
// (leon_re4hud; cg_re4hud_mp.cpp)
void __cdecl CG_DrawRE4Hud(int localClientNum);
// whether it is drawn in place of BO1's ammo counter (CG_OwnerDraw's clip, stock and low ammo)
bool __cdecl CG_RE4HudHidesAmmoCounter();
