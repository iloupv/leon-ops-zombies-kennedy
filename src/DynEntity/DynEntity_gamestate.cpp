#include "DynEntity_gamestate.h"
#include "DynEntity_coll.h"
#include "DynEntity_client.h"
#include "DynEntity_load_obj.h"
#include <qcommon/cm_load.h>
#include <win32/win_common.h>
#include <qcommon/common.h>
#include <client/splitscreen.h>
#include <physics/phys_main.h>

void __cdecl DynEnt_WriteGameState(msg_t *msg)
{
    char *v1; // eax
    float tvel[3]; // [esp+8h] [ebp-3Ch] BYREF
    float avel[3]; // [esp+14h] [ebp-30h] BYREF
    unsigned __int16 absDynEntId; // [esp+20h] [ebp-24h]
    unsigned int drawType; // [esp+24h] [ebp-20h]
    DynEntityCollType collType; // [esp+28h] [ebp-1Ch]
    DynEntityPose *dynEntClientPose; // [esp+2Ch] [ebp-18h]
    DynEntityClient *dynEntClient; // [esp+30h] [ebp-14h]
    unsigned int sizeAtStart; // [esp+34h] [ebp-10h]
    const DynEntityDef *dynEntDef; // [esp+38h] [ebp-Ch]
    unsigned __int16 dynEntCount; // [esp+3Ch] [ebp-8h]
    unsigned __int16 dynEntId; // [esp+40h] [ebp-4h]

    sizeAtStart = msg->cursize;
    for ( drawType = 0; drawType < 2; ++drawType )
    {
        collType = (DynEntityCollType)drawType;
        dynEntCount = DynEnt_GetEntityCount((DynEntityCollType)drawType);
        if ( !drawType )
            dynEntCount = cm.originalDynEntCount;
        for ( dynEntId = 0; dynEntId < (int)dynEntCount; ++dynEntId )
        {
            dynEntDef = DynEnt_GetEntityDef(dynEntId, (DynEntityDrawType)drawType);
            dynEntClient = DynEnt_GetClientEntity(dynEntId, (DynEntityDrawType)drawType);
            absDynEntId = DynEnt_GetClientAbsId(dynEntId, (DynEntityDrawType)drawType);
            dynEntClientPose = DynEnt_GetClientPose(dynEntId, (DynEntityDrawType)drawType);
            if ( (dynEntClient->flags & 1) != 0
                && !VecNCompareCustomEpsilon(dynEntDef->pose.origin, dynEntClientPose->pose.origin, 0.0099999998, 3) )
            {
                MSG_WriteShort(msg, absDynEntId);
                MSG_WritePosition(msg, dynEntClientPose->pose.origin);
                MSG_WriteQuat(msg, dynEntClientPose->pose.quat);
                Sys_EnterCriticalSection(CRITSECT_PHYSICS);
                if ( dynEntClient->physObjId )
                {
                    MSG_WriteBit1(msg);
                    Phys_ObjGetVelocities(dynEntClient->physObjId, tvel, avel);
                    MSG_WritePosition(msg, tvel);
                    MSG_WritePosition(msg, avel);
                }
                else
                {
                    MSG_WriteBit0(msg);
                }
                Sys_LeaveCriticalSection(CRITSECT_PHYSICS);
            }
        }
    }
    MSG_WriteShort(msg, -1);
    v1 = va("--\nDynEnt_WriteGameState wrote %i bytes\n--\n", msg->cursize - sizeAtStart);
    Com_PrintMessage(0, v1, 0);
}

void __cdecl MSG_WritePosition(msg_t *msg, float *origin)
{
    MSG_WriteBits(msg, *(unsigned int *)origin, 0x20u);
    MSG_WriteBits(msg, *((unsigned int *)origin + 1), 0x20u);
    MSG_WriteBits(msg, *((unsigned int *)origin + 2), 0x20u);
}

