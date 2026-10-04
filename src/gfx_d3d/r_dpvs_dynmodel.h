#pragma once

struct GfxWorldDpvsDynamic // sizeof=0x30
{                                       // XREF: GfxWorld/r
    unsigned int dynEntClientWordCount[2];
                                        // XREF: R_LoadWorldInternal(char const *)+20D/w
                                        // R_LoadWorldRuntime+E6/r ...
    unsigned int dynEntClientCount[2];  // XREF: R_LoadWorldInternal(char const *)+1F3/w
                                        // R_LoadWorldInternal(char const *)+1FD/r ...
    unsigned int *dynEntCellBits[2];    // XREF: R_LoadWorldRuntime+2ED/w
    unsigned __int8 *dynEntVisData[2][3];
                                        // XREF: R_LoadWorldRuntime+102/w
};

void __cdecl R_AddCellDynModelSurfacesInFrustumCmd(const struct DpvsPlane **data);
void __cdecl R_CullDynModelInCell(
                const unsigned int *dynEntCellBits,
                unsigned int dynEntClientWordCount,
                struct DynEntityPose *dynModelList,
                const struct DpvsPlane *planes,
                int planeCount,
                unsigned __int8 *dynEntVisData);
