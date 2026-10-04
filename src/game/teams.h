#pragma once

// This file is inspired by Jedi Academy, unsure if it's accurate to COD4(BLOPS)
// Basically just to hold the `team_t` in a file that can be easily included without recursion

enum team_t : __int32
{                                                                             // XREF: clientState_s/r
                                                                                // sentient_s/r ...
        TEAM_FREE                 = 0x0,
        TEAM_BAD                    = 0x0,
        TEAM_AXIS                 = 0x1,
        TEAM_ALLIES             = 0x2,
        TEAM_SPECTATOR        = 0x3,
        TEAM_NUM_TEAMS        = 0x4,
        TEAM_LOCALPLAYERS = 0x5,
};

enum ffa_team_t : __int32
{                                                                             // XREF: clientState_s/r
                                                                                // clientInfo_t/r ...
        TEAM_FFA_NONE     = 0x0,
        TEAM_FFA_AXIS     = 0x1,
        TEAM_FFA_ALLIES = 0x2,
};