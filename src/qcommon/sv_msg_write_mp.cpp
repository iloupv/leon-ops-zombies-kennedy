#include "sv_msg_write_mp.h"
#include "msg.h"
#include "sv_msg_write.h"

#include <cstring>
#include <server_mp/sv_snapshot_profile_mp.h>
#include <cgame/cg_draw_debug.h>
#include <server_mp/sv_init_mp.h>

int g_entsClientOnce;
int g_entsDeltaCompared;
int g_entsTransmitted;

bool __cdecl MSG_EntityIsLinked(const entityState_s *ent)
{
    return ent->clientLinkInfo.parentEnt != 0;
}

void __cdecl MSG_WriteGroundEntityNum(int clientNum, msg_t *msg, int groundEntityNum)
{
    int bits; // [esp+0h] [ebp-Ch]
    int value; // [esp+8h] [ebp-4h]

    if ( msg->readOnly
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\sv_msg_write_mp.cpp",
                    35,
                    0,
                    "%s",
                    "!msg->readOnly") )
    {
        __debugbreak();
    }
    if ( groundEntityNum == 1022 || (MSG_WriteBit0(msg), !groundEntityNum) )
    {
        MSG_WriteBit1(msg);
    }
    else
    {
        MSG_WriteBit0(msg);
        MSG_WriteBits(msg, groundEntityNum, g_entNumBits - 8); // mod (L25): networked entity numbers 1537+ (retail 2 bits)
        bits = 8;
        value = groundEntityNum >> (g_entNumBits - 8);
        while ( bits )
        {
            MSG_WriteByte(msg, value);
            value >>= 8;
            bits -= 8;
        }
    }
}

void __cdecl MSG_WriteDeltaField_Default(
                const SnapshotInfo_s *snapInfo,
                msg_t *msg,
                const NetField *field,
                const int *fromF,
                const int *toF)
{
    const char *v5; // eax

    if ( field->bits < -50 )
    {
        v5 = va("Missed a MSG_ case in MSG_WriteDeltaField - value is %i", field->bits);
        if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\sv_msg_write_mp.cpp", 82, 0, v5) )
            __debugbreak();
    }
    if ( MSG_GetField(toF, field->size) )
    {
        MSG_WriteBit1(msg);
        MSG_WriteValue(snapInfo, msg, fromF, toF, field->bits, field->size);
    }
    else
    {
        MSG_WriteBit0(msg);
    }
}

int __cdecl MSG_WriteDeltaStruct(
                SnapshotInfo_s *snapInfo,
                msg_t *msg,
                int time,
                unsigned __int8 *from,
                unsigned __int8 *to,
                int force,
                int numFields,
                int indexBits,
                const NetField *stateFields,
                int bChangeBit,
                int skippedFieldBits)
{
    const char *v11; // eax
    int startBits; // [esp+0h] [ebp-8h]
    int lc; // [esp+4h] [ebp-4h]

    startBits = MSG_GetUsedBitCount(msg);
    if ( msg->readOnly
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\sv_msg_write_mp.cpp",
                    122,
                    0,
                    "%s",
                    "!msg->readOnly") )
    {
        __debugbreak();
    }
    if ( !to && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\sv_msg_write_mp.cpp", 123, 0, "%s", "to") )
        __debugbreak();
    if ( *(unsigned int *)to >= (unsigned int)(1 << indexBits) )
    {
        v11 = va("to = %i, bits = %i", *(unsigned int *)to, indexBits);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\sv_msg_write_mp.cpp",
                        125,
                        0,
                        "%s\n\t%s",
                        "*reinterpret_cast< unsigned * >( to ) < (1u << indexBits)",
                        v11) )
            __debugbreak();
    }
    lc = MSG_GetLastChangedField(snapInfo, from, to, numFields, stateFields);
    if ( lc )
    {
        if ( bChangeBit )
            MSG_WriteBit1(msg);
        MSG_WriteEntityIndex(snapInfo, msg, *(unsigned int *)to, indexBits);
        MSG_WriteBit0(msg);
        MSG_WriteDeltaFields(snapInfo, msg, time, from, to, force, lc - 1, numFields, stateFields, skippedFieldBits, 0);
        return MSG_GetUsedBitCount(msg) - startBits;
    }
    else if ( force )
    {
        if ( bChangeBit )
            MSG_WriteBit1(msg);
        MSG_WriteEntityIndex(snapInfo, msg, *(unsigned int *)to, indexBits);
        MSG_WriteBit0(msg);
        MSG_WriteBit0(msg);
        return MSG_GetUsedBitCount(msg) - startBits;
    }
    else
    {
        return 0;
    }
}

