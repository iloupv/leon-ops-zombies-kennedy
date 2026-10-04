#pragma once
// zombies: lane x3 - SP AI builtins (functions and methods) ported from the SP exe.
// Registered in src/game_sp/g_scr_sp_ai.cpp; implementations in g_scr_sp_ai_cmds.cpp.

#include <game/actor.h>

// methods
void G_m_setdeathcontents(scr_entref_t entref);     // SP 0x007f54e0
void G_m_getaivelocity(scr_entref_t entref);        // SP 0x007f53e0
void G_m_lookatentity(scr_entref_t entref);         // SP 0x008061b0
void G_m_getthreatbiasgroup(scr_entref_t entref);   // SP 0x00819630
void G_m_issuppressed(scr_entref_t entref);         // SP 0x007cb9d0
void G_m_stopshoot(scr_entref_t entref);            // SP 0x007c9cb0
void G_m_getturret(scr_entref_t entref);            // SP 0x007ccc80
void G_m_makefakeai(scr_entref_t entref);           // SP 0x00610130

void G_m_findbestcovernode(scr_entref_t entref);   // SP 0x007c9e20
void G_m_canshoot(scr_entref_t entref);            // SP 0x007caba0
void G_m_startactorreact(scr_entref_t entref);      // SP 0x007ce8f0
void G_m_usecovernode(scr_entref_t entref);         // SP 0x007c9ec0
void G_m_checkgrenadethrow(scr_entref_t entref);    // SP 0x007cba70
void G_m_checkgrenadethrowpos(scr_entref_t entref); // SP 0x007cbb70
void G_m_throwgrenade(scr_entref_t entref);         // SP 0x007cbc90
void G_m_updateplayersightaccuracy(scr_entref_t entref); // SP 0x007c9d70
void G_m_useturret(scr_entref_t entref);            // SP 0x007cc3d0
void G_m_canuseturret(scr_entref_t entref);         // SP 0x007cc500
void G_m_stopuseturret(scr_entref_t entref);        // SP 0x007cc4c0

// functions
void G_f_isnodeoccupied();                          // SP 0x0067e850
void G_f_badplace_arc();                            // SP 0x00800160
void G_f_badplace_cylinder();                       // SP 0x00800060
void G_f_findpath();                                // SP 0x0040a420
void G_f_getanynodearray();                         // SP 0x00484140
void G_f_getweaponaccuracy();                       // SP 0x007fcb90
