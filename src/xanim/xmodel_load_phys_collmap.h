#pragma once

struct collision_material_t // sizeof=0x48
{                                       // XREF: .data:collision_material_t * g_collision_materials/r
    char name[64];
    int cflags;
    int sflags;
};

struct XModel;
struct PhysGeomList;
struct PhysGeomInfo;
struct SimplePlaneIntersection;
struct DObjAnimMat;


bool __cdecl PlaneEqual(const float *p1, const float *p2);
char __cdecl RemoveDuplicateBrushPlanes(
                float (*planes)[4],
                unsigned int *sideCount,
                const char *mapname,
                unsigned int brushnum);
void __cdecl SnapPlane(float *plane);
char __cdecl SnapAxialVector(float *normal);
void __cdecl GetCollisionMaterial(char *name, collision_material_t *cmat);
int __cdecl IsAxisAligned(float *normal);
char __cdecl SkipEpair(char *token, const char **file);
void __cdecl XModel_LoadCollMap(const char *name, void *(__cdecl *Alloc)(int), XModel *model, const char *xmodelName);
unsigned int __cdecl Xmodel_CountPhysicsCollMaps(
                const char **file,
                const char *name,
                unsigned int *boneHashes,
                int boneNameLen);
char __cdecl AddBoneName(unsigned int *boneNameArray, int boneNameLen, const char *boneName);
unsigned int __cdecl Xmodel_CountPhysicsCollMapGeoms(const char **file, const char *name, unsigned int boneHash);
PhysGeomList *__cdecl Xmodel_ParsePhysicsCollMap(
                const char **file,
                char *name,
                unsigned int geomCount,
                unsigned int boneHash,
                DObjAnimMat *boneMat,
                void *(__cdecl *Alloc)(int));
char __cdecl Xmodel_ParsePhysicsBrush(
                char **file,
                char *mapname,
                unsigned int brushCount,
                PhysGeomInfo *geom,
                unsigned int boneHash,
                DObjAnimMat *boneMat,
                void *(__cdecl *Alloc)(int));
unsigned int __cdecl GetPlaneIntersections(
                const float (*planes)[4],
                unsigned int planeCount,
                SimplePlaneIntersection *OutPts,
                unsigned int maxPtCount);
int __cdecl AddSimplePoint(
                const float (*planes)[4],
                unsigned int planeCount,
                const unsigned int *planeIndex,
                const float *xyz,
                SimplePlaneIntersection *pts,
                int ptCount,
                int maxPtCount);
void __cdecl Map_SkipOptionalArg(const char **file, const char *argName);
char __cdecl Map_SkipNamedFlags(const char **file, const char *key);
bool __cdecl Xmodel_ParsePhysicsBox(const char **file, PhysGeomInfo *geom);
char __cdecl Xmodel_ParsePhysicsCylinder(const char **file, PhysGeomInfo *geom);
void __cdecl GetGeomAABB(const PhysGeomInfo *geom, float *mins, float *maxs);