void __cdecl MSG_WriteQuat(msg_t *msg, float *quat)
{
    MSG_WriteByte(msg, (int)(float)(*quat * 128.0) + 0x80);
    MSG_WriteByte(msg, (int)(float)(quat[1] * 128.0) + 0x80);
    MSG_WriteByte(msg, (int)(float)(quat[2] * 128.0) + 0x80);
    MSG_WriteByte(msg, (int)(float)(quat[3] * 128.0) + 0x80);
}

void __cdecl DynEnt_ReadGameState(msg_t *msg, int localClientNum)
{
    int PhysObj; // eax
    DynEntityPose *dynEntPose; // [esp+4h] [ebp-50h]
    DynEntityDrawType drawType; // [esp+8h] [ebp-4Ch] BYREF
    DynEntityClient *dynEntClient; // [esp+Ch] [ebp-48h]
    const DynEntityDef *dynEntDef; // [esp+10h] [ebp-44h]
    unsigned __int16 id; // [esp+14h] [ebp-40h] BYREF
    float origin[3]; // [esp+18h] [ebp-3Ch] BYREF
    float quat[4]; // [esp+24h] [ebp-30h] BYREF
    float tvel[3]; // [esp+34h] [ebp-20h] BYREF
    float avel[3]; // [esp+40h] [ebp-14h] BYREF
    int inPhysics; // [esp+4Ch] [ebp-8h]
    unsigned __int16 absDynEntId; // [esp+50h] [ebp-4h]
    int savedregs; // [esp+54h] [ebp+0h] BYREF

    for ( absDynEntId = MSG_ReadShort(msg); absDynEntId != 0xFFFF; absDynEntId = MSG_ReadShort(msg) )
    {
        MSG_ReadPosition(msg, origin);
        MSG_ReadQuat(msg, quat);
        inPhysics = MSG_ReadBit(msg);
        if ( inPhysics )
        {
            MSG_ReadPosition(msg, tvel);
            MSG_ReadPosition(msg, avel);
        }
        if ( CL_LocalClient_IsFirstActive(localClientNum) )
        {
            dynEntClient = DynEnt_GetClientEntity(absDynEntId);
            dynEntDef = DynEnt_GetEntityDef(absDynEntId);
            DynEnt_GetClientIdDrawType(absDynEntId, &id, &drawType);
            dynEntPose = DynEnt_GetClientPose(id, drawType);
            dynEntPose->pose.origin[0] = origin[0];
            dynEntPose->pose.origin[1] = origin[1];
            dynEntPose->pose.origin[2] = origin[2];
            dynEntPose->pose.quat[0] = quat[0];
            dynEntPose->pose.quat[1] = quat[1];
            dynEntPose->pose.quat[2] = quat[2];
            dynEntPose->pose.quat[3] = quat[3];
            if ( inPhysics )
            {
                if ( !dynEntClient->physObjId )
                {
                    PhysObj = DynEntCl_CreatePhysObj(dynEntDef, dynEntClient, &dynEntPose->pose);
                    dynEntClient->physObjId = PhysObj;
                }
                if ( dynEntClient->physObjId )
                {
                    Phys_ObjSetVelocity(dynEntClient->physObjId, tvel);
                    Phys_ObjSetAngularVelocity(dynEntClient->physObjId, avel);
                }
            }
        }
    }
}

void __cdecl MSG_ReadPosition(msg_t *msg, float *origin)
{
    *(unsigned int *)origin = MSG_ReadBits(msg, 0x20u);
    *((unsigned int *)origin + 1) = MSG_ReadBits(msg, 0x20u);
    *((unsigned int *)origin + 2) = MSG_ReadBits(msg, 0x20u);
}

void __cdecl MSG_ReadQuat(msg_t *msg, float *quat)
{
    *quat = (float)(MSG_ReadByte(msg) - 128) / 128.0;
    quat[1] = (float)(MSG_ReadByte(msg) - 128) / 128.0;
    quat[2] = (float)(MSG_ReadByte(msg) - 128) / 128.0;
    quat[3] = (float)(MSG_ReadByte(msg) - 128) / 128.0;
    Vec4Normalize(quat);
}
