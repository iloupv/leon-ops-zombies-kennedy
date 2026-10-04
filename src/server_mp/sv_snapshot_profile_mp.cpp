#include "sv_snapshot_profile_mp.h"

#include <qcommon/sv_msg_write_mp.h>
#include "sv_init_mp.h"
#include <bgame/bg_misc.h>
#include <cgame/cg_draw_debug.h>

const char *s_analyzeEntityTypeNames[28] =
{
  "General Entity",
  "Player Entity",
  "Corpse Entity",
  "Item Entity",
  "Missle Entity",
  "Invisible Entity",
  "Script Mover Entity",
  "Sound Blend Entity",
  "FX Entity",
  "Loop FX Entity",
  "Primary Light Entity",
  "MG42 Entity",
  "Helicopter",
  "Plane",
  "Vehicle",
  "Vehicle Collmap",
  "Vehicle Corpse",
  "Actor",
  "Actor Spawner",
  "Actor Corpse",
  "Streamer Hint",
  "Temp Entity",
  "Archived Entity",
  "Match State",
  "Client State",
  "Player State",
  "Hud Elem",
  "Baselines"
};

int s_maxSnapshotSize;
float s_stdSnapshotDeviation;
int s_numSnapshotSamples;
float s_avgSnapshotSize;
int s_numSnapshotsBuiltSinceLastPoll;
int s_uncompressedDataSinceLastPoll;
ClientSnapshotData s_clientSnapshotData[32];

int s_numSnapshotsSentSinceLastPoll;
int s_compressedDataSinceLastPoll;
int s_lastPSBits;
bool g_archivingSnapshot;
int sv_quickBitsTotal;

void __cdecl MSG_PacketAnalyze_SetPacketEntityType(
                SnapshotInfo_s *snapInfo,
                PacketEntityType packetEntityType,
                bool archived)
{
    snapInfo->packetEntityType = packetEntityType;
    snapInfo->packetEntityTypeArchived = archived;
}

const char *__cdecl SV_GetEntityTypeString(unsigned int packetEntityType)
{
    if ( packetEntityType >= 0x1C
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_profile_mp.cpp",
                    653,
                    0,
                    "packetEntityType doesn't index ANALYZE_DATATYPE_ENTITYTYPE_COUNT\n\t%i not in [0, %i)",
                    packetEntityType,
                    28) )
    {
        __debugbreak();
    }
    return s_analyzeEntityTypeNames[packetEntityType];
}

void __cdecl SV_PacketAnalyze_TrackSnapshotSize(int size)
{
    if ( size > s_maxSnapshotSize )
        s_maxSnapshotSize = size;
    s_stdSnapshotDeviation = (float)((float)((float)s_numSnapshotSamples * s_stdSnapshotDeviation)
                                                                 + (float)((float)size - s_avgSnapshotSize))
                                                 / (float)(s_numSnapshotSamples + 1);
    s_avgSnapshotSize = (float)((float)((float)s_numSnapshotSamples * s_avgSnapshotSize) + (float)size)
                                        / (float)(s_numSnapshotSamples + 1);
    ++s_numSnapshotSamples;
    ++s_numSnapshotsBuiltSinceLastPoll;
    s_uncompressedDataSinceLastPoll += size;
}

void __cdecl SV_PacketAnalyze_TrackPacketCompression(unsigned int clientNum, int originalSize, int compressedSize)
{
    int slot; // [esp+0h] [ebp-4h]

    if (clientNum >= 0x20
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\server_mp\\sv_snapshot_profile_mp.cpp",
            1432,
            0,
            "%s\n\t(clientNum) = %i",
            "(clientNum >= 0 && clientNum < 32)",
            clientNum))
    {
        __debugbreak();
    }
    slot = s_clientSnapshotData[clientNum].index & 7;
    s_clientSnapshotData[clientNum].compressedSize[slot] = compressedSize;
    s_clientSnapshotData[clientNum].snapshotSize[slot] = originalSize;
    ++s_clientSnapshotData[clientNum].index;
    ++s_numSnapshotsSentSinceLastPoll;
    s_compressedDataSinceLastPoll += compressedSize;
}

