#include "g_mem_track.h"

void __cdecl G_track_init()
{
    // just mem tracking crap
#if 0
    g_mem_track_filename = "g_memtrack";
    TRACK_g_memtrack();
    g_mem_track_filename = "g_main";
    TRACK_g_main();
    g_mem_track_filename = "g_svcmds";
    //BLOPS_NULLSUB();
    g_mem_track_filename = "g_utils";
    TRACK_g_utils();
    g_mem_track_filename = "bg_misctables";
    TRACK_bg_misctables();
    g_mem_track_filename = "bg_weapons";
    //BLOPS_NULLSUB();
    g_mem_track_filename = "bg_weapons_ammo";
    TRACK_bg_weapons_ammo();
    g_mem_track_filename = "bg_weapons_load_obj";
    TRACK_bg_weapons_load_obj();
    g_mem_track_filename = "g_combat";
    TRACK_g_combat();
    g_mem_track_filename = "g_scr_main";
    TRACK_g_scr_main();
    g_mem_track_filename = "g_mover";
    TRACK_g_mover();
    g_mem_track_filename = "g_hudelem";
    TRACK_g_hudelem();
    g_mem_track_filename = "bg_animation_mp";
    TRACK_bg_animation_mp();
    g_mem_track_filename = "g_scr_vehicle";
    TRACK_g_scr_vehicle();
    g_mem_track_filename = "pathnode";
    TRACK_pathnode();
    g_mem_track_filename = "actor";
    TRACK_actor();
    g_mem_track_filename = "actor_physics";
    TRACK_actor_physics();
    g_mem_track_filename = "actor_badplace";
    TRACK_actor_badplace();
    g_mem_track_filename = "actor_event_listener";
    TRACK_actor_event_listener();
    g_mem_track_filename = "enthandle";
    TRACK_enthandle();
    g_mem_track_filename = "actor_aim";
    //BLOPS_NULLSUB();
    g_mem_track_filename = "aim_target";
    TRACK_aim_target();
#endif
}

