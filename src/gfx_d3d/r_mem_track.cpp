#include "r_mem_track.h"

void __cdecl R_Track_Init()
{
#if 0
    g_mem_track_filename = "r_bsp_load_obj";
    TRACK_r_bsp_load_obj();
    g_mem_track_filename = "rb_sky";
    TRACK_rb_sky();
    g_mem_track_filename = "r_buffers";
    TRACK_r_buffers();
    g_mem_track_filename = "r_debug";
    TRACK_r_debug();
    g_mem_track_filename = "r_dpvs";
    TRACK_r_dpvs();
    g_mem_track_filename = "r_fog";
    //BLOPS_NULLSUB();
    g_mem_track_filename = "r_font";
    TRACK_r_font();
    g_mem_track_filename = "r_image";
    TRACK_r_image();
    g_mem_track_filename = "r_image_wavelet";
    TRACK_r_image_wavelet();
    g_mem_track_filename = "r_init";
    TRACK_r_init();
    g_mem_track_filename = "r_marks";
    //BLOPS_NULLSUB();
    g_mem_track_filename = "r_material";
    TRACK_r_material();
    g_mem_track_filename = "r_model";
    TRACK_r_model();
    g_mem_track_filename = "r_rendercmds";
    TRACK_r_rendercmds();
    g_mem_track_filename = "r_screenshot";
    TRACK_r_screenshot();
    g_mem_track_filename = "r_staticmodel";
    //BLOPS_NULLSUB();
    g_mem_track_filename = "r_staticmodelcache";
    TRACK_r_staticmodelcache();
    g_mem_track_filename = "r_utils";
    //BLOPS_NULLSUB();
    g_mem_track_filename = "r_xsurface";
    //BLOPS_NULLSUB();
    g_mem_track_filename = "r_water";
    TRACK_r_water();
    g_mem_track_filename = "r_light";
    //BLOPS_NULLSUB();
    g_mem_track_filename = "rb_debug";
    TRACK_rb_debug();
    g_mem_track_filename = "rb_backend";
    TRACK_rb_backend();
    g_mem_track_filename = "rb_shade";
    //BLOPS_NULLSUB();
    g_mem_track_filename = "rb_showcollision";
    TRACK_rb_showcollision();
    g_mem_track_filename = "rb_state";
    TRACK_rb_state();
    g_mem_track_filename = "rb_stats";
    TRACK_rb_stats();
    g_mem_track_filename = "rb_sunshadow";
    TRACK_rb_sunshadow();
    g_mem_track_filename = "rb_tess";
    //BLOPS_NULLSUB();
#endif
}

