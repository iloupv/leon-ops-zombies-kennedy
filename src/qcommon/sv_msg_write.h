#pragma once
#include "sv_msg_write_mp.h"
#include <qcommon/msg_mp.h>

struct msg_t;

// mod (L51): the entity whose delta is being written (set by MSG_WriteEntityDeltaForEType), named by the
// "too far outside the playable area" errors; SV_BO1DescribeEnt lives in server_mp/sv_snapshot_mp.cpp.
const char *__cdecl SV_BO1DescribeEnt(int num, char *buf, int size);
struct msg;
struct NetFieldList;
struct NetField;

const NetFieldList *__cdecl MSG_GetStateFieldListForEntityType(int eType, bool isDemoSnapshot);
void MSG_CheckForDuplicateOffsets();
void __cdecl MSG_LoopThroughFields(const NetFieldList *fieldToCheck);
const NetFieldList *__cdecl MSG_GetNetFieldList(netFieldTypes_t fieldType, bool isDemoSnapshot);
void __cdecl MSG_WriteReliableCommandToBuffer(
                const char *pszCommand,
                char **pszBuffer,
                char *allocBufferBase,
                int allocBufferLength,
                int *allocBufferPos);
void __cdecl MSG_WriteOriginFloat(
                const SnapshotInfo_s *snapInfo,
                int clientNum,
                msg_t *msg,
                int bits,
                float value,
                float oldValue);
void __cdecl MSG_PacketAnalyze_WriteOriginFloat_Delta(
                int clientNum,
                msg_t *msg,
                int bits,
                float value,
                int roundedValue,
                int roundedOldValue);
void __cdecl MSG_WriteOriginZFloat(
                const SnapshotInfo_s *snapInfo,
                int clientNum,
                msg_t *msg,
                float value,
                float oldValue);
void __cdecl MSG_PacketAnalyze_WriteOriginZFloat_Delta(
                const SnapshotInfo_s *snapInfo,
                int clientNum,
                msg_t *msg,
                float value,
                int roundedValue,
                int roundedOldValue);
void __cdecl MSG_WriteDemoRoundedFloat(
                msg_t *msg,
                int bits,
                float value,
                float oldValue,
                unsigned int diffBits,
                int fullBits);
bool __cdecl MSG_ValuesAreEqual(int clientNum, int bits, int size, const int *fromF, const int *toF);
void __cdecl MSG_WriteEntityIndex(const SnapshotInfo_s *snapInfo, msg_t *msg, int index, int indexBits);
void __cdecl MSG_WriteLastChangedField(msg_t *msg, int lastChangedFieldNum, int numFields);
void __cdecl MSG_WriteEventNum(int clientNum, msg_t *msg, unsigned __int8 eventNum);
void __cdecl MSG_WriteEventParam(int clientNum, msg_t *msg, int eventParam);
int __cdecl MSG_HighBitNumber(unsigned int v);
int __cdecl MSG_WriteRangedFloat(
                int clientNum,
                msg_t *msg,
                float oldValue,
                float value,
                float begin,
                float end,
                int precision,
                bool oftenZeroOrOne);
PacketEntityType __cdecl MSG_GetPacketEntityTypeForEType(int eType);
void __cdecl MSG_WriteValue(
                const SnapshotInfo_s *snapInfo,
                msg_t *msg,
                const int *fromF,
                const int *toF,
                int bits,
                int size);
void __cdecl MSG_WriteValueNoXor(const SnapshotInfo_s *snapInfo, msg_t *msg, int value, int bits);
char __cdecl MSG_WriteDeltaField_Internal(
                const SnapshotInfo_s *snapInfo,
                msg_t *msg,
                int time,
                const unsigned __int8 *from,
                const unsigned __int8 *to,
                const NetField *field,
                int fieldNum,
                const int *fromF,
                const int *toF);
