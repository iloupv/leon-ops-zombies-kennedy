#pragma once

struct meminfo_t // sizeof=0x1C0
{                                       // XREF: .data:g_info/r
                                        // .data:g_virtualMemInfo/r ...
    int total[2];                       // XREF: track_PrintInfo(void)+7BC/r
                                        // track_PrintInfo(void)+7DC/w
    int nonSwapTotal;                   // XREF: track_PrintInfo(void)+834/r
                                        // track_PrintInfo(void)+841/w ...
    int nonSwapMinSpecTotal;            // XREF: track_PrintInfo(void)+898/r
                                        // track_PrintInfo(void)+8A5/w ...
    int typeTotal[54][2];               // XREF: track_PrintInfo(void)+780/o
                                        // track_PrintInfo(void)+7A6/o ...
};

struct mem_track_t // sizeof=0x98
{                                       // XREF: track_init(void)+10F/w
                                        // _memmove:UnwindDown1/o ...
    char name[128];                     // XREF: track_init(void)+11A/w
                                        // track_init(void)+126/w ...
    const char *filename;               // XREF: .rdata:00DC1F9E/o
    int size[2];                        // XREF: G_GetPlayerViewOrigin_Internal(playerState_s const *,float * const,bool):off_600000/o
                                        // track_z_alloc(int,char const *,int,void *,int,int)+4B/r ...
    int pos;                            // XREF: track_userhunk_freerange(void *,uint)+4B/r
                                        // track_userhunk_freerange(void *,uint)+65/r ...
    unsigned __int8 type;
    unsigned __int8 usageType;
    // padding byte
    // padding byte
    int count;                          // XREF: .rdata:00D5DB25/o
};


struct TempMemInfo // sizeof=0xAC
{                                       // XREF: .data:g_mallocMemInfoArray/r
                                        // .data:g_combinedMemInfoArray/r ...
    int permanent;
    int high;
    int highExtra;
    int hunkSize;
    int low;
    mem_track_t data;                   // XREF: track_PrintInfo(void)+2B1/o
};

void __cdecl track_static_alloc_internal_simple(void *ptr, int size, const char *name, int memTrack);
void __cdecl track_addbasicinfo(meminfo_t *info, int type, int location, int size);
void __cdecl track_static_alloc_internal(void *ptr, int size, const char *name, int type);
void __cdecl track_flush_physical_alloc(const char *name, unsigned __int8 type);
TempMemInfo *__cdecl GetTempMemInfo(
                int permanent,
                const char *name,
                unsigned __int8 type,
                int usageType,
                TempMemInfo *tempMemInfoArray,
                int *tempMemInfoCount,
                bool add_if_missing);
void __cdecl track_set_max_memory_level();
void __cdecl track_z_alloc(int size, const char *name, int type, char *pos, int project, int overhead);
void __cdecl AddTempMemInfo(
                int size,
                int hunkSize,
                int permanent,
                const char *name,
                unsigned __int8 type,
                int location,
                int usageType,
                TempMemInfo *tempMemInfoArray,
                int *tempMemInfoCount);
void __cdecl CheckHighMemInfo(TempMemInfo *tempMemInfo, int hunkSize, int location);
void __cdecl track_z_free(int type, char *pos, int overhead);
void __cdecl RemoveTempMemInfo(
                int size,
                int permanent,
                const char *name,
                unsigned __int8 type,
                int location,
                int usageType,
                TempMemInfo *tempMemInfoArray,
                int *tempMemInfoCount);
void __cdecl CheckLowMemInfo(TempMemInfo *tempMemInfo, int location);
void __cdecl track_z_commit(int size, int type);
void __cdecl track_z_decommit(int size, int type);
void __cdecl track_physical_alloc(int size, const char *name, int type, int location);
void __cdecl track_temp_alloc(int size, int hunkSize, int permanent, const char *name);
void __cdecl track_temp_free(int size, int permanent, const char *name);
void __cdecl track_temp_clear(int permanent);
void __cdecl track_temp_high_alloc(int size, int hunkSize, int permanent, const char *name);
void __cdecl track_temp_high_clear(int permanent);
void __cdecl track_userhunk_freerange(void *low, unsigned int size);
void __cdecl track_userhunk_free(void *ptr);
void __cdecl track_userhunk_alloc(int size, int pos, const char *name, int type);
void __cdecl track_hunk_alloc(int size, int pos, const char *name, int type);
void __cdecl track_hunk_allocLow(int size, int pos, const char *name, int type);
void __cdecl track_set_hunk_size(int size);
void __cdecl track_hunk_ClearToMarkHigh(int mark);
void __cdecl track_hunk_ClearToMarkLow(int mark);
void __cdecl track_hunk_ClearToStart();
void track_temp_reset();
void track_temp_high_reset();
void __cdecl track_init();
void __cdecl track_getbasicinfo(meminfo_t *info);
void __cdecl track_addbasicmeminfo(meminfo_t *sum, meminfo_t *in);
void __cdecl track_shutdown(int project);
void __cdecl track_PrintInfo();
double __cdecl mb(int n);
int __cdecl mem_track_compare(const char *elem1, const char *elem2);
void __cdecl track_PrintAllInfo();