void __cdecl SV_PacketAnalyze_TrackETypeBytes(
                int eType,
                int number,
                int bits,
                bool archived,
                const NetFieldList *fieldList)
{
    char *EntityTypeName; // eax
    const char *fieldArrayName; // [esp-4h] [ebp-4h]

    if ( bits && sv_debugPacketContentsQuick->current.integer > 0 && !archived )
    {
        fieldArrayName = fieldList->fieldArrayName;
        EntityTypeName = BG_GetEntityTypeName(eType);
        CG_QuickPrint("Entity %d %'%s':%d %s\n", number, EntityTypeName, eType, fieldArrayName);
    }
}

void __cdecl SV_PacketAnalyze_TrackPS_ClearBits(int bits)
{
    s_lastPSBits = bits;
}

void __cdecl SV_PacketAnalyze_TrackPS_FieldDeltasBits(int bits)
{
    int currentTotal; // [esp+0h] [ebp-4h]

    currentTotal = bits - s_lastPSBits;
    if ( !g_archivingSnapshot && sv_debugPacketContentsQuick->current.integer > 0 )
    {
        CG_QuickPrint("PlayerState delta fields %i bits\n", currentTotal);
        sv_quickBitsTotal += currentTotal;
    }
    s_lastPSBits = bits;
}

void __cdecl SV_PacketAnalyze_TrackPS_HudelemBits(int bits)
{
    int currentTotal; // [esp+0h] [ebp-4h]

    currentTotal = bits - s_lastPSBits;
    if ( !g_archivingSnapshot && sv_debugPacketContentsQuick->current.integer > 0 )
    {
        CG_QuickPrint("Hudelems Total %d bits\n", currentTotal);
        sv_quickBitsTotal += currentTotal;
    }
    s_lastPSBits = bits;
}

void __cdecl SV_PacketAnalyze_TrackPS_StatsBits(int bits)
{
    int currentTotal; // [esp+0h] [ebp-4h]

    currentTotal = bits - s_lastPSBits;
    if ( !g_archivingSnapshot && sv_debugPacketContentsQuick->current.integer > 0 )
    {
        CG_QuickPrint("PlayerState delta stats %i bits\n", currentTotal);
        sv_quickBitsTotal += currentTotal;
    }
    s_lastPSBits = bits;
}

void __cdecl SV_PacketAnalyze_TrackPS_WeaponBits(int bits)
{
    int currentTotal; // [esp+0h] [ebp-4h]

    currentTotal = bits - s_lastPSBits;
    if ( !g_archivingSnapshot && sv_debugPacketContentsQuick->current.integer > 0 )
    {
        CG_QuickPrint("PlayerState delta weapons %i bits\n", currentTotal);
        sv_quickBitsTotal += currentTotal;
    }
    s_lastPSBits = bits;
}

void __cdecl SV_PacketAnalyze_TrackPS_AmmoPoolBits(int bits)
{
    int currentTotal; // [esp+0h] [ebp-4h]

    currentTotal = bits - s_lastPSBits;
    if ( !g_archivingSnapshot && sv_debugPacketContentsQuick->current.integer > 0 )
    {
        CG_QuickPrint("PlayerState delta pool ammo %i bits\n", currentTotal);
        sv_quickBitsTotal += currentTotal;
    }
    s_lastPSBits = bits;
}

void __cdecl SV_PacketAnalyze_TrackPS_AmmoClipBits(int bits)
{
    int currentTotal; // [esp+0h] [ebp-4h]

    currentTotal = bits - s_lastPSBits;
    if ( !g_archivingSnapshot && sv_debugPacketContentsQuick->current.integer > 0 )
    {
        CG_QuickPrint("PlayerState delta clip ammo %i bits\n", currentTotal);
        sv_quickBitsTotal += currentTotal;
    }
    s_lastPSBits = bits;
}

void __cdecl SV_PacketAnalyze_TrackPS_ObjectivesBits(int bits)
{
    int currentTotal; // [esp+0h] [ebp-4h]

    currentTotal = bits - s_lastPSBits;
    if ( !g_archivingSnapshot && sv_debugPacketContentsQuick->current.integer > 0 )
    {
        CG_QuickPrint("Objectives %d bits\n", currentTotal);
        sv_quickBitsTotal += currentTotal;
    }
    s_lastPSBits = bits;
}