int __cdecl MSG_WriteAppendedDeltaStruct(
                SnapshotInfo_s *snapInfo,
                msg_t *msg,
                int time,
                unsigned __int8 *from,
                unsigned __int8 *to,
                int force,
                int numFields,
                int indexBits,
                const NetField *stateFields,
                int bChangeBit,
                int skippedFieldBits)
{
    const char *v11; // eax
    int startBits; // [esp+0h] [ebp-8h]
    int lc; // [esp+4h] [ebp-4h]

    startBits = MSG_GetUsedBitCount(msg);
    if ( msg->readOnly
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\sv_msg_write_mp.cpp",
                    221,
                    0,
                    "%s",
                    "!msg->readOnly") )
    {
        __debugbreak();
    }
    if ( !to && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\sv_msg_write_mp.cpp", 222, 0, "%s", "to") )
        __debugbreak();
    if ( *(unsigned int *)to >= (unsigned int)(1 << indexBits) )
    {
        v11 = va("to = %i, bits = %i", *(unsigned int *)to, indexBits);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\sv_msg_write_mp.cpp",
                        224,
                        0,
                        "%s\n\t%s",
                        "*reinterpret_cast< unsigned * >( to ) < (1u << indexBits)",
                        v11) )
            __debugbreak();
    }
    lc = MSG_GetLastChangedField(snapInfo, from, to, numFields, stateFields);
    if ( lc )
    {
        MSG_WriteDeltaFields(snapInfo, msg, time, from, to, force, lc - 1, numFields, stateFields, skippedFieldBits, 0);
        return MSG_GetUsedBitCount(msg) - startBits;
    }
    else if ( force )
    {
        MSG_WriteBit0(msg);
        return MSG_GetUsedBitCount(msg) - startBits;
    }
    else
    {
        return 0;
    }
}

