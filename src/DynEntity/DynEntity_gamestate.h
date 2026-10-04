#pragma once

#include <qcommon/msg_mp.h>

enum DynEntityDrawType : __int32
{                                       // XREF: ?Flame_Impact_Process@@YAX_NPAUflameGeneric_s@@PAUtrace_t@@PAHH@Z/r
                                        // Rope_ApplyConstraint/r ...
    DYNENT_DRAW_MODEL = 0x0,
    DYNENT_DRAW_BRUSH = 0x1,
    DYNENT_DRAW_COUNT = 0x2,
};
inline DynEntityDrawType &operator++(DynEntityDrawType &t)
{
    t = static_cast<DynEntityDrawType>((static_cast<int>(t) + 1));
    return t;
}
inline DynEntityDrawType operator++(DynEntityDrawType &t, int)
{
    DynEntityDrawType old = t;
    t = static_cast<DynEntityDrawType>((static_cast<int>(t) + 1));
    return old;
}

struct DynEntityAreaParms // sizeof=0x14
{                                       // XREF: ?DynEntSv_AreaEntities@@YAGW4DynEntityDrawType@@QBM1HGPAG@Z/r
    const float *mins;                  // XREF: DynEntCl_AreaEntities(DynEntityDrawType,float const * const,float const * const,int,ushort,ushort *)+C0/w
    const float *maxs;                  // XREF: DynEntCl_AreaEntities(DynEntityDrawType,float const * const,float const * const,int,ushort,ushort *)+C6/w
    int contentMask;                    // XREF: DynEntCl_AreaEntities(DynEntityDrawType,float const * const,float const * const,int,ushort,ushort *)+CC/w
    unsigned __int16 *list;             // XREF: DynEntCl_AreaEntities(DynEntityDrawType,float const * const,float const * const,int,ushort,ushort *)+D2/w
    unsigned __int16 maxCount;          // XREF: DynEntCl_AreaEntities(DynEntityDrawType,float const * const,float const * const,int,ushort,ushort *)+D9/w
    unsigned __int16 count;             // XREF: DynEntCl_AreaEntities(DynEntityDrawType,float const * const,float const * const,int,ushort,ushort *)+DF/w
};

void __cdecl DynEnt_WriteGameState(msg_t *msg);
void __cdecl MSG_WritePosition(msg_t *msg, float *origin);
void __cdecl MSG_WriteQuat(msg_t *msg, float *quat);
void __cdecl DynEnt_ReadGameState(msg_t *msg, int localClientNum);
void __cdecl MSG_ReadPosition(msg_t *msg, float *origin);
void __cdecl MSG_ReadQuat(msg_t *msg, float *quat);