void __cdecl MSG_WriteDeltaTime(int clientNum, msg_t *msg, int timeBase, int time);
void __cdecl MSG_WriteDeltaFrameTime(int clientNum, msg_t *msg, int timeBase, int time);
void __cdecl MSG_WriteEFlags(int clientNum, msg_t *msg, int oldFlags, int newFlags);
void __cdecl MSG_WriteDeltaAngle(const SnapshotInfo_s *snapInfo, msg_t *msg, float oldFloat, float fullFloat);
char __cdecl MSG_WriteDeltaField(
                const SnapshotInfo_s *snapInfo,
                msg_t *msg,
                int time,
                const unsigned __int8 *from,
                const unsigned __int8 *to,
                const NetField *field,
                int fieldNum,
                bool forceSend,
                bool xorValue,
                int lastChangedField,
                int skippedFieldBits,
                bool sendSkippedFields);
void __cdecl MSG_EncodeSkippedFields(
                const SnapshotInfo_s *snapInfo,
                msg_t *msg,
                int numFieldsSkipped,
                int skippedFieldBits);
void __cdecl MSG_WriteDeltaFields(
                const SnapshotInfo_s *snapInfo,
                msg_t *msg,
                int time,
                const unsigned __int8 *from,
                const unsigned __int8 *to,
                int force,
                int lastChanged,
                int numFields,
                const NetField *stateFields,
                int skippedFieldBits,
                bool writeEntUnlinkBit);
void __cdecl MSG_WriteEntityRemoval(
                SnapshotInfo_s *snapInfo,
                msg_t *msg,
                unsigned __int8 *from,
                int indexBits,
                bool changeBit);
int __cdecl MSG_WriteEntityDeltaForEType(
                SnapshotInfo_s *snapInfo,
                msg_t *msg,
                int time,
                int eType,
                const entityState_s *from,
                const entityState_s *to,
                DeltaFlags flags);
bool __cdecl MSG_TrTypeUsesTrDuration(int trType);
bool __cdecl MSG_ShouldEntityFieldBeForcedHidden(
                const SnapshotInfo_s *snapInfo,
                const entityState_s *from,
                const entityState_s *to,
                const NetField *field);
bool __cdecl MSG_ShouldSendEntityField(
                const SnapshotInfo_s *snapInfo,
                const entityState_s *from,
                const entityState_s *to,
                const NetField *field);
bool __cdecl MSG_ShouldEntityFieldBeForcedSent(
                const SnapshotInfo_s *snapInfo,
                const entityState_s *from,
                const entityState_s *to,
                const NetField *field);
int __cdecl MSG_WriteEntity(
                SnapshotInfo_s *snapInfo,
                msg_t *msg,
                int time,
                entityState_s *from,
                const entityState_s *to,
                DeltaFlags flags);
void __cdecl MSG_WriteDeltaClient(
                SnapshotInfo_s *snapInfo,
                msg_t *msg,
                int time,
                clientState_s *from,
                clientState_s *to,
                int force);
bool __cdecl MSG_WithinAllowedPredictionError(float dist, const playerState_s *to);
int __cdecl MSG_GetLastChangedField(
                const SnapshotInfo_s *snapInfo,
                const unsigned __int8 *from,
                const unsigned __int8 *to,
                int numFields,
                const NetField *stateFields);
void __cdecl MSG_WriteDeltaPlayerstate(
                SnapshotInfo_s *snapInfo,
                msg_t *msg,
                int time,
                const playerState_s *from,
                const playerState_s *to);
bool __cdecl MSG_ShouldSendPSField(
                const SnapshotInfo_s *snapInfo,
                bool sendOriginAndVel,
                const playerState_s *ps,
                const playerState_s *oldPs,
                const NetField *field);
// local variable allocation has failed, the output may be wrong!
void    MSG_WriteDeltaMatchState(
                SnapshotInfo_s *snapInfo,
                msg_t *msg,
                int time,
                MatchState *from,
                MatchState *to);
int __cdecl MSG_WriteDeltaHudElems_LastChangedField(
                SnapshotInfo_s *snapInfo,
                int i,
                const hudelem_s *from,
                const hudelem_s *to);
void __cdecl MSG_WriteDeltaHudElems_ValidateHudElem(const hudelem_s *from, const hudelem_s *to);
