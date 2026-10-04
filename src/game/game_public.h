#pragma once

struct AntilagClientStore // sizeof=0x320
{                                       // XREF: ?Weapon_Melee@@YAPAUgentity_s@@PAU1@PAUweaponParms@@MMMH@Z/r
                                        // ?G_RunMissile@@YAXPAUgentity_s@@@Z/r ...
    float realClientPositions[32][3];
    float realClientAngles[32][3];
    bool clientMoved[32];
};