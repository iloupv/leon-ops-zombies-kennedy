#include "actor_event_listeners.h"
#include <game_mp/g_main_mp.h>
#include <clientscript/cscr_vm.h>
#include <game_mp/g_spawn_mp.h>
#include <game_mp/g_misc_mp.h>
#include <clientscript/scr_const.h>

unsigned __int16 *g_AIEV_scrConst_table[28] =
{
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  &scr_const.explode,
  &scr_const.grenadedanger,
  &scr_const.grenadedanger,
  &scr_const.gunshot,
  &scr_const.silenced_shot,
  NULL,
  NULL,
  &scr_const.bulletwhizby,
  &scr_const.bulletwhizby,
  NULL,
  &scr_const.projectile_impact,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};



int g_listenerCount;
AIEventListener g_AIEVlisteners[32];
unsigned int array[63];

void __cdecl RemoveSwapWithLast(unsigned int listenerIndex)
{
    int removeIndex; // [esp+0h] [ebp-4h]

    if ( listenerIndex >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_event_listeners.cpp",
                    176,
                    0,
                    "listenerIndex doesn't index MAX_AI_EV_LISTENERS\n\t%i not in [0, %i)",
                    listenerIndex,
                    32) )
    {
        __debugbreak();
    }
    removeIndex = listenerIndex;
    if ( (int)listenerIndex < --g_listenerCount )
    {
        g_AIEVlisteners[listenerIndex].entIndex = g_AIEVlisteners[g_listenerCount].entIndex;
        array[2 * listenerIndex] = array[2 * g_listenerCount];
        removeIndex = g_listenerCount;
    }
    g_AIEVlisteners[removeIndex].entIndex = 1023;
    array[2 * removeIndex] = 0;
}

void __cdecl Actor_EventListener_RemoveEntity(int entIndex)
{
    signed int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < g_listenerCount; ++i )
    {
        if ( g_AIEVlisteners[i].entIndex == entIndex )
        {
            RemoveSwapWithLast(i);
            return;
        }
    }
}

int __cdecl Actor_EventListener_First(ai_event_t event, int teamFlags)
{
    return Actor_EventListener_Next(-1, event, teamFlags);
}

int __cdecl Actor_EventListener_Next(int index, ai_event_t event, int teamFlags)
{
    int i; // [esp+0h] [ebp-8h]
    sentient_s *sentient; // [esp+4h] [ebp-4h]

    for ( i = index + 1; i < g_listenerCount; ++i )
    {
        if (Com_BitCheckAssert(&array[2 * i], event, 0xFFFFFFF))
        {
            sentient = g_entities[g_AIEVlisteners[i].entIndex].sentient;
            if ( !sentient || (teamFlags & (1 << sentient->eTeam)) != 0 )
                return i;
        }
    }
    return -1;
}

gentity_s *__cdecl Actor_EventListener_GetEntity(unsigned int index)
{
    if ( index >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_event_listeners.cpp",
                    287,
                    0,
                    "index doesn't index MAX_AI_EV_LISTENERS\n\t%i not in [0, %i)",
                    index,
                    32) )
    {
        __debugbreak();
    }
    return &g_entities[g_AIEVlisteners[index].entIndex];
}

void __cdecl Actor_EventListener_NotifyToListener(
                gentity_s *listener,
                gentity_s *originator,
                ai_event_t event,
                float *position)
{
    if ( !listener
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_event_listeners.cpp", 302, 0, "%s", "listener") )
    {
        __debugbreak();
    }
    if ( listener->r.inuse )
    {
        Scr_AddVector(position, SCRIPTINSTANCE_SERVER);
        if ( originator )
            Scr_AddEntity(originator, SCRIPTINSTANCE_SERVER);
        else
            Scr_AddUndefined(SCRIPTINSTANCE_SERVER);
        if ( !g_AIEV_scrConst_table[event]
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\actor_event_listeners.cpp",
                        314,
                        0,
                        "%s",
                        "g_AIEV_scrConst_table[event]") )
        {
            __debugbreak();
        }
        Scr_Notify(listener, *g_AIEV_scrConst_table[event], 2u);
        ScrNotify_FaceEvent(listener, *g_AIEV_scrConst_table[event]);
    }
}

