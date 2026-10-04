#pragma once

struct MapnameRichPresenceInfo // sizeof=0x28
{                                       // XREF: .rdata:mapRichPresenceInfo/r
    char mapname[32];                   // XREF: Live_GetMapIndex(char const *)+11/r
    int context;
    int mapPack;                        // XREF: Live_GetMapSource(int)+4C/r
};

int __cdecl Live_GetMapCount();
void __cdecl Live_SetPlaylistVersion(int controllerIndex);
void __cdecl Live_SetPlaylistNum(int controllerIndex, int playlistNum);
int __cdecl Live_GetMapIndex();
int __cdecl Live_GetMapSource(int mapIndex);
