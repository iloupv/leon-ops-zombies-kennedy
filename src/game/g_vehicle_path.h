#pragma once
#include <qcommon/common.h>
#include <qcommon/radiant_remote.h>
#include "sentient_fields.h"

struct vehicle_spline_node_t // sizeof=0x14
{                                       // XREF: $D7144489F720B3BEDA1018D511D72833/r
    __int16 nextIdx;
    __int16 prevIdx;
    float length;
    float dir[3];
};

struct vehicle_path_node_t // sizeof=0x8
{                                       // XREF: $D7144489F720B3BEDA1018D511D72833/r
    __int16 numLinks;
    __int16 firstLinkIndex;
    float radius;
};

struct vehicle_path_node_link_t // sizeof=0x14
{                                       // XREF: .data:s_node_links/r
    __int16 nextIdx;                    // XREF: VP_FindPath(float const * const,float const * const,vehicle_pathpos_t &)+3DC/r
                                        // VP_FindPath(float const * const,float const * const,vehicle_pathpos_t &)+556/r ...
    // padding byte
    // padding byte
    float length;                       // XREF: VP_FindPath(float const * const,float const * const,vehicle_pathpos_t &)+828/r
    float dir[3];                       // XREF: VP_FindPath(float const * const,float const * const,vehicle_pathpos_t &)+595/o
};

struct vehicle_node_t // sizeof=0x44
{                                       // XREF: .data:s_nodes/r
                                        // vehicle_pathpos_t/r ...
    unsigned __int16 name;              // XREF: G_FindVehicleNode(SpawnVar const *)+C0/r
    unsigned __int16 target;            // XREF: G_SetupSplinePaths(void)+C2/r
                                        // G_FindVehicleNode(SpawnVar const *)+E3/r
    unsigned __int16 target2;           // XREF: G_FindVehicleNode(SpawnVar const *)+106/r
    unsigned __int16 script_linkname;   // XREF: G_FindVehicleNode(SpawnVar const *)+129/r
    unsigned __int16 script_noteworthy; // XREF: G_FindVehicleNode(SpawnVar const *)+14C/r
    __int16 index;
    int flags;                          // XREF: VP_FindPath(float const * const,float const * const,vehicle_pathpos_t &)+7D7/r
    float speed;                        // XREF: VP_GetSpeed+85/r
    float lookAhead;                    // XREF: VP_GetLookAhead+88/r
    float origin[3];                    // XREF: VP_FindPath(float const * const,float const * const,vehicle_pathpos_t &)+8F/o
                                        // VP_FindPath(float const * const,float const * const,vehicle_pathpos_t &)+9D/o ...
    float angles[3];
    //$D7144489F720B3BEDA1018D511D72833 ___u11;
    union // union $D7144489F720B3BEDA1018D511D72833 // sizeof=0x14
    {                                       // XREF: VP_FindPath(float const * const,float const * const,vehicle_pathpos_t &)+34D/r
        vehicle_spline_node_t splineNode;
        vehicle_path_node_t pathNode;
    };
};

struct __declspec(align(2)) vehicle_custom_path_t // sizeof=0x202
{                                       // XREF: .data:vehicle_custom_path_t * gCustomPaths/r
    __int16 pathOrder[128];
    __int16 pathLinkIdx[128];
    unsigned __int8 inUse;              // XREF: VP_GetFreeCustomPath(void)+39/w
                                        // VP_GetFreeCustomPath(void)+63/r ...
    // padding byte
};

struct vehicle_pathpos_t // sizeof=0xDC
{                                       // XREF: scr_vehicle_s/r
                                        // VehiclePhysicsBackup/r ...
    __int16 nodeIdx;                    // XREF: VP_DrawPath(vehicle_pathpos_t const *)+81/r
    __int16 lastNodeIdx;
    __int16 endOfPath;                  // XREF: VP_DrawPath(vehicle_pathpos_t const *)+C6/r
    // padding byte
    // padding byte
    float frac;
    float speed;
    float lookAhead;
    float slide;
    float origin[3];                    // XREF: CMD_VEH_GetAttachPos(scr_entref_t)+99/r
                                        // CMD_VEH_GetAttachPos(scr_entref_t)+A9/r ...
    float angles[3];                    // XREF: CMD_VEH_GetAttachPos(scr_entref_t)+C9/r
                                        // CMD_VEH_GetAttachPos(scr_entref_t)+D9/r ...
    float lookPos[3];
    vehicle_node_t switchNode[2];
    int flags;
    vehicle_custom_path_t *customPath;
    float customGoalLength;
    float customGoalDir[3];
};

struct __declspec(align(4)) vn_field_t // sizeof=0x14
{                                       // XREF: .data:vn_fields/r
    const char *name;
    int ofs;
    int size[1];
    fieldtype_t type;
    bool writable;
    // padding byte
    // padding byte
    // padding byte
};

