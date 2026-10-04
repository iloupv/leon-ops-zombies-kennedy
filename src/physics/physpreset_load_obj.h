#pragma once

struct PhysPreset // sizeof=0x54
{                                       // XREF: XAssetPoolEntry<PhysPreset>/r
    const char *name;                   // XREF: Ragdoll_CreatePhysObj+3A9/w
    int flags;
    float mass;                         // XREF: Ragdoll_CreatePhysObj+3A4/w
                                        // GlassShard::InitPhysicsObj(bool)+1CB/w
    float bounce;                       // XREF: Ragdoll_CreatePhysObj+370/w
    float friction;                     // XREF: Ragdoll_CreatePhysObj+397/w
                                        // GlassShard::InitPhysicsObj(bool)+1D9/w
    float bulletForceScale;             // XREF: Ragdoll_CreatePhysObj+37D/w
    float explosiveForceScale;          // XREF: Ragdoll_CreatePhysObj+38A/w
    const char *sndAliasPrefix;         // XREF: Ragdoll_CreatePhysObj+3B0/w
    float piecesSpreadFraction;
    float piecesUpwardVelocity;
    int canFloat;                       // XREF: Ragdoll_CreatePhysObj+3B7/w
    float gravityScale;
    float centerOfMassOffset[3];        // XREF: Ragdoll_CreatePhysObj+3C6/w
                                        // Ragdoll_CreatePhysObj+3D3/w ...
    float buoyancyBoxMin[3];            // XREF: Ragdoll_CreatePhysObj+3ED/w
                                        // Ragdoll_CreatePhysObj+3FA/w ...
    float buoyancyBoxMax[3];            // XREF: Ragdoll_CreatePhysObj+414/w
};

PhysPreset *__cdecl PhysPresetLoadFile(const char *name, void *(__cdecl *Alloc)(int));
void __cdecl PhysPreset_Strcpy(unsigned __int8 *member, const char *keyValue);
PhysPreset *__cdecl PhysPresetPrecache(const char *name, void *(__cdecl *Alloc)(int));
PhysPreset *__cdecl PhysPreset_Register(const char *name);
PhysPreset *__cdecl PhysPreset_Register_FastFile(const char *name);
PhysPreset *__cdecl PhysPreset_Register_LoadObj(const char *name);
