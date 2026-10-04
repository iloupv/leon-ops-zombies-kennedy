#pragma once
// Lane x1 (players): SP player / session builtins, their side state and the gated engine hooks
// that consume it. Rows live in g_scr_sp_players.cpp; bodies in g_scr_sp_players_x1.cpp.

struct scr_entref_t;
struct gclient_s;
struct gentity_s;
struct game_hudelem_s;
struct hudelem_s;

// Side state for SP gclient / clientState fields BO1Zombies's fixed-stride structs lack.
struct SPPlayerX1State
{
    bool beingRevived;  // zombies: SP clientState +0x44 "beingRevived" (1 bit netfield, SP 0x00a5c120).
    int noAutoPickup;   // zombies: SP client +0x1ccc, read by Touch_Item_Auto (SP 0x00562e70).
    int groundRefEnt;   // zombies: SP client +0x1d10, ENTITYNUM_NONE at ClientSpawn (SP 0x0048036c).
};

SPPlayerX1State &G_SP_PlayerX1State(unsigned int clientNum);
void G_SP_ClearPlayerX1State(unsigned int clientNum);
void G_SP_ClearLevelX1State();
// zombies: Touch_Item_Auto's SP early return (SP 0x00562e70).
bool G_SP_NoAutoPickup(const gentity_s *other, int touched);

// player methods
void PlayerCmdSP_startrevive(scr_entref_t entref);
void PlayerCmdSP_stoprevive(scr_entref_t entref);
void PlayerCmdSP_visionsetlaststand(scr_entref_t entref);
void PlayerCmdSP_setmaxhealth(scr_entref_t entref);
void PlayerCmdSP_hideviewmodel(scr_entref_t entref);
void PlayerCmdSP_showviewmodel(scr_entref_t entref);
void PlayerCmdSP_reloadbuttonpressed(scr_entref_t entref);
void PlayerCmdSP_allowpickupweapons(scr_entref_t entref);
void PlayerCmdSP_setautopickup(scr_entref_t entref);
void PlayerCmdSP_getweaponmuzzlepoint(scr_entref_t entref);
void PlayerCmdSP_getweaponforwarddir(scr_entref_t entref);
void PlayerCmdSP_playersetgroundreferenceent(scr_entref_t entref);
void PlayerCmdSP_playerlinktoabsolute(scr_entref_t entref);
void PlayerCmdSP_setdoublevision(scr_entref_t entref);
void PlayerCmdSP_getplayerviewheight(scr_entref_t entref);
void PlayerCmdSP_islookingat(scr_entref_t entref);
void PlayerCmdSP_resetadswidthandlerp(scr_entref_t entref);
void PlayerCmdSP_setadswidthandlerp(scr_entref_t entref);
void PlayerCmdSP_uploadscore(scr_entref_t entref);
void PlayerCmdSP_setvolfog(scr_entref_t entref);
void PlayerCmdSP_setshadowhint(scr_entref_t entref);
void PlayerCmdSP_startcameratween(scr_entref_t entref);
void PlayerCmdSP_getnormalizedmovement(scr_entref_t entref);
void PlayerCmdSP_getnormalizedcameramovement(scr_entref_t entref);
void PlayerCmdSP_getweaponrenderoptions(scr_entref_t entref);
void PlayerCmdSP_updateweaponoptions(scr_entref_t entref);
void PlayerCmdSP_disableweaponfire(scr_entref_t entref);
void PlayerCmdSP_enableweaponfire(scr_entref_t entref);
void PlayerCmdSP_disableweaponreload(scr_entref_t entref);
void PlayerCmdSP_enableweaponreload(scr_entref_t entref);
void PlayerCmdSP_setloweredweapon(scr_entref_t entref);
// actor methods
void ActorCmdSP_dropweapon(scr_entref_t entref);
void ActorCmdSP_enterprone(scr_entref_t entref);
void ActorCmdSP_exitprone(scr_entref_t entref);
// hudelem method
void HECmdSP_changefontscaleovertime(scr_entref_t entref);
float HudElem_CurrentFontScale_SP(const game_hudelem_s *hud, int time);
void HudElem_SP_ClearFontScaleLerp(const game_hudelem_s *hud);
void HudElem_SP_WireFontScale(const game_hudelem_s *hud, hudelem_s *elem, int time);
// functions
void GScrSP_refreshhudammocounter();
void GScrSP_reportclientdisconnected();
void GScrSP_playerpositionvalid();
void GScrSP_getpersistentprofilevar();
void GScrSP_setpersistentprofilevar();
void GScrSP_hascollectible();
void GScrSP_iscoopepd();
void GScrSP_getweaponclipmodel();
void GScrSP_weaponfightdist();
void GScrSP_weaponmaxdist();
