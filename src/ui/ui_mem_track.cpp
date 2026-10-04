#include "ui_mem_track.h"

void __cdecl UI_track_init()
{
#if 0
    g_mem_track_filename = "ui_main";
    TRACK_ui_main();
    g_mem_track_filename = "ui_shared";
    TRACK_ui_shared();
    g_mem_track_filename = "ui_utils";
    TRACK_ui_utils();
    g_mem_track_filename = "ui_atoms";
    //BLOPS_NULLSUB();
    g_mem_track_filename = "ui_memtrack";
    TRACK_g_memtrack();
    g_mem_track_filename = "ui_shared_obj";
    TRACK_ui_shared_obj();
#endif
}