int __cdecl MSG_WriteEntityDelta(
                const SnapshotInfo_s *snapInfo,
                msg_t *msg,
                int time,
                const entityState_s *fromES,
                const entityState_s *toES,
                DeltaFlags flags,
                int numFields,
                int indexBits,
                const NetField *stateFields)
{
    const char *v9; // eax
    const char *v10; // eax
    int startBits; // [esp+0h] [ebp-30h]
    const NetField *field; // [esp+8h] [ebp-28h]
    int lc; // [esp+Ch] [ebp-24h]
    int i; // [esp+1Ch] [ebp-14h]
    int fieldsChanged[3]; // [esp+20h] [ebp-10h] BYREF
    const unsigned __int8 *to; // [esp+2Ch] [ebp-4h]

    lc = -1;
    if ( maxStateFieldCount / 32 != 2 )
    {
        v9 = va("The fieldsChanged array needs to be changed to %i\n", numEntityStateFields / 32 + 1);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\sv_msg_write_mp.cpp",
                        382,
                        0,
                        "%s\n\t%s",
                        "maxStateFieldCount / 32 + 1 == (sizeof ( fieldsChanged ) / sizeof( int ))",
                        v9) )
            __debugbreak();
    }
    ++g_entsDeltaCompared;
    to = (const unsigned __int8 *)toES;
    startBits = MSG_GetUsedBitCount(msg);
    memset(fieldsChanged, 0, sizeof(fieldsChanged));
    if ( msg->readOnly
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\sv_msg_write_mp.cpp",
                    394,
                    0,
                    "%s",
                    "!msg->readOnly") )
    {
        __debugbreak();
    }
    if ( !to && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\sv_msg_write_mp.cpp", 395, 0, "%s", "to") )
        __debugbreak();
    if ( *(unsigned int *)to >= (unsigned int)(1 << indexBits) )
    {
        v10 = va("to = %i, bits = %i", *(unsigned int *)to, indexBits);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\sv_msg_write_mp.cpp",
                        397,
                        0,
                        "%s\n\t%s",
                        "*reinterpret_cast< const uint * >( to ) < (1u << indexBits)",
                        v10) )
            __debugbreak();
    }
    i = 0;
    field = stateFields;
    while ( i < numFields )
    {
        if ( MSG_ShouldSendEntityField(snapInfo, fromES, toES, field) )
        {
            lc = i;
            fieldsChanged[i >> 5] |= 1 << (i & 0x1F);
        }
        ++i;
        ++field;
    }
    if ( (lc < -1 || lc > numFields)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\sv_msg_write_mp.cpp",
                    413,
                    0,
                    "%s\n\t(lc) = %i",
                    "(lc >= -1 && lc <= numFields)",
                    lc) )
    {
        __debugbreak();
    }
    if ( lc >= 0 )
    {
        MSG_WriteEntityIndex(snapInfo, msg, *(unsigned int *)to, indexBits);
        MSG_WriteBit0(msg);
        if ( (fieldsChanged[lc >> 5] & (1 << (lc & 0x1F))) == 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\sv_msg_write_mp.cpp",
                        468,
                        0,
                        "%s",
                        "fieldsChanged[lc >> 5] & ( 1 << ( lc & 31 ) )") )
        {
            __debugbreak();
        }
        MSG_WriteEntityDeltaFields(
            snapInfo,
            msg,
            time,
            fromES,
            toES,
            flags == DELTA_FLAGS_FORCE,
            lc,
            numFields,
            stateFields,
            3,
            fieldsChanged);
        return MSG_GetUsedBitCount(msg) - startBits;
    }
    else if ( flags )
    {
        return 0;
    }
    else
    {
        MSG_WriteEntityIndex(snapInfo, msg, *(unsigned int *)to, indexBits);
        MSG_WriteBit0(msg);
        MSG_WriteBit0(msg);
        return MSG_GetUsedBitCount(msg) - startBits;
    }
}

