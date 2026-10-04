#include "g_team_mp.h"
#include "g_main_mp.h"

bool __cdecl OnSameTeam(gentity_s *ent1, gentity_s *ent2)
{
    if ( !ent1->client || !ent2->client )
        return 0;
    if ( ent1->client->sess.cs.team )
        return ent1->client->sess.cs.team == ent2->client->sess.cs.team;
    return 0;
}

void __cdecl TeamplayInfoMessage(gentity_s *ent)
{
    ent->client->ps.stats[3] = -1;
}

void __cdecl CheckTeamStatus()
{
    gentity_s *ent; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    if ( level.time - level.lastTeammateHealthTime > 0 )
    {
        level.lastTeammateHealthTime = level.time;
        for ( i = 0; i < com_maxclients->current.integer; ++i )
        {
            ent = &g_entities[i];
            if ( ent->r.inuse )
            {
                if ( (ent->client->ps.otherFlags & 2) == 0 )
                    TeamplayInfoMessage(ent);
            }
        }
    }
}

