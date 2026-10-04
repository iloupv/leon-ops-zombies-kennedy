#pragma once
#include <cgame/cg_draw_names.h>

struct __declspec(align(4)) rankData_t // sizeof=0x488
{                                       // XREF: .data:s_rankData/r
    int minxp[96];                      // XREF: CL_BuildRankXPTable(void)+B6/w
                                        // CL_GetRankForXp(int)+76/r
    int maxxp[96];                      // XREF: CL_BuildRankXPTable(void)+DB/w
                                        // CL_GetRankForXp(int)+83/r
    int rankId[96];                     // XREF: CL_BuildRankXPTable(void)+100/w
                                        // CL_GetRankForXp(int)+F9/r
    int topRow;                         // XREF: CL_BuildRankXPTable(void)+34/w
                                        // CL_GetMaxXP(void)+20/r ...
    bool isValid;                       // XREF: CL_BuildRankXPTable(void):loc_57321C/w
                                        // CL_GetRankForXp(int)+6/r ...
    // padding byte
    // padding byte
    // padding byte
};

void __cdecl CL_BuildRankXPTable();
int __cdecl CL_GetMaxXP();
int __cdecl CL_GetMaxPrestige();
int __cdecl CL_GetRankForXp(int xp);
const char *__cdecl CL_GetRankData(int rank, rankTableColumns_t column);
void __cdecl CL_GetRankIcon(int rank, int prestige, Material **handle);
const char *__cdecl CL_GetRankIconName(int rank, int prestige);
