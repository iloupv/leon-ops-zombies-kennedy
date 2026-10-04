#pragma once
#include <qcommon/common.h>
#include "pathnode.h"

void __cdecl Path_CreateNodes();
void __cdecl Path_InitStatic(int restart);
void __cdecl Path_ShutdownStatic();
void __cdecl G_ParsePathnodeFields(SpawnVar *spawnVar, pathnode_t *node, nodeType type);
void __cdecl G_ParsePathnodeField(const char *key, char *value, pathnode_t *node);
int __cdecl G_GetNodeTypeFromClassname(const char *classname);
void __cdecl G_SpawnPathnodeStatic(SpawnVar *spawnVar, const char *classname);
void __cdecl SP_spawn_node(SpawnVar *spawnVar, nodeType type);
void __cdecl Scr_FreePathnodeFields(pathnode_t *node);
void __cdecl Path_BuildChains();
void __cdecl Path_BuildChain_r(pathnode_t *node, __int16 wChainId, __int16 wChainDepth);
int __cdecl compare_pathnodes(unsigned __int16 *e0, unsigned __int16 *e1);
void __cdecl Path_ErasePaths(const char *pathsError);
bool __cdecl Path_FindOverlappingNodes();
char *__cdecl vtosf(const float *v);
void __cdecl Path_LoadPaths();
int __cdecl Path_LoadPathsInternal();
bool __cdecl Path_IsNegotiationLink(pathnode_t *pNodeFrom, pathnode_t *pNodeTo);
void __cdecl Path_MakePathDataPermanent(
                pathlink_s *pSourceLinks,
                unsigned int totalLinkCount,
                unsigned __int8 *pVis,
                int visBytes);
pathnode_tree_t *__cdecl Path_BuildNodeBsp_r(unsigned __int16 *nodes, int numNodes);
pathnode_tree_t *__cdecl Path_AllocPathnodeTree();
void __cdecl Path_BuildNodeVis(unsigned __int8 *visbuf, unsigned int iVisCacheSize);
void __cdecl GetNodeAttackPoint(const pathnode_t *node, float *attackPos);
int __cdecl Path_GetExpandedNodeNums_r(unsigned int iNodeStart, float fDistLeft, int *iNodeNums, int iNodeCount);
int __cdecl Path_CheckExpandedNodeVis(int (*iNodeNums)[8192], int *iNodeCount, unsigned __int8 *visbuf);
pathlink_s *__cdecl G_GetNextAvailableTempLinks();
void __cdecl Path_ConnectPathsForSingleNode(pathnode_t *node);
char __cdecl Path_AttemptLink(pathnode_t *pNodeFrom, pathnode_t *pNodeTo, pathlink_s *pLinks, int iMaxLinks);
char __cdecl Path_AttemptLink1(pathnode_t *pNodeFrom, pathnode_t *pNodeTo, pathlink_s *pLinks, int iMaxLinks);
bool __cdecl Path_CanLinkNodes(pathnode_t *pNodeFrom, pathnode_t *pNodeTo, float *pfDist, int *pbNegotiationLink);
bool __cdecl isDeflectionOK(const float *start, const float *origin, const float *movedir);
void __cdecl Path_ConnectPaths();
void __cdecl Path_SavePaths();
unsigned __int8 *__cdecl WriteShort(unsigned __int8 *buffer, unsigned __int16 data);
unsigned __int8 *__cdecl WriteInt(unsigned __int8 *buffer, int data);
