#pragma once

// mod (the Leon mod): RE4's HUD - the life meter, the rounds, the character's name - in place of BO1's ammo counter
// (leon_re4hud; cg_re4hud_mp.cpp)
void __cdecl CG_DrawRE4Hud(int localClientNum);
// whether CG_OwnerDraw leaves this one out: BO1's own panel in the corner the RE4 HUD takes (its ammo counter, the
// blood and lines under it, the d-pad and its slots, the grenades', the weapon's name)
bool __cdecl CG_RE4HudHidesOwnerDraw(int ownerDraw);
// how far BO1's points box (owner-draw 288) is lifted over the ring, in virtual units (0: no RE4 HUD)
float __cdecl CG_RE4HudPointsLift();
