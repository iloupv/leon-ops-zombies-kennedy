#pragma once

struct CgEntCollNode // sizeof=0x1C
{                                       // XREF: .data:CgEntCollNode (* cgEntCollNodes_data)[1536]/r
    unsigned __int16 sector;
    unsigned __int16 nextEntInSector;
    float linkMins[3];
    float linkMaxs[3];
};

struct CgEntCollTree // sizeof=0xC
{                                       // XREF: CgEntCollSector/r
    float dist;
    unsigned __int16 axis;
    //CgEntCollTree::<unnamed_type_u> u;
    union //CgEntCollTree::<unnamed_type_u> // sizeof=0x2
    {                                       // XREF: CgEntCollTree/r
        unsigned __int16 parent;
        unsigned __int16 nextFree;
    } u;
    unsigned __int16 child[2];
};

struct __declspec(align(4)) CgEntCollSector // sizeof=0x10
{                                       // XREF: CgEntCollWorld/r
    CgEntCollTree tree;
    unsigned __int16 entListHead;
    // padding byte
    // padding byte
};

struct CgEntCollWorld // sizeof=0x401C
{                                       // XREF: .data:CgEntCollWorld * cgEntCollWorld/r
    float mins[3];
    float maxs[3];
    unsigned __int16 freeHead;
    // padding byte
    // padding byte
    CgEntCollSector sectors[1024];
};

void __cdecl CG_ClearEntityCollWorld(int localClientNum);
void __cdecl CG_UnlinkEntityColl(int localClientNum, unsigned int entIndex);
void __cdecl CG_LinkEntityColl(int localClientNum, unsigned int entIndex, const float *absMins, const float *absMaxs);
void __cdecl CG_AddEntityToCollSector(int localClientNum, unsigned int entIndex, unsigned __int16 sectorIndex);
void __cdecl CG_SortEntityCollSector(
                int localClientNum,
                unsigned __int16 sectorIndex,
                const float *mins,
                const float *maxs);
unsigned __int16 __cdecl CG_AllocEntityCollSector(int localClientNum, const float *mins, const float *maxs);


extern CgEntCollWorld cgEntCollWorld[1];
extern CgEntCollNode *cgEntCollNodes[1];