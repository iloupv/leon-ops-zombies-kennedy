#include "rope_gamestate.h"
#include <game_mp/g_main_mp.h>
#include <qcommon/cm_load.h>
#include <DynEntity/DynEntity_client.h>
#include "physconstraints_load_obj.h"

int __cdecl GetEntityIndex(const unsigned __int16 *targetname)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < G_EntEnd(); i = G_EntNext(i) )
    {
        if ( g_entities[i].targetname == *targetname )
            return i;
    }
    return -1;
}

void __cdecl Rope_WriteGameState(msg_t *msg)
{
    int c; // [esp+0h] [ebp-10h]
    int entnum; // [esp+4h] [ebp-Ch]
    PhysConstraint *constraint; // [esp+8h] [ebp-8h]
    int i; // [esp+Ch] [ebp-4h]

    for ( i = 0; i < cm.num_constraints; ++i )
    {
        constraint = &cm.constraints[i];
        if ( constraint->type == CONSTRAINT_ROPE )
        {
            if ( constraint->attach_point_type1 == ATTACH_POINT_ENT )
            {
                entnum = GetEntityIndex(&constraint->target_ent1);
                MSG_WriteLong(msg, entnum);
            }
            if ( constraint->attach_point_type2 == ATTACH_POINT_ENT )
            {
                c = GetEntityIndex(&constraint->target_ent2);
                MSG_WriteLong(msg, c);
            }
        }
    }
}

void __cdecl Rope_ReadGameState(msg_t *msg, int localClientNum)
{
    PhysConstraint *constraint; // [esp+0h] [ebp-8h]
    int i; // [esp+4h] [ebp-4h]

    for ( i = 0; i < cm.num_constraints; ++i )
    {
        constraint = &cm.constraints[i];
        if ( constraint->type == CONSTRAINT_ROPE
            && (constraint->attach_point_type1 == ATTACH_POINT_ENT || constraint->attach_point_type2 == ATTACH_POINT_ENT) )
        {
            if ( constraint->attach_point_type1 == ATTACH_POINT_ENT )
                constraint->target_index1 = MSG_ReadLong(msg);
            if ( constraint->attach_point_type2 == ATTACH_POINT_ENT )
                constraint->target_index2 = MSG_ReadLong(msg);
        }
    }
    CreateRopes(localClientNum);
}

