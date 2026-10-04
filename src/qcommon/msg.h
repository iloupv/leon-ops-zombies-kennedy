#pragma once
#include "msg_mp.h"
#include <demo/demo_common.h>

int __cdecl MSG_RoundFloatToInt(float value);

int __cdecl MSG_ReadValueNoXor(msg_t *msg, int bits);
int __cdecl MSG_ReadValue(msg_t *msg, const int *fromF, int *toF, int bits, int size);
int __cdecl MSG_GetField(const int *i, int size);
double __cdecl MSG_ReadDeltaAngle(msg_t *msg, float oldFloat);
int __cdecl MSG_ReadLastChangedField(msg_t *msg, int totalFields);
int __cdecl MSG_ReadNumFieldsSkipped(msg_t *msg, int skippedFieldBits, int maxVal);
void __cdecl MSG_CopyFieldOver(const NetField *stateFields, char *from, char *to, int fieldNum);
void __cdecl MSG_ReadDeltaFields(
                msg_t *msg,
                int time,
                char *from,
                char *to,
                int numFields,
                const NetField *stateFields,
                int skippedFieldBits);
int __cdecl MSG_ReadDeltaStruct(
                msg_t *msg,
                int time,
                char *from,
                char *to,
                unsigned int number,
                int numFields,
                char indexBits,
                const NetField *stateFields,
                int skippedFieldBits);
void    MSG_ReadDeltaMatchState(msg_t *msg, int time, MatchState *from, MatchState *to);
int __cdecl MSG_ReadDeltaClient(msg_t *msg, int time, clientState_s *from, clientState_s *to, unsigned int number);
int __cdecl MSG_ReadDeltaEntity(msg_t *msg, int time, entityState_s *from, entityState_s *to, unsigned int number);


static constexpr __int16 g_commonAngleDeltas[7] = { 364, 546, 182, 1, 910, 1092, 729 }; // BO1TODO: this is kinda interesting, look at this later