void __cdecl MSG_WriteEntityDeltaFields(
                const SnapshotInfo_s *snapInfo,
                msg_t *msg,
                int time,
                const entityState_s *fromES,
                const entityState_s *toES,
                int force,
                int lastChanged,
                int numFields,
                const NetField *stateFields,
                int skippedFieldBits,
                int *fieldsChanged)
{
    const char *v11; // eax
    bool v12; // [esp+0h] [ebp-28h]
    int v13; // [esp+4h] [ebp-24h]
    bool v14; // [esp+8h] [ebp-20h]
    int bits; // [esp+Ch] [ebp-1Ch]
    const NetField *field; // [esp+1Ch] [ebp-Ch]
    const NetField *fielda; // [esp+1Ch] [ebp-Ch]
    int i; // [esp+20h] [ebp-8h]
    int ia; // [esp+20h] [ebp-8h]
    int lastChangeda; // [esp+48h] [ebp+20h]
    int lastChangedb; // [esp+48h] [ebp+20h]

    if ( msg->readOnly
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\sv_msg_write_mp.cpp",
                    285,
                    0,
                    "%s",
                    "!msg->readOnly") )
    {
        __debugbreak();
    }
    if ( lastChanged >= numFields
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\sv_msg_write_mp.cpp",
                    286,
                    0,
                    "%s",
                    "lastChanged < numFields") )
    {
        __debugbreak();
    }
    if ( force || lastChanged >= 0 )
    {
        MSG_WriteBit1(msg);
        if ( lastChanged < 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\sv_msg_write_mp.cpp",
                        301,
                        0,
                        "%s",
                        "lastChanged >= 0") )
        {
            __debugbreak();
        }
        if ( snapInfo->entJustUnlinked )
            MSG_WriteBit1(msg);
        else
            MSG_WriteBit0(msg);
        MSG_WriteLastChangedField(msg, lastChanged, numFields);
        lastChangeda = -1;
        for ( i = 0; i <= lastChanged; ++i )
        {
            field = &stateFields[i];
            if ( (fieldsChanged[i >> 5] & (1 << (i & 0x1F))) != 0 && (field->changeHints != 2 || i == lastChanged) )
            {
                v14 = 0;
                if ( snapInfo->entJustUnlinked )
                {
                    bits = field->bits;
                    if ( bits >= -66 && bits <= -61 )
                        v14 = 1;
                }
                if ( MSG_WriteDeltaField(
                             snapInfo,
                             msg,
                             time,
                             (const unsigned __int8 *)fromES,
                             (const unsigned __int8 *)toES,
                             field,
                             i,
                             1,
                             !v14,
                             lastChangeda,
                             skippedFieldBits,
                             1) )
                {
                    lastChangeda = i;
                }
                else if ( !Assert_MyHandler(
                                         "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\sv_msg_write_mp.cpp",
                                         335,
                                         0,
                                         "We expected to send a field and it didn't send!") )
                {
                    __debugbreak();
                }
            }
        }
        if ( lastChangeda != lastChanged
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\sv_msg_write_mp.cpp",
                        338,
                        0,
                        "%s",
                        "lastChanged == numFields") )
        {
            __debugbreak();
        }
        lastChangedb = -1;
        for ( ia = 0; ia < lastChanged; ++ia )
        {
            fielda = &stateFields[ia];
            if ( fielda->changeHints == 2 )
            {
                v12 = 0;
                if ( snapInfo->entJustUnlinked )
                {
                    v13 = fielda->bits;
                    if ( v13 >= -66 && v13 <= -61 )
                        v12 = 1;
                }
                if ( MSG_WriteDeltaField(
                             snapInfo,
                             msg,
                             time,
                             (const unsigned __int8 *)fromES,
                             (const unsigned __int8 *)toES,
                             fielda,
                             ia,
                             1,
                             !v12,
                             lastChangedb,
                             skippedFieldBits,
                             0) )
                {
                    lastChangedb = ia;
                }
                else
                {
                    v11 = va("An ALWAYS_CHANGES field (%s) didn't have its delta written?", fielda->name);
                    if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\sv_msg_write_mp.cpp", 358, 0, v11) )
                        __debugbreak();
                }
            }
        }
    }
    else
    {
        MSG_WriteBit0(msg);
    }
}

bool __cdecl MSG_WriteDeltaArchivedEntity(
                SnapshotInfo_s *snapInfo,
                msg_t *msg,
                int time,
                const archivedEntity_s *from,
                const archivedEntity_s *to,
                DeltaFlags flags)
{
    PacketEntityType PacketEntityTypeForEType; // eax
    const NetFieldList *NetFieldList; // eax
    const NetField *field; // [esp+4h] [ebp-Ch]

    if ( msg->readOnly
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\sv_msg_write_mp.cpp",
                    499,
                    0,
                    "%s",
                    "!msg->readOnly") )
    {
        __debugbreak();
    }
    PacketEntityTypeForEType = MSG_GetPacketEntityTypeForEType(to->s.eType);
    MSG_PacketAnalyze_SetPacketEntityType(snapInfo, PacketEntityTypeForEType, 1);
    MSG_WriteEntityDeltaForEType(snapInfo, msg, time, to->s.eType, &from->s, &to->s, flags);
    field = MSG_GetNetFieldList(NET_FIELD_TYPE_ARCHIVED_ENTITY, snapInfo->demoSnapshot)->array;
    NetFieldList = MSG_GetNetFieldList(NET_FIELD_TYPE_ARCHIVED_ENTITY, snapInfo->demoSnapshot);
    return MSG_WriteAppendedDeltaStruct(
                     snapInfo,
                     msg,
                     time,
                     (unsigned __int8 *)&from->r,
                     (unsigned __int8 *)&to->r,
                     flags == DELTA_FLAGS_FORCE,
                     NetFieldList->count,
                     10,
                     field,
                     0,
                     1) > 0;
}

