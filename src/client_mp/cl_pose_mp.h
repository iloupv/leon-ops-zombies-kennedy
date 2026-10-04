#pragma once
#include <xanim/dobj.h>

char *__cdecl CL_AllocSkelMemory(unsigned int size);
int __cdecl CL_GetSkelTimeStamp();
int __cdecl CL_DObjCreateSkelForBones(const DObj *obj, int *partBits, DObjAnimMat **pMatOut);
