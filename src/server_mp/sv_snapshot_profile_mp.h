#pragma once

struct SnapshotInfo_s;
struct NetFieldList;
enum PacketEntityType : __int32;

struct ClientSnapshotData // sizeof=0x44
{                                       // XREF: .data:ClientSnapshotData * s_clientSnapshotData/r
    int snapshotSize[8];                // XREF: SV_PacketAnalyze_TrackPacketCompression(int,int,int)+6B/w
    int compressedSize[8];              // XREF: SV_PacketAnalyze_TrackPacketCompression(int,int,int)+58/w
    int index;                          // XREF: SV_PacketAnalyze_TrackPacketCompression(int,int,int)+40/r
};

void __cdecl MSG_PacketAnalyze_SetPacketEntityType(
                SnapshotInfo_s *snapInfo,
                PacketEntityType packetEntityType,
                bool archived);
const char *__cdecl SV_GetEntityTypeString(unsigned int packetEntityType);
void __cdecl SV_PacketAnalyze_TrackSnapshotSize(int size);
void __cdecl SV_PacketAnalyze_TrackPacketCompression(unsigned int clientNum, int originalSize, int compressedSize);
void __cdecl SV_PacketAnalyze_TrackETypeBytes(
                int eType,
                int number,
                int bits,
                bool archived,
                const NetFieldList *fieldList);
void __cdecl SV_PacketAnalyze_TrackPS_ClearBits(int bits);
void __cdecl SV_PacketAnalyze_TrackPS_FieldDeltasBits(int bits);
void __cdecl SV_PacketAnalyze_TrackPS_HudelemBits(int bits);
void __cdecl SV_PacketAnalyze_TrackPS_StatsBits(int bits);
void __cdecl SV_PacketAnalyze_TrackPS_WeaponBits(int bits);
void __cdecl SV_PacketAnalyze_TrackPS_AmmoPoolBits(int bits);
void __cdecl SV_PacketAnalyze_TrackPS_AmmoClipBits(int bits);
void __cdecl SV_PacketAnalyze_TrackPS_ObjectivesBits(int bits);

extern int sv_quickBitsTotal;
extern bool g_archivingSnapshot;