void __cdecl MSG_WriteDeltaHudElems(
                SnapshotInfo_s *snapInfo,
                msg_t *msg,
                int time,
                const hudelem_s *from,
                const hudelem_s *to,
                int count)
{
    const char *v6; // eax
    int UsedBitCount; // eax
    int j; // [esp+0h] [ebp-18h]
    int lastChanged; // [esp+4h] [ebp-14h]
    int lc; // [esp+8h] [ebp-10h]
    int i; // [esp+Ch] [ebp-Ch]
    int inuse; // [esp+10h] [ebp-8h]
    int bitsUsedAtStart; // [esp+14h] [ebp-4h]

    if ( msg->readOnly
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\sv_msg_write_mp.cpp",
                    542,
                    0,
                    "%s",
                    "!msg->readOnly") )
    {
        __debugbreak();
    }
    if ( count != 31
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\sv_msg_write_mp.cpp",
                    543,
                    0,
                    "%s",
                    "count == MAX_HUDELEMS_ARCHIVAL || count == MAX_HUDELEMS_CURRENT") )
    {
        __debugbreak();
    }
    for ( inuse = 0; inuse < count && to[inuse].type; ++inuse )
        ;
    MSG_WriteBits(msg, inuse, 5u);
    // zombies: the live (non-demo) table comes from MSG_GetNetFieldList, like the reader (MSG_ReadDeltaHudElems) and
    // MSG_WriteDeltaHudElems_LastChangedField; since L22 it is hudElemFields in every mode (SP 0x00A5D380 encodes alike).
    const NetField *hudFields = snapInfo->demoSnapshot ? hudElemFields : MSG_GetNetFieldList(NET_FIELD_TYPE_HUDELEMS, 0)->array;
    for ( i = 0; i < inuse; ++i )
    {
        bitsUsedAtStart = MSG_GetUsedBitCount(msg);
        MSG_WriteDeltaHudElems_ValidateHudElem(&from[i], &to[i]);
        lc = MSG_WriteDeltaHudElems_LastChangedField(snapInfo, i, from, to);
        MSG_WriteBits(msg, lc + 1, 6u);
        lastChanged = -1;
        for ( j = 0; j <= lc; ++j )
        {
            if ( MSG_WriteDeltaField(
                         snapInfo,
                         msg,
                         time,
                         (const unsigned __int8 *)&from[i],
                         (const unsigned __int8 *)&to[i],
                         &hudFields[j],
                         j,
                         0,
                         1,
                         lastChanged,
                         3,
                         1) )
            {
                lastChanged = j;
            }
        }
        if ( lastChanged != lc )
        {
            v6 = va("%i != %i", lastChanged, lc);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\sv_msg_write_mp.cpp",
                            577,
                            0,
                            "%s\n\t%s",
                            "lastChanged == lc",
                            v6) )
                __debugbreak();
        }
        if ( !snapInfo->archived && sv_debugPacketContentsQuick->current.integer > 0 )
        {
            UsedBitCount = MSG_GetUsedBitCount(msg);
            CG_QuickPrint("Hudelem %d bits\n", UsedBitCount - bitsUsedAtStart);
        }
    }
}

