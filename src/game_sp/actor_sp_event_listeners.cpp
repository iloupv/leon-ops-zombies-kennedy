#include "actor_sp_event_listeners.h"
#include <game/actor_event_listeners.h>
#include <game_mp/g_main_mp.h>
#include <game_mp/g_scr_main_mp.h>
#include <clientscript/cscr_vm.h>
#include <clientscript/cscr_stringlist.h>

// Use the storage consumed by KB's First/Next/Remove functions. The decompilation
// split the event masks out of AIEventListener::events into array[2 * index].
// G_FreeEntity and Actor_BecomeCorpse remove registrations; G_ShutdownGame frees
// the entities, so no second listener table or new lifecycle hook is needed.
extern unsigned __int16 *g_AIEV_scrConst_table[28];
extern int g_listenerCount;
extern AIEventListener g_AIEVlisteners[32];
extern unsigned int array[63];

// zombies: AI event name lookup (SP 0x007c0ca0, table 0x00b75058).
static int Actor_EventListener_EventForName(unsigned int eventName)
{
    for (int event = 0; event < 28; ++event)
    {
        if (g_AIEV_scrConst_table[event] && *g_AIEV_scrConst_table[event] == eventName)
            return event;
    }
    Scr_Error(va("Unable to find AI event for [%s]", SL_ConvertToString(eventName, SCRIPTINSTANCE_SERVER)), 0);
    return 0;
}

// zombies: addaieventlistener registration (SP 0x00504260).
void Actor_EventListener_Add(int entIndex, unsigned int eventName)
{
    int event = Actor_EventListener_EventForName(eventName);
    if (!event)
        return;
    for (int i = 0; i < g_listenerCount; ++i)
    {
        if (g_AIEVlisteners[i].entIndex == entIndex)
        {
            array[2 * i] |= 1u << event;
            return;
        }
    }
    if (g_listenerCount >= 32)
    {
        Scr_Error(va("Max listeners exceeded; entity id: %d\n", entIndex), 0);
        return;
    }
    g_AIEVlisteners[g_listenerCount].entIndex = entIndex;
    array[2 * g_listenerCount] |= 1u << event;
    ++g_listenerCount;
}

// zombies: addaieventlistener (SP 0x008053c0).
void G_m_addaieventlistener(scr_entref_t entref)
{
    gentity_s *ent = GetEntity(entref);
    Actor_EventListener_Add(ent->s.number, Scr_GetConstString(0, SCRIPTINSTANCE_SERVER));
}
