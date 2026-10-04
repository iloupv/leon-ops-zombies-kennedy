#pragma once

struct leafList_s;
struct cLeafBrushNode_s;

int __cdecl CM_PointLeafnum_r(const float *p, int num);
int __cdecl CM_PointLeafnum(const float *p);
void __cdecl CM_BoxLeafnums_r(leafList_s *ll, int nodenum);
void __cdecl CM_StoreLeafs(leafList_s *ll, int nodenum);
int __cdecl CM_BoxLeafnums(const float *mins, const float *maxs, unsigned __int16 *list, int listsize, int *lastLeaf);
int __cdecl CM_PointContents(const float *p, unsigned int model);
int __cdecl CM_PointContentsLeafBrushNode_r(const float *p, cLeafBrushNode_s *node);
int __cdecl CM_TransformedPointContents(const float *p, unsigned int model, const float *origin, const float *angles);
unsigned __int8 *__cdecl CM_ClusterPVS(int cluster);
