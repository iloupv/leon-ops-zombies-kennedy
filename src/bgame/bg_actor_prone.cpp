#include "bg_actor_prone.h"

int __cdecl BG_ActorIsProne(actor_prone_info_s *pInfo, int iCurrentTime)
{
    if ( !pInfo->prone || pInfo->bCorpseOrientation || pInfo->orientPitch )
        return 0;
    if ( !pInfo->iProneTrans )
        return 1;
    if ( pInfo->iProneTrans >= 0 )
    {
        if ( pInfo->iProneTrans + pInfo->iProneTime < iCurrentTime )
            pInfo->iProneTrans = 0;
    }
    else if ( pInfo->iProneTime - pInfo->iProneTrans < iCurrentTime )
    {
        pInfo->prone = 0;
        return 0;
    }
    return 1;
}

double __cdecl BG_GetActorProneFraction(actor_prone_info_s *pInfo, int iCurrentTime)
{
    if ( !pInfo->prone )
        return 0.0;
    if ( !pInfo->iProneTrans )
        return 1.0;
    if ( pInfo->iProneTrans >= 0 )
    {
        if ( pInfo->iProneTrans + pInfo->iProneTime >= iCurrentTime )
        {
            return (double)(iCurrentTime - pInfo->iProneTime) / (double)pInfo->iProneTrans;
        }
        else
        {
            pInfo->iProneTrans = 0;
            return 1.0;
        }
    }
    else if ( pInfo->iProneTime - pInfo->iProneTrans >= iCurrentTime )
    {
        return 1.0 - (double)(iCurrentTime - pInfo->iProneTime) / (double)-pInfo->iProneTrans;
    }
    else
    {
        pInfo->prone = 0;
        return 0.0;
    }
}

