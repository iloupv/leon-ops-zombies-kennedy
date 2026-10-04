#pragma once

struct DObj;
struct centity_s;

void __cdecl CG_GetTagMatrix(int localClientNum, int linkEntNum, unsigned __int16 tagName, float (*resultTagMat)[3]);
DObj *__cdecl GetLinkEntDObj(int localClientNum, centity_s *centLink);
void __cdecl CG_CalcTagParentAxis(int localClientNum, centity_s *cent, float (*parentAxis)[3]);
void __cdecl CG_LinkTransformForEntity(int localClientNum, centity_s *cent, float *resultOrigin, float *resultAngles);
void __cdecl CG_GenerateLinkInfo(int localClientNum, centity_s *cent, int attachedEntNum, int attachedTagIndex);
void __cdecl CG_UpdateEntityLink(int localClientNum, centity_s *cent);
void __cdecl CG_UpdateFakeEntityLink(int localClientNum, centity_s *cent, int parentNum, int tagIndex);
void __cdecl CG_UpdateFakeEntityLink(int localClientNum, centity_s *cent);
bool __cdecl CG_EntityLinked(int localClientNum, centity_s *cent);
centity_s *__cdecl CG_EntGetLinkToParent(int localClientNum, centity_s *cent);
