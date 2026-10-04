#include "cg_mem_track.h"

void __cdecl CG_track_init()
{
#if 0
    g_mem_track_filename = "cg_memtrack";
    TRACK_g_memtrack();
    g_mem_track_filename = "cg_main";
    TRACK_cg_main();
    g_mem_track_filename = "cg_camerashake";
    TRACK_cg_camerashake();
    g_mem_track_filename = "cg_draw";
    TRACK_cg_draw();
    g_mem_track_filename = "cg_draw_names";
    TRACK_cg_draw_names();
    g_mem_track_filename = "cg_compassfriendlies";
    TRACK_cg_compassfriendlies();
    g_mem_track_filename = "cg_predict";
    TRACK_cg_predict();
    g_mem_track_filename = "CG_CollWorld";
    TRACK_CG_CollWorld();
    g_mem_track_filename = "cg_view";
    TRACK_cg_view();
    g_mem_track_filename = "cg_localents";
    TRACK_cg_localents();
    g_mem_track_filename = "cg_players";
    //BLOPS_NULLSUB();
    g_mem_track_filename = "aim_assist";
    TRACK_aim_assist();
    g_mem_track_filename = "aim_target";
    TRACK_aim_target();
    g_mem_track_filename = "destructible";
    TRACK_destructible();
#endif
}

