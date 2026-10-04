#pragma once
#include "destructible.h"

struct XModel;

bool __cdecl DestructiblePiece_ParseDefSpecificFieldType(unsigned __int8 *pStruct, const char *pValue, int iFieldType);
int __cdecl DestructiblePiece_SetFlag(
                DestructiblePiece *piece,
                const char *pValue,
                int iFieldType,
                int minVal,
                int maxVal,
                int flag);
DestructiblePiece *__cdecl DestructiblePieceLoadFile(const char *name, DestructiblePiece *destructiblePiece);
void __cdecl DestructibleDef_Strcpy(unsigned __int8 *member, const char *keyValue);
int __cdecl Destructible_ParseDefSpecificFieldType(unsigned __int8 *pStruct, char *pValue, int iFieldType);
DestructibleDef *__cdecl DestructibleDefLoadFile(const char *name, void *(__cdecl *Alloc)(int));
int __cdecl XModelGetBoneIndex(const XModel *model, unsigned int name, unsigned __int8 *index);
