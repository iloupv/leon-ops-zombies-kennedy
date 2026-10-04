#include "g_actor_prone.h"
#include "actor.h"
#include <universal/assertive.h>

void __cdecl G_InitActorProneInfo(actor_s *actor)
{
    if ( !actor && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\g_actor_prone.cpp", 65, 0, "%s", "actor") )
        __debugbreak();
    *(unsigned int *)&actor->ProneInfo.bCorpseOrientation = 0;
    actor->ProneInfo.iProneTime = 0;
    actor->ProneInfo.iProneTrans = 0;
    actor->ProneInfo.fBodyHeight = 0.0;
    actor->ProneInfo.fTorsoPitch = 0.0;
    actor->ProneInfo.fWaistPitch = 0.0;
}

