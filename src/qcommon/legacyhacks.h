#pragma once

struct LegacyHacks // sizeof=0x4C
{                                       // XREF: .data:LegacyHacks legacyHacks/r
    int cl_downloadSize;                // XREF: CL_BeginDownload(char const *,char const *)+6C/w
                                        // CL_ParseWWWDownload(int,msg_t *)+D6/w ...
    int cl_downloadCount;               // XREF: CL_BeginDownload(char const *,char const *)+76/w
                                        // CL_ParseDownload+28E/w ...
    int cl_downloadTime;                // XREF: CL_BeginDownload(char const *,char const *)+86/w
    char cl_downloadName[64];           // XREF: CL_Disconnect(int,bool)+14B/w
};

extern LegacyHacks legacyHacks;