void __cdecl VP_ResetLinks();
__int16 __cdecl VP_GetNextFreePathNode();
void __cdecl VP_ClearNode(vehicle_node_t *node);
__int16 __cdecl VP_GetNodeIndex(unsigned __int16 name, float *origin);
__int16 __cdecl VP_FindClosestPathNode(const float *pos);
vehicle_custom_path_t *__cdecl VP_GetFreeCustomPath();
void __cdecl VP_FreeCustomPath(vehicle_custom_path_t *path);
int __cdecl VP_FindPath(float *start, float *end, vehicle_pathpos_t *vpp);
void __cdecl VP_DrawCustomPath(vehicle_custom_path_t *customPath);
void __cdecl VP_DrawPath(const vehicle_pathpos_t *vpp);
void __cdecl VP_AddDebugLine(const float *start, const float *end, int forceDraw);
void __cdecl VP_DebugArrow(const float *pos, const float *angles);
void __cdecl G_InitVehiclePaths();
void __cdecl G_FreeVehiclePaths();
void __cdecl VP_ResetNode(vehicle_node_t *node);
void __cdecl G_SetupSplinePaths();
double __cdecl VP_CalcNodeSpeed(__int16 nodeIdx);
double __cdecl VP_CalcNodeLookAhead(__int16 nodeIdx);
void __cdecl VP_CalcNodeAngles(__int16 nodeIdx, float *angles);
void __cdecl G_ConnectVehiclePaths();
void __cdecl G_SetupVehiclePaths();
void __cdecl VP_ZeroNode(vehicle_node_t *node);
void __cdecl G_VehInitPathPos(vehicle_pathpos_t *vpp);
void __cdecl VP_InitNode(vehicle_node_t *node, __int16 nodeIdx);
void __cdecl G_VehFreePathPos(vehicle_pathpos_t *vpp);
void __cdecl G_VehSetUpPathPos(vehicle_pathpos_t *vpp, __int16 nodeIdx);
int __cdecl G_VehUpdatePathPos(vehicle_pathpos_t *vpp, __int16 testNode);
void __cdecl VP_GetAngles(const vehicle_pathpos_t *vpp, float *angles);
__int16 __cdecl VPP_GetCurrentNode(const vehicle_pathpos_t *vpp, __int16 currentNode);
void __cdecl VP_GetLookAheadXYZCustom(const vehicle_pathpos_t *vpp, float *lookXYZ);
void __cdecl VP_GetLookAheadXYZ(const vehicle_pathpos_t *vpp, float *lookXYZ);
int __cdecl VP_UpdatePathPosCustom(vehicle_pathpos_t *vpp, const float *dir, __int16 nodeTest);
double __cdecl VP_GetSpeed(const vehicle_pathpos_t *vpp);
double __cdecl VP_GetLookAhead(const vehicle_pathpos_t *vpp);
double __cdecl VP_GetSlide(const vehicle_pathpos_t *vpp);
int __cdecl VP_UpdatePathPos(vehicle_pathpos_t *vpp, const float *dir, __int16 nodeTest);
void __cdecl VP_BeginSwitchNode(const vehicle_pathpos_t *vpp);
void __cdecl VP_CopyNode(const vehicle_node_t *src, vehicle_node_t *dst);
void __cdecl VP_EndSwitchNode(const vehicle_pathpos_t *vpp);
void __cdecl G_VehSetSwitchNode(vehicle_pathpos_t *vpp, __int16 srcNodeIdx, __int16 dstNodeIdx);
vehicle_node_t *__cdecl G_FindVehicleNode(const SpawnVar *spawnVar);
void __cdecl VP_ParseFields(vehicle_node_t *node, const SpawnVar *spawnVar, bool setScriptVariable);
void __cdecl VP_ParseField(const char *key, char *value, vehicle_node_t *node, bool setScriptVariable);
void __cdecl VP_SetScriptVariable(const char *key, char *value, vehicle_node_t *node);
void __cdecl SP_info_vehicle_node(const SpawnVar *spawnVar, int rotated, int radiantLiveUpdate);
void __cdecl G_ClearSelectedVehicleNode();
void __cdecl G_ProcessVehicleNodeCommand(const RadiantCommand *command, SpawnVar *spawnVar);
unsigned __int16 __cdecl GScr_GetVehicleNodeIndex(unsigned int index);
void __cdecl GScr_AddFieldsForVehicleNode();
void __cdecl GScr_GetVehicleNodeField(unsigned int entnum, unsigned int offset);
void __cdecl GScr_GetVehicleNode();
void __cdecl G_SpawnHeliHeightLock(SpawnVar *spawnVar);


extern int num_heli_height_lock_patches;
