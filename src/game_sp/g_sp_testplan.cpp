#include "g_sp_testplan.h"
#include "g_sp_levelstart.h"
#include "g_sp_player_state.h"

#include <game_mp/g_main_mp.h>
#include <game_mp/g_utils_mp.h>
#include <client_mp/sv_client_mp.h>
#include <server_mp/sv_main_mp.h>
#include <server_mp/sv_bot_mp.h>
#include <win32/win_main.h>
#include <universal/dvar.h>
#include <universal/com_math.h>
#include <qcommon/common.h>
#include <qcommon/cmd.h>
#include <game/actor_navigation.h>
#include <game_mp/g_combat_mp.h>
#include <bgame/bg_animation.h>
#include <game/pathnode.h>
#include <game/actor.h>
#include <game_mp/actor_mp.h>
#include <game/g_bsp.h>
#include <bgame/bg_weapons.h>
#include <bgame/bg_weapons_def.h>
#include <bgame/bg_weapons_ammo.h>
#include <bgame/bg_misc.h>
#include <bgame/bg_perks.h>
#include <game/g_items.h>
#include <clientscript/cscr_stringlist.h>
#include <clientscript/cscr_main.h>
#include <clientscript/cscr_compiler.h>
#include <clientscript/scr_const.h>
#include <server/sv_game.h>
#include <server/sv_world.h>
#include <qcommon/cm_trace.h>
#include <qcommon/cm_load.h>
#include <glass/glass_server.h>
#include <cgame/cg_weapons.h>
#include <algorithm>
#include <cmath>
#include <cstring>
#include <string>
#include <vector>

extern bot_info_t botInfos[32]; // sv_bot_mp.cpp

// HEADLESS TEST HARNESS ONLY - not game logic. The plan is a list of steps "go to the trigger of
// feature X, use it the way a player would, wait for the script's own evidence". Triggers are found at
// runtime from the entity list (targetname / script_noteworthy / target / the map's spawn keys, which
// the scripts read as entity fields). The scripts decide what every press does.
//
// TEST SWITCH bo1_testclient_score: when > 0, before and during a purchase step whose price the
// player cannot pay, the harness raises the test player's score to max(price, value). It writes the
// storage the script's `score` field setter writes (ClientScr_SetScore: sess.cs.score.score, then
// CalculateRanks) - the field maps\_zombiemode_score reads for every purchase. Every top-up is logged
// as "bo1_plan: TEST SWITCH". The dvar is registered only in headless runs.

namespace
{
static float s_boxViewCenter[3];
static bool s_boxViewValid;
static bool s_boxViewStraight;
enum StepKind
{
    STEP_DEVGUI_POWERUP, // retail developer command; aim at the floor and collect its ordinary drop
    STEP_BOX_VIEW, // headless screenshot: retreat from the active box and look up at its beam
    STEP_TRAP_VIEW, // headless screenshot: hold the nearest trap in view after activation
    STEP_USE,   // walk to a trigger, press use until the success evidence appears
    STEP_FLOOR, // ride elevators until the player is on floor `count`
    STEP_TOUCH, // walk into a trigger volume (portals)
    STEP_FIRE,  // fire `count` shots of the current weapon (ammo use, fire-rate / reload measurement)
    STEP_BOX,   // mystery box: pay, wait for the weapon, take it (repeat `count` times or until the teddy)
    STEP_PAP,   // pack-a-punch: pay, wait, take the upgraded weapon
    STEP_PAP_VIEW, // w1 c11: -Client evidence: hold the upgraded gun at the Pack-a-Punch machine, hip and ADS screenshots
    STEP_EQUIP, // select an already held weapon through the ordinary usercmd
    STEP_WAIT,  // leave the nearest trap volume, then observe for `count` milliseconds
    STEP_DEPLOY, // select and fire a held claymore, then lure a zombie across its front
    STEP_GRENADE, // throw an ordinary frag through the offhand usercmd
    STEP_CRAWLER, // throw a frag at the blast edge, then observe the retail crawler state
    STEP_GLASS, // observe a pane, shoot once, then shoot the remaining fragments
    STEP_JUMP,  // L42: stand still and jump `count` times; log apex height, air time and ps.gravity (low gravity measure)
};

struct PlanStep
{
    const char *name;
    StepKind kind;
    const char *targetname; // trigger targetname (null: any)
    const char *key;        // filter: "script_noteworthy", "target" or a spawn-key field name
    const char *value;
    int cost;               // what the script charges (TEST SWITCH top-up floor), 0 for free
    const char *notify;     // level notify that proves the script's effect
    const char *perk;       // perk the script must set
    const char *weapon;     // weapon the player must hold afterwards
    int count;              // FIRE shots / FLOOR number / BOX spins
    bool ammo;              // ammo purchase: score spent and ammo count raised
    bool unique;            // never reuse a trigger an earlier step of this plan used
    int timeoutMs;
    // L41: where the notify counts from: 0 this step's start, 1 the previous step's start (a ride the previous step's
    // lever starts: players_riding_minecart fires 0.5 s after the pull, minecart.gsc:700-781), 2 the whole run.
    // notify may list alternatives: "a|b".
    int notifyFrom;
};

const PlanStep s_steps[] =
{
    { "qr", STEP_USE, "zombie_vending", "script_noteworthy", "specialty_quickrevive", 500, nullptr, "specialty_quickrevive", nullptr, 0, false, false, 90000 },
    { "wall_rottweil", STEP_USE, "weapon_upgrade", "zombie_weapon_upgrade", "rottweil72_zm", 1500, nullptr, nullptr, "rottweil72_zm", 0, false, false, 90000 },
    { "wall_m14", STEP_USE, "weapon_upgrade", "zombie_weapon_upgrade", "m14_zm", 1500, nullptr, nullptr, "m14_zm", 0, false, false, 90000 },
    { "fire3", STEP_FIRE, nullptr, nullptr, nullptr, 0, nullptr, nullptr, nullptr, 3, false, false, 30000 },
    { "fire40", STEP_FIRE, nullptr, nullptr, nullptr, 0, nullptr, nullptr, nullptr, 40, false, false, 90000 },
    { "fire12", STEP_FIRE, nullptr, nullptr, nullptr, 0, nullptr, nullptr, nullptr, 12, false, false, 60000 },
    { "equip_pm63", STEP_EQUIP, nullptr, nullptr, nullptr, 0, nullptr, nullptr, "pm63_zm", 0, false, false, 10000 },
    // L18: raise the wall M16 before Pack-a-Punch (the PaP script upgrades the current weapon: m16_gl_upgraded_zm).
    { "equip_m16", STEP_EQUIP, nullptr, nullptr, nullptr, 0, nullptr, nullptr, "m16_zm", 0, false, false, 10000 },
    { "ammo_pm63", STEP_USE, "weapon_upgrade", "zombie_weapon_upgrade", "pm63_zm", 500, nullptr, nullptr, nullptr, 0, true, false, 60000 },
    { "ammo_rottweil", STEP_USE, "weapon_upgrade", "zombie_weapon_upgrade", "rottweil72_zm", 1500, nullptr, nullptr, nullptr, 0, true, false, 60000 },
    { "ammo_m14", STEP_USE, "weapon_upgrade", "zombie_weapon_upgrade", "m14_zm", 1500, nullptr, nullptr, nullptr, 0, true, false, 60000 },
    { "door_conf", STEP_USE, "zombie_door", nullptr, nullptr, 750, "conf1_hall1", nullptr, nullptr, 0, false, false, 90000 },
    { "debris_elev2", STEP_USE, "zombie_debris", "target", "elev2_blocker", 1000, "junk purchased", nullptr, nullptr, 0, false, false, 90000 },
    { "floor1", STEP_FLOOR, nullptr, nullptr, nullptr, 250, nullptr, nullptr, nullptr, 1, false, false, 120000 },
    { "floor2", STEP_FLOOR, nullptr, nullptr, nullptr, 250, nullptr, nullptr, nullptr, 2, false, false, 120000 },
    { "floor3", STEP_FLOOR, nullptr, nullptr, nullptr, 250, nullptr, nullptr, nullptr, 3, false, false, 180000 },
    { "debris_stair", STEP_USE, "zombie_debris", "target", "war_room_stair", 1000, "war_room_stair", nullptr, nullptr, 0, false, false, 90000 },
    { "debris_west", STEP_USE, "zombie_debris", "target", "war_room_west", 1250, "war_room_west", nullptr, nullptr, 0, false, false, 90000 },
    { "debris_elev1", STEP_USE, "zombie_debris", "target", "elev1_blocker", 1000, "war_room_elevator", nullptr, nullptr, 0, false, false, 90000 },
    { "battery", STEP_USE, "trigger_trap_piece", nullptr, nullptr, 0, nullptr, nullptr, nullptr, 0, false, false, 90000 },
    { "power", STEP_USE, "use_elec_switch", nullptr, nullptr, 0, "power_on", nullptr, nullptr, 0, false, false, 90000 },
    // Retail phone_egg increments level.phone_counter after an ordinary use press.
    { "phone1", STEP_USE, "secret_phone_trig", nullptr, nullptr, 0, nullptr, nullptr, nullptr, 1, false, true, 90000 },
    { "phone2", STEP_USE, "secret_phone_trig", nullptr, nullptr, 0, nullptr, nullptr, nullptr, 2, false, true, 90000 },
    { "phone3", STEP_USE, "secret_phone_trig", nullptr, nullptr, 0, nullptr, nullptr, nullptr, 3, false, true, 90000 },
    { "devgui_fire", STEP_DEVGUI_POWERUP, nullptr, nullptr, "fire_sale", 0, nullptr, nullptr, nullptr, 0, false, false, 30000 },
    { "devgui_bonfire", STEP_DEVGUI_POWERUP, nullptr, nullptr, "bonfire_sale", 0, nullptr, nullptr, nullptr, 0, false, false, 30000 },
    // L18: retail devgui "full_ammo" (Max Ammo); passes on the retail "zmb_max_ammo" player notify.
    { "devgui_ammo", STEP_DEVGUI_POWERUP, nullptr, nullptr, "full_ammo", 0, nullptr, nullptr, nullptr, 0, false, false, 30000 },
    // Retail devgui "thief_round" (zombie_devgui_thief_round -> goto_round(level.next_thief_round)); passes on the
    // retail flag_set("thief_round") level notify. The thief itself is never touched by the harness.
    { "devgui_thief", STEP_DEVGUI_POWERUP, nullptr, nullptr, "thief_round", 0, nullptr, nullptr, nullptr, 0, false, false, 90000 },
    { "box", STEP_BOX, "treasure_chest_use", nullptr, nullptr, 950, nullptr, nullptr, nullptr, 1, false, false, 120000 },
    { "box_view", STEP_BOX_VIEW, "treasure_chest_use", nullptr, nullptr, 0, nullptr, nullptr, nullptr, 0, false, false, 45000 },
    // Waits (fighting) for a naturally dropped fire sale; retail only drops one after the box has moved
    // (maps/_zombiemode_powerups: level.chest_moves < 1 skips it), so plan box_teddy first.
    { "box_sale", STEP_BOX, "treasure_chest_use", nullptr, nullptr, 10, nullptr, nullptr, nullptr, 1, false, false, 1500000 },
    // Brief said teddy by the eighth pull; retail counts completed pickups, then tests >= 8
    // on the next roll (maps/_zombiemode_weapons:1259,2061). b4 observed teddy on roll nine.
    { "box_teddy", STEP_BOX, "treasure_chest_use", nullptr, nullptr, 950, "moving_chest_now", nullptr, nullptr, 12, false, false, 400000 },
    { "jugg", STEP_USE, "zombie_vending", "script_noteworthy", "specialty_armorvest", 2500, nullptr, "specialty_armorvest", nullptr, 0, false, false, 90000 },
    { "doubletap", STEP_USE, "zombie_vending", "script_noteworthy", "specialty_rof", 2000, nullptr, "specialty_rof", nullptr, 0, false, false, 90000 },
    { "speedcola", STEP_USE, "zombie_vending", "script_noteworthy", "specialty_fastreload", 3000, nullptr, "specialty_fastreload", nullptr, 0, false, false, 90000 },
    // p1 c17: Mule Kick, the machine _zombiemode_perks::place_additionalprimaryweapon_machine spawns (Five: labs floor,
    // (-1081, 1497, -512)) when scr_zm_extra_perk_* keep it; retail cost 4000 (_zombiemode_perks vending_trigger_think).
    { "mule", STEP_USE, "zombie_vending", "script_noteworthy", "specialty_additionalprimaryweapon", 4000, nullptr, "specialty_additionalprimaryweapon", nullptr, 0, false, false, 90000 },
    { "defcon", STEP_USE, "punch_switch", nullptr, nullptr, 0, nullptr, nullptr, nullptr, 0, false, true, 90000 },
    { "portal_pack", STEP_TOUCH, "portal_trigs", nullptr, nullptr, 0, "open_pack_hideaway", nullptr, nullptr, 0, false, false, 90000 },
    { "portal", STEP_TOUCH, "portal_trigs", nullptr, nullptr, 0, nullptr, nullptr, nullptr, 0, false, false, 90000 },
    { "pap", STEP_PAP, "zombie_vending_upgrade", nullptr, nullptr, 5000, nullptr, nullptr, nullptr, 0, false, false, 120000 },
    // w1 c11: box_leave pays and walks away (the 12 s treasure_chest_timeout path); pap_thief pays as the next round
    // becomes a thief round (devgui thief_round, needs developer_script) and takes only after the round started;
    // pap_view is the -Client gold camo evidence (bo1_testclient_shots).
    { "box_leave", STEP_BOX, "treasure_chest_use", nullptr, nullptr, 950, nullptr, nullptr, nullptr, 1, false, false, 120000 },
    { "pap_thief", STEP_PAP, "zombie_vending_upgrade", nullptr, nullptr, 5000, nullptr, nullptr, nullptr, 0, false, false, 240000 },
    // pap_thief_leave: the same, but the gun is never taken (r3d's path: the thief round takes the player away from it).
    { "pap_thief_leave", STEP_PAP, "zombie_vending_upgrade", nullptr, nullptr, 5000, nullptr, nullptr, nullptr, 0, false, false, 240000 },
    { "pap_view", STEP_PAP_VIEW, "zombie_vending_upgrade", nullptr, nullptr, 0, nullptr, nullptr, nullptr, 0, false, false, 30000 },
    { "trap_fix", STEP_USE, "trigger_battery_trap_fix", nullptr, nullptr, 0, nullptr, nullptr, nullptr, 0, false, false, 90000 },
    { "trap_fix_qr", STEP_USE, "trigger_battery_trap_fix", "script_flag_wait", "trap_quickrevive", 0, "trap_quickrevive", nullptr, nullptr, 0, false, false, 90000 },
    { "trap_fix_elev", STEP_USE, "trigger_battery_trap_fix", "script_flag_wait", "trap_elevator", 0, "trap_elevator", nullptr, nullptr, 0, false, false, 90000 },
    { "trap_qr", STEP_USE, "trap_quickrevive", nullptr, nullptr, 1000, nullptr, nullptr, nullptr, 0, false, false, 60000 },
    { "trap_elev", STEP_USE, "trap_elevator", nullptr, nullptr, 1000, nullptr, nullptr, nullptr, 0, false, false, 60000 },
    { "wait110", STEP_WAIT, nullptr, nullptr, nullptr, 0, nullptr, nullptr, nullptr, 110000, false, false, 120000 },
    // q1: walk into the nearest trap volume (the "zombie_trap" damage trigger) while it runs; passes on the
    // shellshock _zombiemode_traps::player_elec_damage applies ("electrocution", 2.5 s).
    { "trap_touch", STEP_TOUCH, "zombie_trap", nullptr, nullptr, 0, nullptr, nullptr, nullptr, 0, false, false, 20000 },
    // L40: Call of the Dead. After George is angry (his own director_activated notify), walk into the beach water
    // (the waterdamage trigger, script_noteworthy beach_zone) so he follows; passes on his director_calmed
    // (maps_zombie_coast_water touching -> maps\_zombiemode_ai_director::director_calmed). The scripts decide.
    // L40: Call of the Dead's flinger (free; power_on + the residence_beach_group zone: zombie_unlock_all 1) passes on the
    // trigger's trap_done (maps_zombie_coast_flinger.gsc:238; flinger_in_place :84 fires at load, f1 2700); a zipline (free) on the rider's exit_zipline
    // (maps\_zombiemode_player_zipline.gsc:481, an entity notify recorded below).
    { "flinger", STEP_USE, "flinger_activate", nullptr, nullptr, 0, "trap_done", nullptr, nullptr, 0, false, false, 90000 },
    { "zipline", STEP_USE, "player_zipline", nullptr, nullptr, 0, "exit_zipline", nullptr, nullptr, 0, false, false, 90000 },
    { "water_calm", STEP_TOUCH, "waterdamage", "script_noteworthy", "beach_zone", 0, "director_calmed", nullptr, nullptr, 0, false, false, 150000 },
    { "trap_view", STEP_TRAP_VIEW, nullptr, nullptr, nullptr, 0, nullptr, nullptr, nullptr, 25000, false, false, 35000 },
    // The labs' doors (script_flag of the zombie_door triggers; flag_set notifies the level).
    { "door_lab1", STEP_USE, "zombie_door", "script_flag", "lab1_level3", 1250, "lab1_level3", nullptr, nullptr, 0, false, false, 90000 },
    { "door_lab2", STEP_USE, "zombie_door", "script_flag", "lab2_level3", 1250, "lab2_level3", nullptr, nullptr, 0, false, false, 90000 },
    { "door_lab3", STEP_USE, "zombie_door", "script_flag", "lab3_level3", 1250, "lab3_level3", nullptr, nullptr, 0, false, false, 90000 },
    // The other wall buys.
    { "wall_mpl", STEP_USE, "weapon_upgrade", "zombie_weapon_upgrade", "mpl_zm", 1500, nullptr, nullptr, "mpl_zm", 0, false, false, 90000 },
    { "wall_mp5k", STEP_USE, "weapon_upgrade", "zombie_weapon_upgrade", "mp5k_zm", 1500, nullptr, nullptr, "mp5k_zm", 0, false, false, 90000 },
    { "wall_pm63", STEP_USE, "weapon_upgrade", "zombie_weapon_upgrade", "pm63_zm", 1500, nullptr, nullptr, "pm63_zm", 0, false, false, 90000 },
    { "wall_ithaca", STEP_USE, "weapon_upgrade", "zombie_weapon_upgrade", "ithaca_zm", 1500, nullptr, nullptr, "ithaca_zm", 0, false, false, 90000 },
    { "wall_ak74u", STEP_USE, "weapon_upgrade", "zombie_weapon_upgrade", "ak74u_zm", 1500, nullptr, nullptr, "ak74u_zm", 0, false, false, 90000 },
    { "wall_m16", STEP_USE, "weapon_upgrade", "zombie_weapon_upgrade", "m16_zm", 1500, nullptr, nullptr, "m16_zm", 0, false, false, 90000 },
    { "wall_frag", STEP_USE, "weapon_upgrade", "zombie_weapon_upgrade", "frag_grenade_zm", 250, nullptr, nullptr, "frag_grenade_zm", 0, true, false, 90000 },
    { "claymore", STEP_USE, "claymore_purchase", nullptr, nullptr, 1000, nullptr, nullptr, "claymore_zm", 0, false, false, 90000 },
    { "deploy_claymore", STEP_DEPLOY, nullptr, nullptr, nullptr, 0, nullptr, nullptr, "claymore_zm", 0, false, false, 90000 },
    { "throw_frag", STEP_GRENADE, nullptr, nullptr, nullptr, 0, nullptr, nullptr, "frag_grenade_zm", 0, false, false, 10000 },
    { "crawler", STEP_CRAWLER, nullptr, nullptr, nullptr, 0, nullptr, nullptr, "frag_grenade_zm", 0, false, false, 360000 },
    // j1: self-damage routes. STEP_GRENADE's count is how long the frag button is held (0: 300 ms);
    // value "feet" pitches the view straight down for the throw. cook_frag holds past the fuse, so the
    // grenade-suicide event fires in hand.
    { "frag_feet", STEP_GRENADE, nullptr, nullptr, "feet", 0, nullptr, nullptr, "frag_grenade_zm", 0, false, false, 10000 },
    { "cook_frag", STEP_GRENADE, nullptr, nullptr, nullptr, 0, nullptr, nullptr, "frag_grenade_zm", 8000, false, false, 15000 },
    { "glass", STEP_GLASS, nullptr, nullptr, nullptr, 0, nullptr, nullptr, nullptr, 0, false, false, 25000 },
    { "bowie", STEP_USE, "bowie_upgrade", nullptr, nullptr, 3000, nullptr, nullptr, "bowie_knife_zm", 0, false, false, 90000 },
    { "battery_labs", STEP_USE, "trigger_trap_piece", nullptr, nullptr, 0, nullptr, nullptr, nullptr, 0, false, false, 90000 },
    // r1: ammo from the other wall buys (the harness's arm policy inserts them; see ArmCheck).
    { "ammo_m16", STEP_USE, "weapon_upgrade", "zombie_weapon_upgrade", "m16_zm", 1500, nullptr, nullptr, "m16_zm", 0, true, false, 60000 },
    { "ammo_ak74u", STEP_USE, "weapon_upgrade", "zombie_weapon_upgrade", "ak74u_zm", 1500, nullptr, nullptr, "ak74u_zm", 0, true, false, 60000 },
    { "ammo_mp5k", STEP_USE, "weapon_upgrade", "zombie_weapon_upgrade", "mp5k_zm", 1500, nullptr, nullptr, "mp5k_zm", 0, true, false, 60000 },
    { "ammo_mpl", STEP_USE, "weapon_upgrade", "zombie_weapon_upgrade", "mpl_zm", 1500, nullptr, nullptr, "mpl_zm", 0, true, false, 60000 },
    { "ammo_ithaca", STEP_USE, "weapon_upgrade", "zombie_weapon_upgrade", "ithaca_zm", 1500, nullptr, nullptr, "ithaca_zm", 0, true, false, 60000 },
    // L28: Kino der Toten (zombie_theater). Entity keys from the map's entity string; flags from
    // maps\zombie_theater.gsc theater_zone_init and maps\_zombiemode_blockers (a bought door flag_sets its script_flag).
    { "k_door_crem", STEP_USE, "zombie_door", "script_flag", "magic_box_crematorium1", 750, "magic_box_crematorium1", nullptr, nullptr, 0, false, false, 90000 },
    { "k_door_vip", STEP_USE, "zombie_door", "script_flag", "magic_box_foyer1", 750, "magic_box_foyer1", nullptr, nullptr, 0, false, false, 90000 },
    { "k_door_dining", STEP_USE, "zombie_door", "script_flag", "vip_to_dining", 1000, "vip_to_dining", nullptr, nullptr, 0, false, false, 90000 },
    { "k_door_dressing", STEP_USE, "zombie_door", "script_flag", "dining_to_dressing", 1250, "dining_to_dressing", nullptr, nullptr, 0, false, false, 90000 },
    { "k_door_stage", STEP_USE, "zombie_door", "script_flag", "magic_box_dressing1", 1250, "magic_box_dressing1", nullptr, nullptr, 0, false, false, 90000 },
    { "k_door_balcony", STEP_USE, "zombie_door", "script_flag", "magic_box_west_balcony2", 1250, "magic_box_west_balcony2", nullptr, nullptr, 0, false, false, 90000 },
    { "k_door_alley", STEP_USE, "zombie_door", "script_flag", "magic_box_alleyway1", 1000, "magic_box_alleyway1", nullptr, nullptr, 0, false, false, 90000 },
    // Kino's traps: the use triggers carry the zombie_trap's target name; evidence is the trap's _trap_in_use
    // (maps\_zombiemode_traps trap_use_think), not the payment alone (CheckUseSuccess).
    { "k_trap_e1", STEP_USE, "foyer_room_trap", nullptr, nullptr, 1000, nullptr, nullptr, nullptr, 0, false, false, 60000 },
    { "k_trap_e2", STEP_USE, "vip_room_trap", nullptr, nullptr, 1000, nullptr, nullptr, nullptr, 0, false, false, 60000 },
    { "k_trap_e3", STEP_USE, "control_room_trap", nullptr, nullptr, 1000, nullptr, nullptr, nullptr, 0, false, false, 60000 },
    { "k_trap_e4", STEP_USE, "dressing_room_trap", nullptr, nullptr, 1000, nullptr, nullptr, nullptr, 0, false, false, 60000 },
    { "k_trap_f1", STEP_USE, "crematorium_room_trap", nullptr, nullptr, 1000, nullptr, nullptr, nullptr, 0, false, false, 60000 },
    // Kino's teleporter (maps\zombie_theater_teleporter): pad (core_linked), the lobby mainframe pf16_auto1
    // (teleporter_linked), the pad again (teleport to the projection room); count 1: pass on the teleport, count 2:
    // pass on the teleport back to the theater's return points (theater_teleport_player*, about 0 -1270).
    { "k_tp_pad", STEP_USE, "trigger_teleport_pad_0", nullptr, nullptr, 0, "core_linked", nullptr, nullptr, 0, false, false, 90000 },
    { "k_tp_link", STEP_USE, "pf16_auto1", nullptr, nullptr, 0, "teleporter_linked", nullptr, nullptr, 0, false, false, 90000 },
    { "k_tp_go", STEP_USE, "trigger_teleport_pad_0", nullptr, nullptr, 0, nullptr, nullptr, nullptr, 1, false, false, 90000 },
    { "k_tp_back", STEP_USE, "trigger_teleport_pad_0", nullptr, nullptr, 0, nullptr, nullptr, nullptr, 2, false, false, 90000 },
    { "k_wall_m14", STEP_USE, "weapon_upgrade", "zombie_weapon_upgrade", "m14_zm", 500, nullptr, nullptr, "m14_zm", 0, false, false, 90000 },
    { "k_wall_mp40", STEP_USE, "weapon_upgrade", "zombie_weapon_upgrade", "mp40_zm", 1000, nullptr, nullptr, "mp40_zm", 0, false, false, 90000 },
    { "k_bowie", STEP_USE, "bowie_upgrade", nullptr, nullptr, 3000, nullptr, nullptr, "bowie_knife_zm", 0, false, false, 90000 },
    // L35: Der Riese (zombie_cod5_factory). Keys from the map's entity string; flags from maps\zombie_cod5_factory.gsc
    // (zone init: a bought door/debris flag_sets its script_flag; use_power_switch flag_sets power_on).
    { "f_wall_kar98k", STEP_USE, "weapon_upgrade", "zombie_weapon_upgrade", "zombie_kar98k", 200, nullptr, nullptr, "zombie_kar98k", 0, false, false, 90000 },
    { "f_door_east", STEP_USE, "zombie_door", "script_flag", "enter_outside_east", 750, "enter_outside_east", nullptr, nullptr, 0, false, false, 90000 },
    { "f_door_wnuen", STEP_USE, "zombie_door", "script_flag", "enter_wnuen_building", 750, "enter_wnuen_building", nullptr, nullptr, 0, false, false, 90000 },
    { "f_debris_dock", STEP_USE, "zombie_debris", "script_flag", "enter_wnuen_loading_dock", 1000, "enter_wnuen_loading_dock", nullptr, nullptr, 0, false, false, 90000 },
    { "f_door_west", STEP_USE, "zombie_door", "script_flag", "enter_outside_west", 750, "enter_outside_west", nullptr, nullptr, 0, false, false, 90000 },
    { "f_door_warehouse", STEP_USE, "zombie_door", "script_flag", "enter_warehouse_building", 750, "enter_warehouse_building", nullptr, nullptr, 0, false, false, 90000 },
    { "f_debris_2nd", STEP_USE, "zombie_debris", "script_flag", "enter_warehouse_second_floor", 1000, "enter_warehouse_second_floor", nullptr, nullptr, 0, false, false, 90000 },
    { "f_door_tp_east", STEP_USE, "zombie_door", "script_flag", "enter_tp_east", 1250, "enter_tp_east", nullptr, nullptr, 0, false, false, 90000 },
    { "f_door_tp_west", STEP_USE, "zombie_door", "script_flag", "enter_tp_west", 750, "enter_tp_west", nullptr, nullptr, 0, false, false, 90000 },
    { "f_door_tp_south", STEP_USE, "zombie_door", "script_flag", "enter_tp_south", 1250, "enter_tp_south", nullptr, nullptr, 0, false, false, 90000 },
    { "f_power", STEP_USE, "use_power_switch", nullptr, nullptr, 0, "power_on", nullptr, nullptr, 0, false, false, 90000 },
    // Der Riese traps (electric_trap_think): the use trigger's target is the damage trigger; evidence is its in_use 1.
    { "f_trap_wuen", STEP_USE, "wuen_electric_trap", nullptr, nullptr, 1000, nullptr, nullptr, nullptr, 0, false, false, 60000 },
    { "f_trap_warehouse", STEP_USE, "warehouse_electric_trap", nullptr, nullptr, 1000, nullptr, nullptr, nullptr, 0, false, false, 60000 },
    { "f_trap_bridge", STEP_USE, "bridge_electric_trap", nullptr, nullptr, 1000, nullptr, nullptr, nullptr, 0, false, false, 90000 },
    // Der Riese teleporters (maps\zombie_cod5_factory_teleporter): a pad use starts a 30 s countdown (level.teleport[i]
    // "timer_on", no level notify), the mainframe trigger_teleport_core then links it: flag_set teleporter_pad_link_<n>,
    // n = links so far. The pad step passes on the use press alone; the link step after it is the proof.
    // f_tp_go (count 1): a paid teleport (1500) from pad 0, passes on the teleport.
    { "f_tp_pad_e", STEP_USE, "trigger_teleport_pad_0", nullptr, nullptr, 0, nullptr, nullptr, nullptr, 0, false, false, 90000 },
    { "f_tp_pad_s", STEP_USE, "trigger_teleport_pad_1", nullptr, nullptr, 0, nullptr, nullptr, nullptr, 0, false, false, 90000 },
    { "f_tp_pad_w", STEP_USE, "trigger_teleport_pad_2", nullptr, nullptr, 0, nullptr, nullptr, nullptr, 0, false, false, 90000 },
    { "f_tp_link1", STEP_USE, "trigger_teleport_core", nullptr, nullptr, 0, "teleporter_pad_link_1", nullptr, nullptr, 0, false, false, 30000 },
    { "f_tp_link2", STEP_USE, "trigger_teleport_core", nullptr, nullptr, 0, "teleporter_pad_link_2", nullptr, nullptr, 0, false, false, 30000 },
    { "f_tp_link3", STEP_USE, "trigger_teleport_core", nullptr, nullptr, 0, "teleporter_pad_link_3", nullptr, nullptr, 0, false, false, 30000 },
    { "f_tp_go", STEP_USE, "trigger_teleport_pad_0", nullptr, nullptr, 1500, nullptr, nullptr, nullptr, 1, false, false, 90000 },
    // L37: Verruckt (zombie_cod5_asylum). Keys from the map's entity string; zones maps\zombie_cod5_asylum.gsc:137-150.
    // The two start rooms join only through electric_door (opens on power_on). Power: use_master_switch (gsc:1099).
    // Gas traps: gas_access, 1000 after power; the script keeps its own in_use (no zombie_trap), so a v_ trap passes on payment.
    // Two kar98k wall buys (866 -215 south start, 1231 58 north start): the nearest from the south start is the
    // north-side one behind the wall, so v_wall_kar picks the south one by its target.
    { "v_door_n0", STEP_USE, "zombie_door", "script_flag", "north_door1", 750, "north_door1", nullptr, nullptr, 0, false, false, 90000 },
    { "v_debris_n", STEP_USE, "zombie_debris", "script_flag", "north_upstairs_blocker", 1000, "north_upstairs_blocker", nullptr, nullptr, 0, false, false, 90000 },
    { "v_door_n1", STEP_USE, "zombie_door", "script_flag", "upstairs_north_door1", 750, "upstairs_north_door1", nullptr, nullptr, 0, false, false, 90000 },
    { "v_door_n2", STEP_USE, "zombie_door", "script_flag", "upstairs_north_door2", 1000, "upstairs_north_door2", nullptr, nullptr, 0, false, false, 90000 },
    { "v_door_boxn", STEP_USE, "zombie_door", "script_flag", "magic_box_north", 750, "magic_box_north", nullptr, nullptr, 0, false, false, 90000 },
    { "v_debris_s", STEP_USE, "zombie_debris", "script_flag", "south_upstairs_blocker", 1000, "south_upstairs_blocker", nullptr, nullptr, 0, false, false, 90000 },
    { "v_door_s1", STEP_USE, "zombie_door", "script_flag", "south_access_1", 750, "south_access_1", nullptr, nullptr, 0, false, false, 90000 },
    { "v_door_boxs", STEP_USE, "zombie_door", "script_flag", "magic_box_south", 750, "magic_box_south", nullptr, nullptr, 0, false, false, 90000 },
    { "v_power", STEP_USE, "use_master_switch", nullptr, nullptr, 0, "power_on", nullptr, nullptr, 0, false, false, 90000 },
    { "v_trap_s", STEP_USE, "gas_access", "script_noteworthy", "south_gas_valves", 1000, nullptr, nullptr, nullptr, 0, false, false, 60000 },
    { "v_trap_n", STEP_USE, "gas_access", "script_noteworthy", "north_gas_valves", 1000, nullptr, nullptr, nullptr, 0, false, false, 60000 },
    // L37: walk into the gas trap's damage volume (the gas_access target, trigger_on while it burns): PASS on the
    // electrocution shellshock (player_elec_damage, shocktime 2.5).
    { "v_trap_touch_s", STEP_TOUCH, "auto242", nullptr, nullptr, 0, nullptr, nullptr, nullptr, 0, false, false, 20000 },
    { "v_trap_touch_n", STEP_TOUCH, "auto246", nullptr, nullptr, 0, nullptr, nullptr, nullptr, 0, false, false, 20000 },
    { "v_wall_kar", STEP_USE, "weapon_upgrade", "target", "pf175_auto1", 200, nullptr, nullptr, "zombie_kar98k", 0, false, false, 90000 },
    { "v_wall_garand", STEP_USE, "weapon_upgrade", "zombie_weapon_upgrade", "zombie_m1garand", 600, nullptr, nullptr, "zombie_m1garand", 0, false, false, 90000 },
    { "v_wall_mp40", STEP_USE, "weapon_upgrade", "zombie_weapon_upgrade", "mp40_zm", 1000, nullptr, nullptr, "mp40_zm", 0, false, false, 90000 },
    { "v_betty", STEP_USE, "betty_purchase", nullptr, nullptr, 1000, nullptr, nullptr, "mine_bouncing_betty", 0, false, false, 90000 },
    // L36: Shi No Numa (zombie_cod5_sumpf). Entity keys and origins from the map's entity string; scripts are the patch
    // zone's maps\zombie_cod5_sumpf*.gsc. Start zone center_building_upstairs (zombie_cod5_sumpf.gsc:87). No power, no PaP.
    // Type 99 wall buy upstairs (10661 395 -477), stair debris (10124 373 -478, flag unlock_hospital_downstairs),
    // NW door (9556 1270, flag nw_magic_box starts the flogger: magic_box.gsc:42-56), flogger buy (pendulum_buy_trigger,
    // trap_pendulum.gsc:78), NW hut (8316 3030, flag northwest_building_unlocked), the hut's random perk (perks.gsc:40-55;
    // passes on score spent), the hut's electric trap switch (target north_west_tgt, trap_perk_electric.gsc:68,101),
    // NE door (10704 1359, flag ne_magic_box starts the zipline: magic_box.gsc:61-65), zipline lever (free, zipline.gsc:99-124)
    // and the platform's buy (the nonstatic trigger: the static one is trigger_off'd at zipline.gsc:68 until a ride; 1500 after 40 s, zipline.gsc:136-151,230-260). Traps pass on score spent (not the k_trap_ path).
    { "s_wall_type99", STEP_USE, "weapon_upgrade", "zombie_weapon_upgrade", "zombie_type99_rifle", 200, nullptr, nullptr, "zombie_type99_rifle", 0, false, false, 90000 },
    { "s_debris_stairs", STEP_USE, "zombie_debris", "script_flag", "unlock_hospital_downstairs", 1000, "unlock_hospital_downstairs", nullptr, nullptr, 0, false, false, 90000 },
    { "s_door_nw", STEP_USE, "zombie_door", "script_flag", "nw_magic_box", 1000, "nw_magic_box", nullptr, nullptr, 0, false, false, 90000 },
    { "s_trap_flogger", STEP_USE, "pendulum_buy_trigger", nullptr, nullptr, 750, nullptr, nullptr, nullptr, 0, false, false, 60000 },
    { "s_hut_nw", STEP_USE, "zombie_door", "script_flag", "northwest_building_unlocked", 750, "northwest_building_unlocked", nullptr, nullptr, 0, false, false, 90000 },
    { "s_perk_hut", STEP_USE, "zombie_vending", nullptr, nullptr, 500, nullptr, nullptr, nullptr, 0, false, false, 90000 },
    { "s_trap_elec_nw", STEP_USE, "elec_trap_trig", "target", "north_west_tgt", 1000, nullptr, nullptr, nullptr, 0, false, false, 60000 },
    { "s_door_ne", STEP_USE, "zombie_door", "script_flag", "ne_magic_box", 1000, "ne_magic_box", nullptr, nullptr, 0, false, false, 90000 },
    { "s_zip_lever", STEP_USE, "zip_lever_trigger", nullptr, nullptr, 0, "machine_off", nullptr, nullptr, 0, false, false, 90000 },
    { "s_zip_ride", STEP_USE, "zipline_buy_trigger", "script_noteworthy", "nonstatic", 1500, "machine_off", nullptr, nullptr, 0, false, false, 150000 },
    // L34: Nacht der Untoten (zombie_cod5_prototype). Entity keys from the map's entity string; wall costs from
    // maps\zombie_cod5_prototype.gsc include_weapons (add_zombie_weapon); zones from prototype_zone_init. The
    // weapon cabinet is the level script's own weapon_cabinet_think (first buy 1500 opens the doors).
    { "n_wall_kar98k", STEP_USE, "weapon_upgrade", "zombie_weapon_upgrade", "zombie_kar98k", 200, nullptr, nullptr, "zombie_kar98k", 0, false, false, 90000 },
    { "n_wall_carbine", STEP_USE, "weapon_upgrade", "zombie_weapon_upgrade", "zombie_m1carbine", 600, nullptr, nullptr, "zombie_m1carbine", 0, false, false, 90000 },
    { "n_wall_thompson", STEP_USE, "weapon_upgrade", "zombie_weapon_upgrade", "zombie_thompson", 1200, nullptr, nullptr, "zombie_thompson", 0, false, false, 90000 },
    { "n_wall_dbl", STEP_USE, "weapon_upgrade", "zombie_weapon_upgrade", "zombie_doublebarrel", 1200, nullptr, nullptr, "zombie_doublebarrel", 0, false, false, 90000 },
    { "n_wall_sawed", STEP_USE, "weapon_upgrade", "zombie_weapon_upgrade", "zombie_doublebarrel_sawed", 1200, nullptr, nullptr, "zombie_doublebarrel_sawed", 0, false, false, 90000 },
    { "n_wall_shotgun", STEP_USE, "weapon_upgrade", "zombie_weapon_upgrade", "zombie_shotgun", 1500, nullptr, nullptr, "zombie_shotgun", 0, false, false, 90000 },
    { "n_wall_bar", STEP_USE, "weapon_upgrade", "zombie_weapon_upgrade", "zombie_bar", 1800, nullptr, nullptr, "zombie_bar", 0, false, false, 90000 },
    { "n_wall_stiel", STEP_USE, "weapon_upgrade", "zombie_weapon_upgrade", "stielhandgranate", 250, nullptr, nullptr, nullptr, 0, false, false, 90000 },
    // Nacht's box never moves (magic_chest_movable 0): ten pulls sample its list for ray_gun_zm / thundergun_zm.
    { "n_box10", STEP_BOX, "treasure_chest_use", nullptr, nullptr, 950, nullptr, nullptr, nullptr, 10, false, false, 400000 },
    { "n_cabinet", STEP_USE, "weapon_cabinet_use", nullptr, nullptr, 1500, nullptr, nullptr, "kar98k_scoped_zombie", 0, false, false, 90000 },
    { "n_door_box", STEP_USE, "zombie_door", "script_flag", "start_2_box", 1000, "start_2_box", nullptr, nullptr, 0, false, false, 90000 },
    { "n_debris_box_up", STEP_USE, "zombie_debris", "script_flag", "box_2_upstairs", 1000, "box_2_upstairs", nullptr, nullptr, 0, false, false, 90000 },
    { "n_debris_start_up", STEP_USE, "zombie_debris", "script_flag", "start_2_upstairs", 1000, "start_2_upstairs", nullptr, nullptr, 0, false, false, 90000 },
    // L39: Ascension (zombie_cosmodrome). Keys and costs from the map's entity string (build/L39/ents.txt); zones
    // maps\zombie_cosmodrome.gsc:468-555. The intro lander carries the players (linked) for ~10 s of level time and lands
    // at lander_station5 (centrifuge_zone). Perks PhD Flopper (flakjacket) and Stamin-Up (longersprint), 2000 each.
    { "c_intro", STEP_WAIT, nullptr, nullptr, nullptr, 0, nullptr, nullptr, nullptr, 12000, false, false, 20000 },
    { "c_door_c2p", STEP_USE, "zombie_door", "script_flag", "centrifuge2power", 750, "centrifuge2power", nullptr, nullptr, 0, false, false, 90000 },
    { "c_door_p2c", STEP_USE, "zombie_door", "script_flag", "power2centrifuge", 1250, "power2centrifuge", nullptr, nullptr, 0, false, false, 90000 },
    { "c_door_roof", STEP_USE, "zombie_door", "script_flag", "power_interior_2_roof", 1000, "power_interior_2_roof", nullptr, nullptr, 0, false, false, 90000 },
    { "c_door_catwalk", STEP_USE, "zombie_door", "script_flag", "power_catwalk_access", 1250, "power_catwalk_access", nullptr, nullptr, 0, false, false, 90000 },
    { "c_door_base", STEP_USE, "zombie_door", "script_flag", "base_entry_2_power", 1250, "base_entry_2_power", nullptr, nullptr, 0, false, false, 90000 },
    { "c_door_storage", STEP_USE, "zombie_door", "script_flag", "base_entry_2_storage", 1000, "base_entry_2_storage", nullptr, nullptr, 0, false, false, 90000 },
    { "c_power", STEP_USE, "use_elec_switch", nullptr, nullptr, 0, "power_on", nullptr, nullptr, 0, false, false, 90000 },
    // Lunar lander (maps\zombie_cosmodrome_lander.gsc): a call box brings the lander to its station, zip_buy (250) rides it back to
    // lander_station5. A call passes on lander_takeoff (lander_grounded also notifies on flag_clear at takeoff), a ride on the
    // station flag the purchase sets; c_land waits out the flight (the player is linked). Rides from 1/3/4 set
    // lander_a/b/c_used -> launch_activated -> trig_launch_rocket (look-at) -> launch_complete opens the Pack-a-Punch room.
    { "c_door_shed", STEP_USE, "zombie_door", "script_flag", "catwalks_2_shed", 1250, "catwalks_2_shed", nullptr, nullptr, 0, false, false, 90000 },
    { "c_door_slander", STEP_USE, "zombie_door", "script_flag", "storage_lander_area", 1250, "storage_lander_area", nullptr, nullptr, 0, false, false, 90000 },
    { "c_land", STEP_WAIT, nullptr, nullptr, nullptr, 0, nullptr, nullptr, nullptr, 25000, false, false, 40000 },
    { "c_call1", STEP_USE, "zip_call_box", "script_noteworthy", "lander_station1", 0, "lander_takeoff", nullptr, nullptr, 0, false, false, 120000 },
    { "c_ride1", STEP_USE, nullptr, "script_noteworthy", "zip_buy", 250, "lander_a_used", nullptr, nullptr, 0, false, false, 120000 },
    { "c_call3", STEP_USE, "zip_call_box", "script_noteworthy", "lander_station3", 0, "lander_takeoff", nullptr, nullptr, 0, false, false, 120000 },
    { "c_ride3", STEP_USE, nullptr, "script_noteworthy", "zip_buy", 250, "lander_b_used", nullptr, nullptr, 0, false, false, 120000 },
    { "c_call4", STEP_USE, "zip_call_box", "script_noteworthy", "lander_station4", 0, "lander_takeoff", nullptr, nullptr, 0, false, false, 120000 },
    { "c_ride4", STEP_USE, nullptr, "script_noteworthy", "zip_buy", 250, "lander_c_used", nullptr, nullptr, 0, false, false, 120000 },
    { "c_launch", STEP_USE, "trig_launch_rocket", nullptr, nullptr, 0, "launch_complete", nullptr, nullptr, 0, false, false, 150000 },
    // L39: the PaP control room opens only to under_rocket, and that only to north_path (patch zone table
    // maps_zombie_cosmodrome.gsc:498-499); north_path's door from base_entry_zone2 (155 1863 -124, cost 1250).
    { "c_door_north", STEP_USE, "zombie_door", "script_flag", "base_entry_2_north_path", 1250, "base_entry_2_north_path", nullptr, nullptr, 0, false, false, 90000 },
    { "c_pap", STEP_PAP, "zombie_vending_upgrade", nullptr, nullptr, 5000, nullptr, nullptr, nullptr, 0, false, false, 120000 },
    { "c_phd", STEP_USE, "zombie_vending", "script_noteworthy", "specialty_flakjacket", 2000, nullptr, "specialty_flakjacket", nullptr, 0, false, false, 90000 },
    { "c_staminup", STEP_USE, "zombie_vending", "script_noteworthy", "specialty_longersprint", 2000, nullptr, "specialty_longersprint", nullptr, 0, false, false, 90000 },
    // L41: Shangri-La (zombie_temple). Doors / costs / flags from the map's entity string and maps\zombie_temple.gsc:254-288;
    // power is two free switches (maps\zombie_temple_power.gsc:22-132, each sets <side>_switch_pulled, both -> power_on);
    // geysers (patch elevators.gsc:179-302) launch a player who stands 1.8 s in the trigger and set <noteworthy>_active;
    // the minecart lever (minecart.gsc:732-781, 250) needs power and a waterfall door and sets players_riding_minecart.
    { "t_door_pressure", STEP_USE, nullptr, "script_flag", "start_to_pressure", 750, "start_to_pressure", nullptr, nullptr, 0, false, false, 90000 },
    { "t_door_cave01", STEP_USE, nullptr, "script_flag", "pressure_to_cave01", 1000, "pressure_to_cave01", nullptr, nullptr, 0, false, false, 90000 },
    { "t_door_cave02", STEP_USE, nullptr, "script_flag", "cave01_to_cave02", 1000, "cave01_to_cave02", nullptr, nullptr, 0, false, false, 90000 },
    { "t_door_power", STEP_USE, nullptr, "script_flag", "cave03_to_power", 1250, "cave03_to_power", nullptr, nullptr, 0, false, false, 90000 },
    { "t_power_l", STEP_USE, "power_trigger_left", nullptr, nullptr, 0, "left_switch_pulled", nullptr, nullptr, 0, false, false, 90000 },
    { "t_power_r", STEP_USE, "power_trigger_right", nullptr, nullptr, 0, "power_on", nullptr, nullptr, 0, false, false, 90000 },
    { "t_door_wfup", STEP_USE, nullptr, "script_flag", "start_to_waterfall_upper", 750, "start_to_waterfall_upper", nullptr, nullptr, 0, false, false, 90000 },
    { "t_door_wf", STEP_USE, nullptr, "script_flag", "cave_water_to_waterfall", 1000, "cave_water_to_waterfall", nullptr, nullptr, 0, false, false, 90000 },
    { "t_door_tunnel", STEP_USE, nullptr, "script_flag", "waterfall_to_tunnel", 1000, "waterfall_to_tunnel", nullptr, nullptr, 0, false, false, 90000 },
    // start_geyser runs once cave02_to_cave_water, cave_water_to_power or cave_water_to_waterfall is open (elevators.gsc:540).
    { "t_door_cwp", STEP_USE, nullptr, "script_flag", "cave_water_to_power", 1250, "cave_water_to_power", nullptr, nullptr, 0, false, false, 90000 },
    { "t_geyser", STEP_TOUCH, "temple_geyser", "script_noteworthy", "start_geyser", 0, "start_geyser_active", nullptr, nullptr, 0, false, false, 60000 },
    // The puller need not ride: players touching minecart1_start_volume ~0.5 s after the pull are linked in (minecart.gsc:282,
    // 306, 522-527); the lever passes on the 250 spent, the rider walks into the volume, a rider's minecart_exit (:1014) ends it.
    { "t_minecart", STEP_USE, "minecart_lever_trigger", nullptr, nullptr, 250, nullptr, nullptr, nullptr, 0, false, false, 90000 },
    { "t_minecart_ride", STEP_TOUCH, "minecart1_start_volume", nullptr, nullptr, 0, "players_riding_minecart", nullptr, nullptr, 0, false, false, 20000, 1 },
    { "t_minecart_end", STEP_WAIT, nullptr, nullptr, nullptr, 0, "minecart_exit", nullptr, nullptr, 0, false, false, 60000, 1 },
    // L41 rows in a player's order. Spear traps: a touch fires the spikes 3x; each fire sets the clip's
    // _CF_SCRIPTMOVER_CLIENT_FLAG_SPIKES (3) (zombie_temple_traps.gsc:69-82, 254-265; zombie_temple.gsc:179), no notify.
    { "t_spears", STEP_TOUCH, "spear_trap", nullptr, nullptr, 0, "scriptmover_clientflag_3", nullptr, nullptr, 0, false, false, 30000 },
    // Waterslide: the crouch trigger at the top, the stand trigger at the bottom notifies the player water_slide_exit
    // (zombie_temple_waterslide.gsc:826-850).
    { "t_slide", STEP_TOUCH, "cave_slide_force_crouch", nullptr, nullptr, 0, "water_slide_exit", nullptr, nullptr, 0, false, false, 60000 },
    // Maze: touching a start cell (script_string start) starts a path; level maze_path_end when the player leaves it,
    // maze_timer_end 10 s after the start (zombie_temple_traps.gsc:677-699, 1083-1098, 1610-1637).
    { "t_maze", STEP_TOUCH, "maze_trigger", "script_string", "start", 0, "maze_path_end|maze_timer_end", nullptr, nullptr, 0, false, false, 60000 },
    // PaP plates: solo, only the plate whose trigger has requiredPlayers 1 rises (random per round); stand on it until it
    // is down, then each trigger notifies pap_active (zombie_temple_pack_a_punch.gsc:259-268, 318, 333-453).
    { "t_pap_plate", STEP_TOUCH, "pap_blocker_trigger*", "requiredplayers", "1", 0, "pap_active", nullptr, nullptr, 0, false, false, 90000 },
    // Shrink Ray (run with bo1_testclient_fightweapons shrink_ray_zm): a hit zombie notifies shrink; touching a
    // shrunk zombie kicks it (kicked) or steps on it (stepped_on) (_zombiemode_weap_shrink_ray.gsc:220-221, 539-566).
    { "t_shrink", STEP_WAIT, nullptr, nullptr, nullptr, 0, "shrink", nullptr, nullptr, 0, false, false, 60000, 2 },
    { "t_kick", STEP_WAIT, nullptr, nullptr, nullptr, 0, "kicked|stepped_on", nullptr, nullptr, 0, false, false, 60000, 2 },
    // L42: Moon (zombie_moon). The Area 51 pad is the info_volume nml_teleporter (14057 -15356 -571): every player inside
    // for 2.5 s teleports the team to the moon, level notify stop_ramp then level.on_the_moon (maps_zombie_moon_teleporter
    // .gsc:33-172, 525-528; wasteland.gsc:129-147). The bunker_gate over it opens 20 s after enter_nml (teleporter.gsc:806-822).
    { "m_jump", STEP_JUMP, nullptr, nullptr, nullptr, 0, nullptr, nullptr, nullptr, 3, false, false, 30000 },
    { "m_to_moon", STEP_TOUCH, "nml_teleporter", nullptr, nullptr, 0, "stop_ramp", nullptr, nullptr, 0, false, false, 90000 },
    // The astronaut (spawns only while level.on_the_moon, _zombiemode_ai_astro.gsc astro_zombie_manager) grabs the player
    // (headbutt_anim notetrack "grabbed", astro_turn_player) and on "fire" astro_zombie_teleport_enemy SetOrigins them to a
    // struct_black_hole_teleport (:540-557, 670-790). astro_teleport = the harness saw the player move > 200 units within
    // 1 s of an astronaut's "fire" notetrack.
    { "m_astro", STEP_WAIT, nullptr, nullptr, nullptr, 0, "astro_teleport", nullptr, nullptr, 0, false, false, 300000, 2 },
    // L42 route rows in the order a player meets them (zombie_moon_patch unless marked M = zombie_moon, C = common_zombie_patch).
    // NML dogs: they spawn only while a player is inside info_volume nml_dogs_volume, from 30 s after the start
    // (wasteland.gsc:61-62, 93-98, 155-160); the first one sets flag dog_clips (C _zombiemode_ai_dogs.gsc:887-918).
    { "m_nml_dogs", STEP_TOUCH, "nml_dogs_volume", nullptr, nullptr, 0, "dog_clips", nullptr, nullptr, 0, false, false, 90000 },
    // P.E.S.: the free helmet pickup next to the moon landing (C _zombiemode_equipment.gsc:108-137, 191-217).
    { "m_pes", STEP_USE, "zombie_equipment_upgrade", "target", "pf1326_auto37", 0, nullptr, nullptr, "equip_gasmask_zm", 0, false, false, 60000 },
    // Airlocks and the tunnel door on the cheapest way to power (M utility.gsc:129-201 sets script_flag; moon.gsc:962-985).
    // Putting the P.E.S. on: raising equip_gasmask_zm is the activation (C _zombiemode_equipment.gsc:228-275).
    { "m_pes_on", STEP_EQUIP, nullptr, nullptr, nullptr, 0, "equip_gasmask_zm_activate", nullptr, "equip_gasmask_zm", 0, false, false, 20000 },
    // Oxygen (run without god mode and without the P.E.S.: IsGodMode skips it): on the moon before power, 15 s without
    // the helmet does DoDamage(health * 10) (gravity.gsc:662, 716-753); solo without Quick Revive that is level fake_death
    // (r9: moon at 27900, fake_death at 43000), else player_downed.
    { "m_oxygen", STEP_WAIT, nullptr, nullptr, nullptr, 0, "fake_death|player_downed", nullptr, nullptr, 0, false, false, 60000 },
    { "m_air_rx", STEP_USE, "zombie_airlock_buy", "script_flag", "receiving_exit", 750, "receiving_exit", nullptr, nullptr, 0, false, false, 90000 },
    { "m_air_cw", STEP_USE, "zombie_airlock_buy", "script_flag", "catacombs_west", 750, "catacombs_west", nullptr, nullptr, 0, false, false, 90000 },
    { "m_door_t6", STEP_USE, "zombie_door", "script_flag", "tunnel_6_door1", 1000, "tunnel_6_door1", nullptr, nullptr, 0, false, false, 90000 },
    { "m_air_cw4", STEP_USE, "zombie_airlock_buy", "script_flag", "catacombs_west4", 1250, "catacombs_west4", nullptr, nullptr, 0, false, false, 90000 },
    // L42g r13: the Hacker (four of its six spots), Mule (1481 3450 -65) and the teleporter back to Earth (tower_zone_east)
    // are east of the generator room: airlocks generator_exit_east, then dig_enter_east, then exit_dig_east (moon.gsc:992-998).
    { "m_air_ge", STEP_USE, "zombie_airlock_buy", "script_flag", "generator_exit_east", 750, "generator_exit_east", nullptr, nullptr, 0, false, false, 90000 },
    { "m_air_de", STEP_USE, "zombie_airlock_buy", "script_flag", "dig_enter_east", 1000, "dig_enter_east", nullptr, nullptr, 0, false, false, 90000 },
    { "m_air_xe", STEP_USE, "zombie_airlock_buy", "script_flag", "exit_dig_east", 1000, "exit_dig_east", nullptr, nullptr, 0, false, false, 90000 },
    // The Hacker: one of six zombie_equipment_upgrade triggers (key zombie_equipment_upgrade equip_hacker_zm) is kept at
    // random, the others deleted (M utility.gsc:727-771); the pickup is free (C _zombiemode_equipment.gsc:108-137).
    { "m_hacker", STEP_USE, "zombie_equipment_upgrade", "zombie_equipment_upgrade", "equip_hacker_zm", 0, nullptr, nullptr, "equip_hacker_zm", 0, false, false, 150000 },
    // Wave Gun (fightweapons alt:microwavegundw_zm): a kill with it raised = wavegun_kill (see the zom_kill hook); microwaved
    // is non-AI objects only (M _zombiemode_weap_microwavegun.gsc:293). Quantum Bomb (fightweapons zombie_quantum_bomb): the thrown grenade's
    // explode picks a result (quantum_bomb.gsc:236-275); quantum_bomb_explode = the harness saw a thrown one explode.
    { "m_wavegun", STEP_WAIT, nullptr, nullptr, nullptr, 0, "wavegun_kill|microwaved", nullptr, nullptr, 0, false, false, 120000, 2 },
    { "m_qed", STEP_WAIT, nullptr, nullptr, nullptr, 0, "quantum_bomb_explode", nullptr, nullptr, 0, false, false, 120000, 2 },
    // Excavators: 20 s after power, one digger starts within two rounds (flag start_<name>_digger, digger.gsc:90-134).
    // L42g r17: the guaranteed digger comes at the 4th between_round_over after power + 20 s (rnd > 2, digger.gsc:107-121);
    // r17 timed out 2.5 s before it at 400 s.
    { "m_digger", STEP_WAIT, nullptr, nullptr, nullptr, 0, "start_teleporter_digger|start_hangar_digger|start_biodome_digger", nullptr, nullptr, 0, false, false, 600000, 2 },
    // Back to Earth: info_volume generator_teleporter; landing in NML notifies enter_nml (teleporter.gsc:66-153, 494-519).
    // Its gate closes on the moon landing and reopens the next round end + 120 s (:715-785), hence the long timeout.
    { "m_to_earth", STEP_TOUCH, "generator_teleporter", nullptr, nullptr, 0, "enter_nml", nullptr, nullptr, 0, false, false, 400000 },

};

const char *s_fullPlan =
    "qr wall_rottweil fire3 ammo_rottweil door_conf debris_elev2 floor2 debris_stair debris_elev1 battery "
    "floor3 power box box_teddy floor2 jugg fire12 doubletap fire12 defcon defcon defcon defcon portal_pack pap "
    "portal floor1 fire12 speedcola fire12 trap_fix trap_qr";

// Spawn keys the scripts read as entity fields, and the level fields the plan reads (find-only).
const char *s_fieldNames[] = { "zombie_weapon_upgrade", "script_flag", "powerup_name", "_trap_piece", "defcon_level",
    "zombie_cost", "chest_moves", "zombie_vars", "zombie_insta_kill", "zombie_point_scalar",
    "state", "script_parameters", "script_flag_wait", "chest_accessed", "zombie_powerup_fire_sale_on",
    "zombie_powerup_bonfire_sale_on", "phone_counter", "music_override", "cost", "has_legs", "round_number",
    "devcheater", "zombie_devgui_power", "powerup_drop_count", "flag", "next_thief_round", "_trap_in_use", "in_use",
    "script_string", "requiredplayers", "zombie_equipment_upgrade" };
enum { F_WEAPON, F_FLAG, F_POWERUP, F_TRAPPIECE, F_DEFCON, F_COST, F_CHESTMOVES,
    F_ZOMBIEVARS, F_INSTA, F_SCALAR, F_BOARDSTATE, F_PARAMETERS, F_FLAGWAIT, F_CHESTACCESSED, F_FIRESALE,
    F_BONFIRE, F_PHONE, F_MUSIC, F_PAPCOST, F_HASLEGS, F_ROUND,
    F_DEVCHEATER, F_DEVGUIPOWER, F_DROPCOUNT, F_LEVELFLAG, F_NEXTTHIEF, F_TRAPINUSE, F_INUSE, F_SCRIPTSTRING, F_REQPLAYERS, F_EQUIPUPGRADE, F_COUNT };
unsigned int s_fieldKeys[F_COUNT];
int s_featureState[4] = { -2, -2, -2, -2 };

const dvar_s *s_planDvar;
const dvar_s *s_planAltDvar; // L37: bo1_testclient_plan_alt (a map with a random start room, Verruckt)
const dvar_s *s_planAltYDvar; // L37: bo1_testclient_plan_alty
const dvar_s *s_resupplyDvar;
const dvar_s *s_dryAmmoDvar;
int s_dryStart = -1;
const dvar_s *s_scoreDvar;
const dvar_s *s_quitDoneDvar;
const dvar_s *s_shotsDvar; // w1 c11: bo1_testclient_shots
const dvar_s *s_thiefStrafeDvar; // L10: bo1_testclient_thiefstrafe
const dvar_s *s_thiefKillDvar; // L18: bo1_testclient_thiefkill
int s_completeTime;
bool s_quitSent;

std::vector<int> s_plan;
// r1 arm policy: per plan entry, PLAN_INSERTED (a recovery step the harness added: logged "end recovery PASS/FAIL",
// never counted as the route's own) and PLAN_ARMED (the step-boundary arm check already ran for it).
enum { PLAN_INSERTED = 1, PLAN_ARMED = 2 };
std::vector<unsigned char> s_planFlags;
std::vector<unsigned int> s_planReplace; // inserted wall buy: the held gun it replaces (selected before the use)
unsigned int s_holdWeapon;  // the gun the plan keeps raised though it is empty (box / wall buy replacement)
bool s_hadTwoGuns;          // the player held two primaries at some point on this map
int s_recoveryFails;        // failed inserted steps; the arm policy stops inserting after two
bool s_thiefReturn;         // the thief round ended: re-arm and return to the interrupted step's floor
int s_thiefStepFloor;
int s_floorRequeues;        // q1 c8: times the current step was restarted behind a ride to its target's floor
int s_waitResupplies;       // q1 c9: times the current wait was restarted behind an ammo buy
int s_papRequeues;          // L20: times pap went back through portal_pack (at most 2 a run)
int s_lastTrapUse = -1;     // q1 c9: the last trap switch a route step used (the wait watches its trap)
int s_lastTrapUseTime = -1; // L20: level time of that use
bool s_planParsed;
int s_index;
int s_stepStart;
int s_prevStepStart; // L41: the previous step's s_stepStart (PlanStep::notifyFrom 1)
int s_ent = -1;
int s_quantumMissile = -1; // L42: the last thrown zombie_quantum_bomb
int s_waitTrap = -1; // r1 c6: the trap volume a STEP_WAIT / STEP_TRAP_VIEW watches (set in its phase 0)
int s_phase;
int s_phaseTime;
int s_startScore;
int s_startAmmo;
int s_spent;
// r1 c3: the largest one-frame score drop since the step started (a purchase), and last frame's score. q111m2: the
// trap_qr buy (1000) was missed because kills earlier in the step had raised the score by 80 (net spent 920).
int s_largestDrop;
int s_lastScore;
int s_uses;
int s_lastUse;
int s_defconUseTime;
int s_spins;
float s_startOrigin[3];
float s_rideCarZ;    // elevator car centre z last frame while riding
int s_rideCarMoved;  // level.time the car last moved while riding
int s_rideCar = -1;
bool s_ridePanelUsed;
float s_lastOrigin[3];
float s_stand[3];
int s_teleportTime;
int s_kinoOutTime;    // L28: level time k_tp_go saw the teleport out
int s_kinoReturnTime; // L28: last teleport that landed on Kino's theater return points (theater_teleport_player*)
int s_claymoreKillTime; // last actor death whose death notify carried claymore_zm
unsigned int s_startWeapons[15];
bool s_complete;
std::vector<int> s_usedEnts;
std::vector<int> s_crawlersSeen; // q1 c15: crawlers the crawler step already observed for 20 s

// walking
path_t s_path;
int s_pathGoal = -1;
int s_pathTime;
int s_pathPoint = -1;
int s_pathFound = -1;
float s_progressOrigin[3];
int s_progressTime;
int s_unstickUntil;
int s_unstickDir = 1;
// L36: the path's last node can sit behind solid geometry from a use trigger (sumpf2: zombie_vending 335 moved by
// randomize_vending_machines to 8510 3187; Path_FindPath ended at 8430 3345, behind brush 1626, 90 s without a hint).
// After 5 s stalled near a trigger goal it cannot see, Walk avoids that spot and takes the nearest node around the
// goal that has sight of it.
int s_stallGoal = -1;
int s_stallTime;
float s_stallDist;
int s_avoidGoal = -1;
float s_avoidOrigin[3];

// observations
struct Seen
{
    std::string name;
    int time;
};
std::vector<Seen> s_levelNotifies;
int s_triggerNotify[1024];
struct Powerup
{
    int ent;
    int useCount;
    int dropTime;
    std::string name;
    bool grabbed;
    float nearest; // the test player's closest approach (xy) while it existed
};
std::vector<Powerup> s_powerups;
int s_powerupsGrabbed;
unsigned int s_ammoWeapons[15];
int s_reserves[15];
int s_effectScore;
int s_chestAccessed = -1, s_chestMoves = -1;
int s_boardState[1024]; // 0 unknown, 1 intact, 2 destroyed
int s_boardCounts[3]; // board chunks now "repaired" / "destroyed" / any other state (mid_tear, mid_repair)

// Carpenter evidence: how many exterior board chunks are destroyed at a powerup pickup and afterwards.
void LogBoardCounts(const char *why)
{
    Com_Printf(16, "bo1_effect: time %d boards %s repaired %d destroyed %d other %d\n", level.time, why,
        s_boardCounts[0], s_boardCounts[1], s_boardCounts[2]);
}

// shot / reload measurement (current weapon)
unsigned int s_measureWeapon;
int s_lastClip = -1;
int s_lastShotTime;
int s_shots, s_intervalSum, s_intervalCount;
int s_reloadStart;
int s_reloadSum, s_reloadCount;
int s_stepShots;
// fire-timer rate this step: weaponTime run down per ms of level time while firing / rechambering
// (1.0 without Double Tap; 1/perk_weapRateMultiplier with it), frames with an unchanged state only
int s_fireDec, s_fireDt;
int s_prevWeaponTime, s_prevWeaponState = -1, s_prevTime;

bool Enabled()
{
    return Sys_IsHeadless() && G_SP_IsZombieMode() && s_planDvar && s_planDvar->current.string
        && s_planDvar->current.string[0] && Dvar_GetBool("bo1_testclient") && Dvar_GetBool("bo1_testclient_fight");
}

const char *Str(unsigned int id)
{
    return id ? SL_ConvertToString(id, SCRIPTINSTANCE_SERVER) : "";
}

// Find-only read of an entity or level field: never creates variables or strings.
bool ReadField(unsigned int object, int field, std::string *text, int *number)
{
    const unsigned int key = s_fieldKeys[field];
    const unsigned int id = object && key ? FindVariable(SCRIPTINSTANCE_SERVER, object, key) : 0;
    if (!id)
        return false;
    const unsigned int type = GetValueType(SCRIPTINSTANCE_SERVER, id);
    const VariableUnion &value = GetVariableValueAddress(SCRIPTINSTANCE_SERVER, id)->u;
    if (type == VAR_STRING || type == VAR_ISTRING)
    {
        if (text)
            *text = Str(value.stringValue);
        return true;
    }
    if (type == VAR_INTEGER)
    {
        if (number)
            *number = value.intValue;
        if (text)
            *text = std::to_string(value.intValue);
        return true;
    }
    return false;
}

unsigned int EntObject(int entnum)
{
    return FindEntityId(SCRIPTINSTANCE_SERVER, entnum, 0, 0);
}

bool EntField(int entnum, int field, std::string *text, int *number)
{
    return ReadField(EntObject(entnum), field, text, number);
}

int LevelInt(int field, int fallback)
{
    int value = fallback;
    ReadField(gScrVarPub[SCRIPTINSTANCE_SERVER].levelId, field, nullptr, &value);
    return value;
}

int ZombieVar(int field)
{
    const unsigned int id = FindVariable(SCRIPTINSTANCE_SERVER, gScrVarPub[SCRIPTINSTANCE_SERVER].levelId,
        s_fieldKeys[F_ZOMBIEVARS]);
    if (id && GetValueType(SCRIPTINSTANCE_SERVER, id) == VAR_POINTER)
    {
        // Array indices are string IDs; canonical field IDs only apply to object fields.
        const unsigned int key = SL_FindString(s_fieldNames[field], SCRIPTINSTANCE_SERVER);
        const unsigned int entry = key ? FindVariable(SCRIPTINSTANCE_SERVER,
            GetVariableValueAddress(SCRIPTINSTANCE_SERVER, id)->u.pointerValue, key) : 0;
        if (entry && GetValueType(SCRIPTINSTANCE_SERVER, entry) == VAR_INTEGER)
            return GetVariableValueAddress(SCRIPTINSTANCE_SERVER, entry)->u.intValue;
    }
    return -1;
}

static int s_maxAmmoTime = -1; // L18: last "zmb_max_ammo" notify (full_ammo_powerup, any entity)

int NotifyTime(const char *name, int since)
{
    for (const Seen &seen : s_levelNotifies)
        if (seen.time >= since && seen.name == name)
            return seen.time;
    return -1;
}

// L41: a step's notify (alternatives "a|b"), counted from PlanStep::notifyFrom.
int StepNotifyTime(const PlanStep &step)
{
    const int since = step.notifyFrom == 2 ? 0 : step.notifyFrom == 1 ? s_prevStepStart : s_stepStart;
    const char *name = step.notify;
    while (name && *name)
    {
        const char *bar = strchr(name, '|');
        const std::string one = bar ? std::string(name, bar - name) : std::string(name);
        const int time = NotifyTime(one.c_str(), since);
        if (time >= 0)
            return time;
        name = bar ? bar + 1 : nullptr;
    }
    return -1;
}

int LastNotifyTime(const char *name)
{
    int time = -1;
    for (const Seen &seen : s_levelNotifies)
        if (seen.name == name)
            time = seen.time;
    return time;
}

// r1 c3: between rounds (end_of_round seen after the last start_of_round) and the next round is a thief round
// (thief_round_tracker starts it at between_round_over when round_number >= level.next_thief_round).
bool ThiefRoundComing()
{
    const int round = LevelInt(F_ROUND, -1), next = LevelInt(F_NEXTTHIEF, -1);
    return round >= 0 && next >= 0 && next <= round + 1 && LastNotifyTime("end_of_round") > LastNotifyTime("start_of_round");
}

void Center(const gentity_s *ent, float *center)
{
    for (int i = 0; i < 3; ++i)
        center[i] = 0.5f * (ent->r.absmin[i] + ent->r.absmax[i]);
}

// L38: the brushes of a brush-model trigger (every leaf under its cmodel's leaf brush node, as
// CM_TestInLeafBrushNode_r walks them). Verruckt's south gas trap volume (*68) is two brushes at the hallway's
// ends; the centre of its bounds is in neither, so a player standing there never touches it (SV_EntityContact 0).
int TriggerBrushes(const gentity_s *ent, const cbrush_t **out, int max)
{
    if (!ent->s.index.brushmodel)
        return 0;
    const cmodel_t *cmod = CM_ClipHandleToModel(ent->s.index.brushmodel);
    cLeafBrushNode_s *stack[64];
    int top = 0, count = 0;
    stack[top++] = &cm.leafbrushNodes[cmod->leaf.leafBrushNode];
    while (top > 0 && count < max)
    {
        cLeafBrushNode_s *node = stack[--top];
        if (node->leafBrushCount > 0)
        {
            for (int k = 0; k < node->leafBrushCount && count < max; ++k)
                out[count++] = &cm.brushes[node->data.leaf.brushes[k]];
            continue;
        }
        if (node->leafBrushCount < 0 && top < 63)
            stack[top++] = node + 1;
        if (top < 62)
        {
            stack[top++] = &node[node->data.children.childOffset[0]];
            stack[top++] = &node[node->data.children.childOffset[1]];
        }
    }
    return count;
}

// L38: the point a touch step walks to: off Five, the centre of the trigger's brush nearest the player (unrotated
// brush models); otherwise, and on Five, the centre of the bounds.
void TouchPoint(const gentity_s *ent, const gentity_s *player, bool nearestBrush, float *point)
{
    Center(ent, point);
    const cbrush_t *brushes[32];
    const int count = nearestBrush && !ent->r.currentAngles[0] && !ent->r.currentAngles[1] && !ent->r.currentAngles[2]
        ? TriggerBrushes(ent, brushes, 32) : 0;
    float best = FLT_MAX;
    for (int i = 0; i < count; ++i)
    {
        float c[3];
        for (int j = 0; j < 3; ++j)
            c[j] = ent->r.currentOrigin[j] + 0.5f * (brushes[i]->mins[j] + brushes[i]->maxs[j]);
        const float d = Vec3DistanceSq(c, player->r.currentOrigin);
        if (d < best)
        {
            best = d;
            Vec3Copy(c, point);
        }
    }
}

// L28: the floor / pack-room geometry below is Five's (zombie_pentagon); on any other map (Kino) everything is
// floor 1 and there is no pack room, so the elevator / portal_pack recoveries never insert themselves.
static bool s_isFive = true;

// Floors of the Pentagon: 1 conference level, 2 war room, 3 labs.
int Floor(float z)
{
    if (!s_isFive)
        return 1;
    if (z > -150.0f)
        return 1;
    if (z > -600.0f)
        return 2;
    return 3;
}

// q1 c9: the pack room (Pack-a-Punch): x < -1650 at floor 2's height; the pads arrive at -2479 2078 -511.
bool InPackRoom(const float *origin)
{
    if (!s_isFive)
        return false;
    return origin[0] < -1650.0f && origin[2] > -600.0f && origin[2] < -400.0f;
}

bool IsTrigger(const gentity_s *ent)
{
    const char *classname = Str(ent->classname);
    return !strncmp(classname, "trigger_", 8);
}

// L42: a STEP_TOUCH may also target an info_volume (zombie_moon's nml_teleporter is one; scripts poll istouching).
bool IsPlanTarget(const gentity_s *ent, const PlanStep &step)
{
    return IsTrigger(ent) || (step.kind == STEP_TOUCH && !strcmp(Str(ent->classname), "info_volume"));
}

bool Matches(const gentity_s *ent, const PlanStep &step)
{
    if (!ent->r.inuse)
        return false;
    // L41: a targetname ending in '*' is a prefix (Shangri-La's pap_blocker_trigger1..4).
    const size_t tnLen = step.targetname ? strlen(step.targetname) : 0;
    if (tnLen && step.targetname[tnLen - 1] == '*')
    {
        if (strncmp(Str(ent->targetname), step.targetname, tnLen - 1))
            return false;
    }
    else if (step.targetname && strcmp(Str(ent->targetname), step.targetname))
        return false;
    if (!step.key)
        return true;
    if (!strcmp(step.key, "script_noteworthy"))
        return !strcmp(Str(ent->script_noteworthy), step.value);
    if (!strcmp(step.key, "target"))
        return !strcmp(Str(ent->target), step.value);
    std::string text;
    int field = !strcmp(step.key, "zombie_weapon_upgrade") ? F_WEAPON : !strcmp(step.key, "script_flag") ? F_FLAG
        : !strcmp(step.key, "script_flag_wait") ? F_FLAGWAIT : !strcmp(step.key, "script_string") ? F_SCRIPTSTRING
        : !strcmp(step.key, "requiredplayers") ? F_REQPLAYERS
        : !strcmp(step.key, "zombie_equipment_upgrade") ? F_EQUIPUPGRADE : -1; // L42: the Hacker pickup
    return field >= 0 && EntField(ent->s.number, field, &text, nullptr) && text == step.value;
}

// A trigger the scripts switched off (trigger_off / disable_trigger) is moved 10000 units down.
bool TriggerEnabled(const gentity_s *ent)
{
    float center[3];
    Center(ent, center);
    return center[2] > -5000.0f;
}

bool Used(int ent)
{
    for (int used : s_usedEnts)
        if (used == ent)
            return true;
    return false;
}

bool DefconSwitchReady(const gentity_s *ent)
{
    // Harness: retail defcon_sign_setup clears the prompt after a pull, but
    // leaves HINT_NOICON set. The cursor icon alone does not mean it can be used.
    return ent->s.un1.scale != 255;
}

bool PathReaches(const gentity_s *player, const float *pos);

gentity_s *FindTarget(const PlanStep &step, const gentity_s *player, bool requireEnabled)
{
    gentity_s *best = nullptr;
    float bestScore = 0;
    // L41 path rule (not Five): a door has a trigger on each side; when several match, one the planner reaches from
    // the player wins over a nearer one it does not (Shangri-La pressure_to_cave01: the nearer trigger, 1622 -1100
    // -281, is on the cave01 side; the walk stalled 90 s at 1340 -1020 17 above it).
    int matching = 0;
    if (!s_isFive)
    {
        for (int i = level.maxclients; i < level.num_entities && matching < 2; ++i)
        {
            gentity_s *ent = &g_entities[i];
            if (ent->r.inuse && IsPlanTarget(ent, step) && Matches(ent, step) && (!requireEnabled || TriggerEnabled(ent))
                && !(step.unique && Used(i)))
                ++matching;
        }
    }
    for (int i = level.maxclients; i < level.num_entities; ++i)
    {
        gentity_s *ent = &g_entities[i];
        if (!ent->r.inuse || !IsPlanTarget(ent, step) || !Matches(ent, step))
            continue;
        if (requireEnabled && !TriggerEnabled(ent))
            continue;
        const bool defcon = !strcmp(step.name, "defcon");
        if (defcon && !DefconSwitchReady(ent))
            continue;
        // A pack_room_reset rearms switches, including ones used earlier in the plan.
        if (step.unique && !defcon && Used(i))
            continue;
        float center[3];
        Center(ent, center);
        float score = Vec3Distance(center, player->r.currentOrigin);
        if (Floor(center[2]) != Floor(player->r.currentOrigin[2]))
            score += 100000.0f; // prefer this floor
        if (matching > 1 && !PathReaches(player, center))
            score += 50000.0f;
        if (!best || score < bestScore)
        {
            best = ent;
            bestScore = score;
        }
    }
    return best;
}

gentity_s *FindByTargetname(const char *targetname, const char *noteworthy)
{
    for (int i = level.maxclients; i < level.num_entities; ++i)
    {
        gentity_s *ent = &g_entities[i];
        if (ent->r.inuse && !strcmp(Str(ent->targetname), targetname)
            && (!noteworthy || !strcmp(Str(ent->script_noteworthy), noteworthy)))
            return ent;
    }
    return nullptr;
}

gentity_s *FindByNoteworthy(const char *noteworthy)
{
    for (int i = level.maxclients; i < level.num_entities; ++i)
    {
        gentity_s *ent = &g_entities[i];
        if (ent->r.inuse && IsTrigger(ent) && !strcmp(Str(ent->script_noteworthy), noteworthy))
            return ent;
    }
    return nullptr;
}

int TotalAmmo(const playerState_s *ps, unsigned int weapon)
{
    return weapon ? BG_GetAmmoInClip(ps, weapon) + BG_GetAmmoNotInClip(ps, weapon) : 0;
}

bool HasWeaponNamed(const playerState_s *ps, const char *name)
{
    const int index = BG_GetWeaponIndexForName(name);
    return index > 0 && BG_PlayerHasWeapon(ps, index);
}

bool HasUpgradedWeapon(const playerState_s *ps)
{
    for (int i = 0; i < 15; ++i)
        if (ps->heldWeapons[i].weapon && strstr(BG_WeaponName(ps->heldWeapons[i].weapon), "upgraded"))
            return true;
    return false;
}

bool WeaponsChanged(const playerState_s *ps)
{
    for (int i = 0; i < 15; ++i)
    {
        bool found = false;
        for (int j = 0; j < 15 && !found; ++j)
            found = ps->heldWeapons[i].weapon == s_startWeapons[j];
        if (ps->heldWeapons[i].weapon && !found)
            return true;
    }
    return false;
}

std::string WeaponList(const playerState_s *ps)
{
    std::string text;
    for (int i = 0; i < 15; ++i)
        if (ps->heldWeapons[i].weapon)
        {
            if (!text.empty())
                text += ",";
            text += BG_WeaponName(ps->heldWeapons[i].weapon);
        }
    return text;
}

void Log(const char *event, const char *detail = "")
{
    const char *name = s_index < (int)s_plan.size() ? s_steps[s_plan[s_index]].name : "-";
    Com_Printf(16, "bo1_plan: time %d step %d %s %s %s\n", level.time, s_index, name, event, detail);
}

// w1 c11: a back-buffer screenshot (the listen client's screenshotJpeg) at a harness event; -Client runs only.
void Shot(const char *name)
{
    if (!s_shotsDvar || !s_shotsDvar->current.enabled)
        return;
    char file[64];
    sprintf_s(file, "bo1_%s_%d", name, level.time);
    Cbuf_AddText(0, va("screenshotJpeg %s\n", file));
    Log("shot", file);
}

void TestSwitchScore(gentity_s *player, int need)
{
    if (!s_scoreDvar || s_scoreDvar->current.integer <= 0 || need <= 0)
        return;
    gclient_s *client = player->client;
    if (client->sess.cs.score.score >= need)
        return;
    int value = s_scoreDvar->current.integer > need ? s_scoreDvar->current.integer : need;
    Com_Printf(16, "bo1_plan: time %d TEST SWITCH score %d -> %d (bo1_testclient_score)\n", level.time,
        client->sess.cs.score.score, value);
    s_startScore += value - client->sess.cs.score.score;
    // L36: the payment frame's drop (s_largestDrop) is measured against s_lastScore after this top-up; sumpf2's
    // s_trap_elec_nw paid 1000 (1830 -> 830) and was topped up to 6000 in the same frame, so no drop was seen.
    s_lastScore += value - client->sess.cs.score.score;
    client->sess.cs.score.score = value;
    CalculateRanks();
}

void ParsePlan(const gentity_s *player)
{
    s_planParsed = true;
    s_isFive = !I_stricmp(Dvar_GetString("mapname"), "zombie_pentagon");
    s_plan.clear();
    s_planFlags.clear();
    s_planReplace.clear();
    std::string text = s_planDvar->current.string;
    // L37: Verruckt starts a solo player in a random one of two rooms (zombie_cod5_asylum.gsc spawn_point_override,
    // randomint(100) > 50). bo1_testclient_plan_alt is the plan for a start y above bo1_testclient_plan_alty.
    if (s_planAltDvar->current.string[0])
    {
        const bool alt = player->r.currentOrigin[1] > s_planAltYDvar->current.value;
        Com_Printf(16, "bo1_plan: start %.0f %.0f %.0f: %s plan\n", player->r.currentOrigin[0],
            player->r.currentOrigin[1], player->r.currentOrigin[2], alt ? "alt" : "main");
        if (alt)
            text = s_planAltDvar->current.string;
    }
    if (text == "all")
        text = s_fullPlan;
    size_t pos = 0;
    while (pos < text.size())
    {
        size_t end = text.find_first_of(" ,;", pos);
        if (end == std::string::npos)
            end = text.size();
        std::string word = text.substr(pos, end - pos);
        pos = end + 1;
        if (word.empty())
            continue;
        bool found = false;
        for (int i = 0; i < (int)ARRAY_COUNT(s_steps); ++i)
            if (word == s_steps[i].name)
            {
                s_plan.push_back(i);
                s_planFlags.push_back(0);
                s_planReplace.push_back(0);
                found = true;
                break;
            }
        if (!found)
            Com_Printf(16, "bo1_plan: unknown step '%s' ignored\n", word.c_str());
    }
    Com_Printf(16, "bo1_plan: %d steps: %s\n", (int)s_plan.size(), text.c_str());
}

void StartStep(gentity_s *player)
{
    playerState_s *ps = &player->client->ps;
    s_stepStart = level.time;
    s_ent = -1;
    s_phase = 0;
    s_phaseTime = level.time;
    s_startScore = player->client->sess.cs.score.score;
    const PlanStep &step = s_steps[s_plan[s_index]];
    s_startAmmo = TotalAmmo(ps, step.ammo && step.weapon ? BG_FindWeaponIndexForName(step.weapon) : ps->weapon);
    s_spent = 0;
    s_largestDrop = 0;
    s_lastScore = s_startScore;
    s_uses = 0;
    s_lastUse = 0;
    s_defconUseTime = 0;
    s_ridePanelUsed = false;
    s_rideCar = -1;
    s_spins = 0;
    s_stepShots = s_shots;
    s_fireDec = s_fireDt = 0;
    s_intervalSum = s_intervalCount = 0; // per step: before / after a perk compare cleanly
    s_reloadSum = s_reloadCount = 0;
    s_pathGoal = -1;
    Vec3Copy(player->r.currentOrigin, s_startOrigin);
    Vec3Copy(player->r.currentOrigin, s_progressOrigin);
    s_progressTime = level.time;
    for (int i = 0; i < 15; ++i)
        s_startWeapons[i] = ps->heldWeapons[i].weapon;
    char detail[256];
    sprintf_s(detail, "score %d weapon %s floor %d origin %.0f %.0f %.0f weapons %s", s_startScore,
        ps->weapon ? BG_WeaponName(ps->weapon) : "none", Floor(player->r.currentOrigin[2]),
        player->r.currentOrigin[0], player->r.currentOrigin[1], player->r.currentOrigin[2], WeaponList(ps).c_str());
    Log("start", detail);
}

void EndStep(gentity_s *player, const char *result, const char *why)
{
    playerState_s *ps = &player->client->ps;
    char detail[512];
    // r1: a step the arm policy inserted reports "recovery PASS/FAIL", apart from the route's own steps.
    const bool inserted = s_index < (int)s_planFlags.size() && (s_planFlags[s_index] & PLAN_INSERTED);
    if (inserted && strcmp(result, "PASS"))
        ++s_recoveryFails;
    s_holdWeapon = 0;
    sprintf_s(detail, "%s%s ent %d uses %d score %d -> %d health %d/%d weapon %s ammo %d weapons %s shots %d interval %.1f reload %.1f firerate %.3f (%s)",
        inserted ? "recovery " : "", result, s_ent, s_uses, s_startScore, player->client->sess.cs.score.score, player->health, player->maxHealth,
        ps->weapon ? BG_WeaponName(ps->weapon) : "none", TotalAmmo(ps, ps->weapon), WeaponList(ps).c_str(),
        s_shots - s_stepShots, s_intervalCount ? (float)s_intervalSum / s_intervalCount : -1.0f,
        s_reloadCount ? (float)s_reloadSum / s_reloadCount : -1.0f, s_fireDt ? (float)s_fireDec / s_fireDt : -1.0f, why);
    Log("end", detail);
    if (s_ent >= 0 && !strcmp(result, "PASS"))
        s_usedEnts.push_back(s_ent);
    if (!inserted)
        s_floorRequeues = s_waitResupplies = 0;
    const PlanStep &ended = s_steps[s_plan[s_index]];
    if (s_ent >= 0 && !strcmp(result, "PASS") && ended.kind == STEP_USE && ended.targetname
        && !strncmp(ended.targetname, "trap_", 5))
    {
        s_lastTrapUse = s_ent;
        s_lastTrapUseTime = level.time;
    }
    ++s_index;
    s_prevStepStart = s_stepStart;
    s_stepStart = 0;
}

// L35: off Five, a perk machine (zombie_vending) pressed 6 times from the flat-40 fallback without its cursor hint
// is approached again through the Bowie standing-point search (the retail sight check from a sampled point).
bool VendingStandSearch(const gentity_s *player, const gentity_s *ent)
{
    return !s_isFive && s_uses >= 6 && !strcmp(Str(ent->targetname), "zombie_vending")
        && !(player->client->ps.cursorHint
            && player->client->ps.cursorHintEntIndex == ent->s.number);
}

// The walk goal for a trigger: its xy on the floor under it (the trigger's own level), not at the player's
// height - the planner picks its goal node with a height check, so the player's z sent it to the level
// below the upper war room (Double Tap). A trigger whose center is inside solid (a perk machine's clip)
// uses its own bottom.
void GoalOnLevel(gentity_s *player, const gentity_s *ent, const float *center, float *ground)
{
    ground[0] = center[0];
    ground[1] = center[1];
    // p1 c17: the trigger's own bottom (origin + mins), not absmin: SV_LinkEntity widens a trigger_radius_use's
    // absolute box to its bounding sphere (Mule Kick: absmin 141 below the origin, one floor too low).
    ground[2] = ent->r.currentOrigin[2] + ent->r.mins[2] + 1.0f;
    float down[3] = { center[0], center[1], center[2] - 256.0f };
    trace_t trace;
    col_context_t context;
    G_TraceCapsule(&trace, center, vec3_origin, vec3_origin, down, player->s.number, player->clipmask, &context);
    if (!trace.startsolid && trace.fraction < 1.0f)
        ground[2] = center[2] + trace.fraction * (down[2] - center[2]) + 1.0f;

    // L40: a touch trigger over a drop - Call of the Dead's ship zipline (player_zipline, bounds z 592..674) hangs off the
    // upper deck's edge (floor z ~624); under its centre the floor is the lower deck (f1: goal z 450, the player's box
    // 450..522 never reached z 592). When the floor under the centre is more than a player's height (70) below the
    // trigger's bottom, stand on the nearest floor at the trigger's own level: sampled over its footprint widened by 14
    // (the touch box is +-15), floor z within bottom-70 .. top. Off Five only.
    const float bottom = ent->r.currentOrigin[2] + ent->r.mins[2];
    if (!s_isFive && ground[2] < bottom - 70.0f)
    {
        const float top = ent->r.currentOrigin[2] + ent->r.maxs[2];
        float best = 1e9f;
        for (float x = ent->r.absmin[0] - 14.0f; x <= ent->r.absmax[0] + 14.0f; x += 8.0f)
            for (float y = ent->r.absmin[1] - 14.0f; y <= ent->r.absmax[1] + 14.0f; y += 8.0f)
            {
                const float from[3] = { x, y, top };
                const float to[3] = { x, y, bottom - 70.0f };
                trace_t floor;
                col_context_t floorContext;
                G_TraceCapsule(&floor, from, vec3_origin, vec3_origin, to, player->s.number, player->clipmask, &floorContext);
                if (floor.startsolid || floor.fraction >= 1.0f || floor.normal.vec.v[2] < 0.7f)
                    continue;
                const float d = (x - center[0]) * (x - center[0]) + (y - center[1]) * (y - center[1]);
                if (d < best)
                {
                    best = d;
                    ground[0] = x;
                    ground[1] = y;
                    ground[2] = top + floor.fraction * (to[2] - top) + 1.0f;
                }
            }
    }

    // L28 waypoint: Kino's Pack-a-Punch is reached from the script_origin projroom_teleport_player1 (-46 -435 328,
    // a teleporter arrival point, k1: used at flat 65); walking at its centre from player0's point (-139 -436)
    // stalls on the machine's collision at x -78 (k2: 30 s, 0 uses).
    if (!s_isFive && !strcmp(Str(ent->targetname), "zombie_vending_upgrade"))
    {
        for (int i = level.maxclients; i < level.num_entities; ++i)
            if (g_entities[i].r.inuse && !strcmp(Str(g_entities[i].targetname), "projroom_teleport_player1"))
            {
                ground[0] = g_entities[i].r.currentOrigin[0];
                ground[1] = g_entities[i].r.currentOrigin[1];
                ground[2] = g_entities[i].r.currentOrigin[2];
                return;
            }
    }

    // L35 waypoint: Der Riese's mainframe (trigger_teleport_core, -56 294 112, bounds z 58..166) stands on the
    // start room's raised floor (z ~120); its bottom sent the goal to the floor below (f2: stalled at 74 220 -3,
    // 0 uses). Walk to the nearest teleporter arrival point on that floor (origin_teleport_player_0..3, z 116).
    if (!strcmp(Str(ent->targetname), "trigger_teleport_core"))
    {
        // f5: from the floor below, every walk to the arrival points took the zombies' climbs (74 285 -3, -188 124 -3,
        // 66 -717 43). The player leaves the spawn down the south stairs (f5 0..5000: 23 99 96 -> -20 -70 0): from
        // below, walk to their top first (between the platform nodes -20 81 120 and -76 81 120). f6: a goal at the top
        // still planned over the climbs (74 220 -3, 65 -717 43): walk to the stairs' foot (nodes -20/-76 -106 25)
        // first, then up the stair column (x -76..-20, nodes at y -42, 22, 81).
        const float *at = player->r.currentOrigin;
        if (at[2] < ent->r.currentOrigin[2] + ent->r.mins[2] + 32.0f)
        {
            const bool onStairs = at[0] > -100.0f && at[0] < 4.0f && at[1] > -140.0f && at[1] < 100.0f;
            ground[0] = -48.0f;
            ground[1] = onStairs ? 81.0f : -120.0f;
            ground[2] = onStairs ? 120.0f : 24.0f;
            return;
        }
        float bestArrival = 1.0e9f;
        for (int i = level.maxclients; i < level.num_entities; ++i)
            if (g_entities[i].r.inuse && !strncmp(Str(g_entities[i].targetname), "origin_teleport_player_", 23))
            {
                const float distance = Vec3DistanceSq(player->r.currentOrigin, g_entities[i].r.currentOrigin);
                if (distance < bestArrival)
                {
                    bestArrival = distance;
                    Vec3Copy(g_entities[i].r.currentOrigin, ground);
                }
            }
        return;
    }

    // Harness: the Bowie brush is in the wall, above the labs floor. Find a
    // standing position beside it, with a clear view inside the 72-unit use reach.
    // Movement and the retail look-at/use checks still decide whether we can buy.
    // The labs' start_chest2 (-486,4849) faces south of a wall (brush x -478..-386, y 4884..4892) and its
    // nearest path node is on the far side (f8-a/f8-d: 0 uses in 120 s): the box uses the same search.
    // (The log lines keep the "bowie" tag.)
    // L35: a perk machine off Five whose flat-40 fallback stood 6 presses without its hint (Der Riese's Double Tap,
    // f2: 152 uses at -404 -1085 199, the sight ray hit the alcove's side wall at x -384) takes the same search.
    if (strcmp(Str(ent->targetname), "bowie_upgrade") && strcmp(Str(ent->targetname), "treasure_chest_use")
        && !VendingStandSearch(player, ent))
        return;
    float best = 1.0e9f;
    // Diagnostics: why each sampled standing point was rejected (logged every 2 s).
    int noFloor = 0, hullSolid = 0, tooFar = 0, eyeBlocked = 0, accepted = 0;
    float eyeFrac = 0.0f;
    // Each sample's verdict, once per step.
    static int s_bowieDumpStart = -1;
    const bool dump = s_bowieDumpStart != s_stepStart;
    s_bowieDumpStart = s_stepStart;
    for (int radius = 32; radius <= 64; radius += 16)
        for (int i = 0; i < 16; ++i)
        {
            const float angle = i * (3.14159265f / 8.0f);
            float start[3] = { center[0] + radius * cosf(angle), center[1] + radius * sinf(angle), center[2] };
            float end[3] = { start[0], start[1], center[2] - 128.0f };
            auto sample = [&](const char *why, float frac) {
                if (dump)
                    Com_Printf(16, "bo1_plan: bowie sample r %d a %d at %.0f %.0f z %.0f frac %.3f %s\n", radius, i,
                        start[0], start[1], end[2], frac, why);
            };
            G_TraceCapsule(&trace, start, vec3_origin, vec3_origin, end, player->s.number, player->clipmask, &context);
            if (trace.startsolid || trace.fraction == 1.0f)
            {
                ++noFloor;
                sample(trace.startsolid ? "nofloor startsolid" : "nofloor miss", trace.fraction);
                continue;
            }
            end[2] = start[2] + trace.fraction * (end[2] - start[2]) + 1.0f;
            G_TraceCapsule(&trace, end, player->r.mins, player->r.maxs, end, player->s.number, player->clipmask, &context);
            if (trace.startsolid || trace.allsolid)
            {
                ++hullSolid;
                sample("hull solid", 0.0f);
                continue;
            }
            float eye[3] = { end[0], end[1], end[2] + player->client->ps.viewHeightCurrent };
            if (Vec3Distance(eye, center) > 71.0f)
            {
                ++tooFar;
                sample("far", Vec3Distance(eye, center));
                continue;
            }
            // The retail use list's own sight check (Player_GetUseList: SV_SightTracePoint, mask 17,
            // skipping the player and the trigger). A clipmask capsule stopped at the wall holding the brush.
            col_context_t sight(17);
            sight.passEntityNum0 = player->s.number;
            sight.passEntityNum1 = ent->s.number;
            int hitNum = -1;
            if (!SV_SightTracePoint(&hitNum, eye, center, &sight))
            {
                ++eyeBlocked;
                G_TraceCapsule(&trace, eye, vec3_origin, vec3_origin, center, player->s.number, player->clipmask, &context);
                if (trace.fraction > eyeFrac)
                    eyeFrac = trace.fraction;
                sample("eye blocked", trace.fraction);
                continue;
            }
            ++accepted;
            sample("accepted", Vec3Distance(eye, center));
            const float distance = Vec3DistanceSq(player->r.currentOrigin, end);
            if (distance < best)
            {
                best = distance;
                Vec3Copy(end, ground);
            }
        }
    static int s_bowieLogTime;
    if (level.time - s_bowieLogTime >= 2000)
    {
        s_bowieLogTime = level.time;
        // The player's own view: reach and the retail sight check, and its cursor hint.
        float view[3];
        G_GetPlayerViewOrigin(&player->client->ps, view);
        col_context_t sight(17);
        sight.passEntityNum0 = player->s.number;
        sight.passEntityNum1 = ent->s.number;
        int hitNum = -1;
        const bool visible = SV_SightTracePoint(&hitNum, view, center, &sight);
        Com_Printf(16, "bo1_plan: time %d bowie view %.0f %.0f %.0f reach %.1f visible %d hit %d hint %d/%d bounds %.0f %.0f .. %.0f %.0f\n",
            level.time, view[0], view[1], view[2], Vec3Distance(view, center), visible ? 1 : 0, hitNum,
            player->client->ps.cursorHint, player->client->ps.cursorHintEntIndex, ent->r.absmin[0], ent->r.absmin[1],
            ent->r.absmax[0], ent->r.absmax[1]);
        Com_Printf(16, "bo1_plan: time %d bowie approach center %.0f %.0f %.0f absmin z %.0f player %.0f %.0f %.0f reject nofloor %d hull %d far %d eye %d (best eye frac %.3f) accepted %d goal %.0f %.0f %.0f\n",
            level.time, center[0], center[1], center[2], ent->r.absmin[2], player->r.currentOrigin[0], player->r.currentOrigin[1],
            player->r.currentOrigin[2], noFloor, hullSolid, tooFar, eyeBlocked, eyeFrac, accepted, ground[0], ground[1], ground[2]);
    }
}

// L38: off Five, the player's plans skip the zombies' upward climbs (a negotiation link whose end is more than 48
// above its begin: Der Riese's -717 climb onto the mainframe floor, L35 f3-f9). Drops stay allowed. Path_FindPath
// already refuses a begin->end link while either node has wOverlapCount (an actor on the traverse), so the begin
// nodes of those climbs carry one extra overlap only for the duration of the harness's own planning calls.
struct ClimbBlock
{
    std::vector<pathnode_t *> nodes;
    explicit ClimbBlock(bool on)
    {
        if (!on || !gameWorldCurrent)
            return;
        // L41 path rule: Shangri-La's geyser shafts are one-way up for a player (a geyser launches, temple_geyser
        // triggers); the zombies' traverse down the shaft (s4b: -35 -1108 2 -> -34 -1049 -299) is not a player's path.
        std::vector<const gentity_s *> geysers;
        for (int e = 0; e < level.num_entities; ++e)
            if (g_entities[e].r.inuse && !strcmp(Str(g_entities[e].targetname), "temple_geyser"))
                geysers.push_back(&g_entities[e]);
        auto inGeyser = [&geysers](const float *p) {
            for (const gentity_s *g : geysers)
                if (p[0] >= g->r.absmin[0] - 32.0f && p[0] <= g->r.absmax[0] + 32.0f
                    && p[1] >= g->r.absmin[1] - 32.0f && p[1] <= g->r.absmax[1] + 32.0f)
                    return true;
            return false;
        };
        for (unsigned int i = 0; i < gameWorldCurrent->path.nodeCount; ++i)
        {
            pathnode_t *node = &gameWorldCurrent->path.nodes[i];
            if (node->constant.type != NODE_NEGOTIATION_BEGIN)
                continue;
            for (int k = 0; k < node->constant.totalLinkCount; ++k)
            {
                const pathnode_t *end = Path_ConvertIndexToNode(node->constant.Links[k].nodeNum);
                const float rise = end->constant.vOrigin[2] - node->constant.vOrigin[2];
                if (end->constant.type == NODE_NEGOTIATION_END && (rise > 48.0f || (rise < -48.0f && inGeyser(end->constant.vOrigin))))
                {
                    nodes.push_back(node);
                    ++node->dynamic.wOverlapCount;
                    break;
                }
            }
        }
    }
    ~ClimbBlock()
    {
        for (pathnode_t *node : nodes)
            --node->dynamic.wOverlapCount;
    }
};

// L41 path rule: whether the planner reaches pos from the player (the same planning call as the walk, climbs skipped).
bool PathReaches(const gentity_s *player, const float *pos)
{
    if (!player->sentient)
        return true;
    static path_t path;
    path.wPathLen = 0;
    const ClimbBlock climbs(true);
    float origin[3] = { player->r.currentOrigin[0], player->r.currentOrigin[1], player->r.currentOrigin[2] };
    float goal[3] = { pos[0], pos[1], pos[2] };
    return Path_FindPath(&path, player->sentient->eTeam, origin, goal, 1) != 0;
}

// Walk toward goal along the pathnode graph (Path_FindPath, the actors' own planner), re-planned every
// two seconds; sidestep and jump when no progress is made.
// L20 v5-4: a teleport pad (targetname portal_trigs) whose bounds, grown by margin, hold the point.
bool NearPortalPad(const float *point, float margin)
{
    for (int i = level.maxclients; i < level.num_entities; ++i)
    {
        const gentity_s *ent = &g_entities[i];
        if (!ent->r.inuse || strcmp(Str(ent->targetname), "portal_trigs"))
            continue;
        if (point[0] > ent->r.absmin[0] - margin && point[0] < ent->r.absmax[0] + margin
            && point[1] > ent->r.absmin[1] - margin && point[1] < ent->r.absmax[1] + margin
            && point[2] > ent->r.absmin[2] - 72.0f && point[2] < ent->r.absmax[2] + 72.0f)
            return true;
    }
    return false;
}

void Walk(gentity_s *player, const float *goal, int goalId, usercmd_s *cmd, float viewYaw)
{
    const float *origin = player->r.currentOrigin;
    float goalView[3] = { goal[0], goal[1], goal[2] + 48.0f };
    const bool entGoal = goalId >= 0 && goalId < level.num_entities;
    const float sx = goal[0] - origin[0], sy = goal[1] - origin[1];
    const float stallFlat = sqrtf(sx * sx + sy * sy);
    // Progress is the flat distance to the goal (the unstick sidesteps move the player without bringing him nearer).
    if (entGoal && s_stallGoal == goalId && stallFlat > s_stallDist - 16.0f)
    {
        if (level.time - s_stallTime > 5000 && s_avoidGoal != goalId && stallFlat < 256.0f)
        {
            float eye[3];
            G_GetPlayerViewOrigin(&player->client->ps, eye);
            col_context_t sight(17);
            sight.passEntityNum0 = player->s.number;
            sight.passEntityNum1 = goalId;
            int hitNum = -1;
            if (!SV_SightTracePoint(&hitNum, eye, goalView, &sight))
            {
                s_avoidGoal = goalId;
                Vec3Copy(origin, s_avoidOrigin);
                s_pathTime = level.time - 2000; // replan now
                char detail[128];
                sprintf_s(detail, "stalled at %.0f %.0f %.0f without sight of goal %d (hit %d): avoiding this spot",
                    origin[0], origin[1], origin[2], goalId, hitNum);
                Log("walk", detail);
            }
        }
    }
    else
    {
        s_stallGoal = goalId;
        s_stallTime = level.time;
        s_stallDist = stallFlat;
    }
    const bool avoiding = entGoal && s_avoidGoal == goalId;
    if (s_pathGoal != goalId || level.time - s_pathTime >= 2000)
    {
        s_pathGoal = goalId;
        s_pathTime = level.time;
        s_path.wPathLen = 0;
        const ClimbBlock climbs(!s_isFive); // L38: every planning call below skips the zombies' upward climbs
        // L35: to Der Riese's mainframe, a path without negotiation links first: those are the zombies' climbs (f3:
        // the walk from pad S stalled 15 s at 65 -717 43, under the climb 65.5 -717.5 70 -> 65 -669 161). Only for
        // this goal: for every walk (f4) the tp_south door walk lost its way (19/8).
        int found = 0;
        if (!avoiding && goalId >= 0 && goalId < level.num_entities && !strcmp(Str(g_entities[goalId].targetname), "trigger_teleport_core"))
            found = Path_FindPath(&s_path, player->sentient->eTeam, origin, goal, 0);
        if (!found && !avoiding)
        {
            s_path.wPathLen = 0;
            found = Path_FindPath(&s_path, player->sentient->eTeam, origin, goal, 1);
        }
        if (!found)
        {
            // The node nearest the goal can belong to a separate cluster (Five's war room has isolated nodes
            // around the map table): try the nodes around the goal on its own level, nearest first, and walk
            // the first one the planner reaches, then straight on.
            pathsort_t nodes[64];
            int count = 0;
            // Start node: the nearest node on the player's own level (in an elevator car standing below the
            // station floor, Path_NearestNode picked the labs node under the shaft).
            pathnode_t *from = nullptr;
            float fromDist = 256.0f * 256.0f;
            for (unsigned int i = 0; i < gameWorldCurrent->path.nodeCount; ++i)
            {
                pathnode_t *node = &gameWorldCurrent->path.nodes[i];
                const float *o = node->constant.vOrigin;
                const float dx = o[0] - origin[0], dy = o[1] - origin[1], dz = o[2] - origin[2];
                if (fabsf(dz) < 64.0f && dx * dx + dy * dy < fromDist)
                {
                    fromDist = dx * dx + dy * dy;
                    from = node;
                }
            }
            if (!from)
                from = Path_NearestNode(origin, nodes, -2, 192.0f, &count, 64, NEAREST_NODE_DO_HEIGHT_CHECK);
            if (from)
            {
                struct Candidate { float dist; pathnode_t *node; };
                std::vector<Candidate> around;
                const unsigned int nodeCount = gameWorldCurrent->path.nodeCount;
                for (unsigned int i = 0; i < nodeCount; ++i)
                {
                    pathnode_t *node = &gameWorldCurrent->path.nodes[i];
                    const float *o = node->constant.vOrigin;
                    const float dx = o[0] - goal[0], dy = o[1] - goal[1], dz = o[2] - goal[2];
                    const float flat = dx * dx + dy * dy;
                    if (!(flat < 256.0f * 256.0f && fabsf(dz) < 72.0f))
                        continue;
                    if (avoiding)
                    {
                        const float ax = o[0] - s_avoidOrigin[0], ay = o[1] - s_avoidOrigin[1];
                        if (ax * ax + ay * ay < 64.0f * 64.0f)
                            continue;
                        const float nodeView[3] = { o[0], o[1], o[2] + 60.0f };
                        col_context_t sight(17);
                        sight.passEntityNum0 = player->s.number;
                        sight.passEntityNum1 = goalId;
                        int hitNum = -1;
                        if (!SV_SightTracePoint(&hitNum, nodeView, goalView, &sight))
                            continue;
                    }
                    around.push_back({ flat, node });
                }
                std::sort(around.begin(), around.end(), [](const Candidate &a, const Candidate &b) { return a.dist < b.dist; });
                for (size_t i = 0; i < around.size() && i < 16 && !found; ++i)
                {
                    s_path.wPathLen = 0;
                    if (Path_FindPathFromTo(&s_path, player->sentient->eTeam, from, origin, around[i].node, goal, 1, 1))
                    {
                        found = 3;
                        if (found != s_pathFound)
                        {
                            char detail[128];
                            sprintf_s(detail, "via node %.0f %.0f %.0f (candidate %d of %d)", around[i].node->constant.vOrigin[0],
                                around[i].node->constant.vOrigin[1], around[i].node->constant.vOrigin[2], (int)i, (int)around.size());
                            Log("walk", detail);
                        }
                    }
                }
            }
        }
        if (!found)
        {
            // The goal's nodes are cut off (a debris pile or door disconnects its paths until bought): walk
            // to the reachable node closest to it, then straight on.
            pathsort_t nodes[64];
            int count = 0;
            pathnode_t *to = Path_NearestNode(goal, nodes, -2, 192.0f, &count, 64, NEAREST_NODE_DO_HEIGHT_CHECK);
            pathnode_t *from = to ? Path_NearestNode(origin, nodes, -2, 192.0f, &count, 64, NEAREST_NODE_DO_HEIGHT_CHECK) : nullptr;
            s_path.wPathLen = 0;
            if (from && Path_FindPathGetCloseAsPossible(&s_path, player->sentient->eTeam, from, origin, to, goal, 1))
                found = 2;
            if (found != s_pathFound)
            {
                char detail[192];
                sprintf_s(detail, "no direct path: goal node %.0f %.0f %.0f, start node %.0f %.0f %.0f",
                    to ? to->constant.vOrigin[0] : 0.0f, to ? to->constant.vOrigin[1] : 0.0f, to ? to->constant.vOrigin[2] : 0.0f,
                    from ? from->constant.vOrigin[0] : 0.0f, from ? from->constant.vOrigin[1] : 0.0f,
                    from ? from->constant.vOrigin[2] : 0.0f);
                Log("walk", detail);
            }
        }
        if (!found)
            s_path.wPathLen = 0;
        if (found != s_pathFound)
        {
            char detail[96];
            sprintf_s(detail, "goal %d %s, %d points", goalId, found == 1 ? "path" : found == 2 ? "closest" : found == 3 ? "via node" : "no path",
                (int)s_path.wPathLen);
            Log("walk", detail);
            s_pathFound = found;
        }
        s_pathPoint = s_path.wPathLen - 1;
    }
    float point[3];
    while (s_pathPoint >= 0)
    {
        const float *p = s_path.pts[s_pathPoint].vOrigPoint;
        const float dx = p[0] - origin[0], dy = p[1] - origin[1];
        if (dx * dx + dy * dy > 24.0f * 24.0f)
            break;
        --s_pathPoint;
    }
    // L20 v5-4: in the pack room, not over its pad unless the pad is the goal. The planner's start node is west of
    // the arrival point, on the room's pad; while defcon_active is set (a Bonfire Sale keeps it) cooldown_portal_timer
    // (maps\zombie_pentagon_teleporter) frees the pad at once and it sends the player out. v5-4: pap arrived at
    // -2479 2078 four times (259200 .. 274650), walked back to -2508 and was teleported out within 1 s each time,
    // until pap timed out (uses 0). The room is open floor from the arrival point to the machine: skip those points.
    const bool padGoal = goalId >= 0 && goalId < level.num_entities && !strcmp(Str(g_entities[goalId].targetname), "portal_trigs");
    if (!padGoal && InPackRoom(origin))
        while (s_pathPoint >= 0 && NearPortalPad(s_path.pts[s_pathPoint].vOrigPoint, 24.0f))
            --s_pathPoint;
    // L28: Kino's projection room has no path nodes of its own (the nearest, 184 -544 248 / -180 -550 248, are on the
    // floor below; k3 stalled 30 s at 160 -496 walking at them): straight to the Pack-a-Punch waypoint (GoalOnLevel).
    if (!s_isFive && goalId >= 0 && goalId < level.num_entities
        && !strcmp(Str(g_entities[goalId].targetname), "zombie_vending_upgrade") && fabsf(goal[2] - origin[2]) < 64.0f
        && (goal[0] - origin[0]) * (goal[0] - origin[0]) + (goal[1] - origin[1]) * (goal[1] - origin[1]) < 320.0f * 320.0f)
    {
        // L39: only when the machine is in sight. Ascension's PaP is behind the control room's west wall (brush
        // 14258): c7 stood 92 units from it at 309 380 -304 until the timeout, this rule dropping every frame the
        // stall rule's path via 383 450 -304 (the room's door side).
        float eye[3];
        G_GetPlayerViewOrigin(&player->client->ps, eye);
        col_context_t sight(17);
        sight.passEntityNum0 = player->s.number;
        sight.passEntityNum1 = goalId;
        int hitNum = -1;
        if (SV_SightTracePoint(&hitNum, eye, goalView, &sight))
            s_pathPoint = -1;
    }
    if (s_pathPoint >= 0)
        Vec3Copy(s_path.pts[s_pathPoint].vOrigPoint, point);
    else
        Vec3Copy(goal, point);
    float dir[2] = { point[0] - origin[0], point[1] - origin[1] };
    const float length = sqrtf(dir[0] * dir[0] + dir[1] * dir[1]);
    if (length < 1.0f)
        return;
    dir[0] /= length;
    dir[1] /= length;
    // No progress for 2.5 s: sidestep (alternating side) and jump for 0.8 s, then re-plan.
    const float moved = sqrtf((origin[0] - s_progressOrigin[0]) * (origin[0] - s_progressOrigin[0])
        + (origin[1] - s_progressOrigin[1]) * (origin[1] - s_progressOrigin[1]));
    if (moved > 24.0f)
    {
        Vec3Copy(origin, s_progressOrigin);
        s_progressTime = level.time;
    }
    else if (level.time - s_progressTime > 2500 && level.time > s_unstickUntil)
    {
        s_unstickUntil = level.time + 800;
        s_unstickDir = -s_unstickDir;
        s_progressTime = level.time;
        s_pathGoal = -1;
    }
    const float yaw = viewYaw * (3.14159265f / 180.0f);
    float forward = dir[0] * cosf(yaw) + dir[1] * sinf(yaw);
    float right = dir[0] * sinf(yaw) - dir[1] * cosf(yaw);
    if (level.time < s_unstickUntil)
    {
        const float f = forward, r = right;
        forward = 0.5f * f - 0.8f * s_unstickDir * r;
        right = 0.5f * r + 0.8f * s_unstickDir * f;
        cmd->button_bits.setBit(10); // jump
    }
    cmd->forwardmove = (char)(int)(127.0f * (forward > 1 ? 1 : forward < -1 ? -1 : forward));
    cmd->rightmove = (char)(int)(127.0f * (right > 1 ? 1 : right < -1 ? -1 : right));
}

void Face(gentity_s *player, usercmd_s *cmd, const float *point, bool pitch)
{
    playerState_s *ps = &player->client->ps;
    float eye[3], dir[3], angles[3];
    G_GetPlayerViewOrigin(ps, eye);
    Vec3Sub(point, eye, dir);
    vectoangles(dir, angles);
    for (int i = 0; i < 2; ++i)
        if (i == 1 || pitch)
            cmd->angles[i] = (unsigned short)(int)((angles[i] - ps->delta_angles[i]) * 182.04445f);
}

float ViewYaw(gentity_s *player, usercmd_s *cmd)
{
    return (float)(unsigned short)cmd->angles[1] / 182.04445f + player->client->ps.delta_angles[1];
}

// Walk to a trigger and press use once it is the engine's use candidate (ps.cursorHintEntIndex) or the
// player is touching it; true while in reach.
bool HasLoadedGun(const playerState_s *ps);

bool GoUse(gentity_s *player, usercmd_s *cmd, gentity_s *ent, gentity_s *target, float targetDistance, bool press)
{
    playerState_s *ps = &player->client->ps;
    float center[3], eye[3];
    Center(ent, center);
    G_GetPlayerViewOrigin(ps, eye);
    const float dx = center[0] - eye[0], dy = center[1] - eye[1];
    const float flat = sqrtf(dx * dx + dy * dy);
    // Player_UpdateCursorHints clears cursorHint every frame but can retain the previous
    // entity index. That stale index does not mean a distant trigger is still usable.
    const bool candidate = ps->cursorHint && ps->cursorHintEntIndex == ent->s.number;
    bool reach = candidate;
    if (!reach && !strcmp(Str(ent->classname), "trigger_use_touch"))
    {
        // L20: the use list takes a trigger_use_touch only in contact with the player's touch box (origin -15 -15 0 ..
        // +15 +15 +70, Player_UpdateUse / player_use_mp.cpp, SP the same). The 40-unit fallback stopped b111 at -1495
        // 2053 -304, 33.9 units from elevator2_down's centre and outside its box: 125 presses, a 120 s recovery FAIL
        // (q1a15 and x632m1 stood there too until a zombie pushed them in). Walk on to the trigger until in contact.
        const float touchMins[3] = { ps->origin[0] - 15.0f, ps->origin[1] - 15.0f, ps->origin[2] };
        const float touchMaxs[3] = { ps->origin[0] + 15.0f, ps->origin[1] + 15.0f, ps->origin[2] + 70.0f };
        reach = SV_EntityContact(touchMins, touchMaxs, ent);
    }
    else if (!reach)
    {
        reach = flat < 40.0f && fabsf(center[2] - eye[2]) < 80.0f;
        // L36: not from behind solid geometry. The use list takes a trigger only with sight of its center (Player_GetUseList:
        // SV_SightTracePoint, mask 17, skipping the player and the trigger); sumpf6 stood 38.6 from vending 336's center
        // behind the machine (8542 3234) and pressed use 170 times. Walk on: Walk's stall rule then picks a spot with sight.
        if (reach)
        {
            col_context_t sight(17);
            sight.passEntityNum0 = player->s.number;
            sight.passEntityNum1 = ent->s.number;
            int hitNum = -1;
            if (!SV_SightTracePoint(&hitNum, eye, center, &sight))
                reach = false;
        }
        // L35: a perk machine off Five whose use hint never shows: search a better place to stand
        if (reach && VendingStandSearch(player, ent))
            reach = false;
    }
    if (!reach)
    {
        float viewYaw = ViewYaw(player, cmd);
        // A wall's collision can stop us outside the 40-unit fallback. Face the buy
        // while within cursor-hint range even if a distant zombie is still visible.
        if (!target || (flat < 128.0f && targetDistance > 256.0f))
        {
            // Retail Bowie requires the view ray to hit the trigger. Set pitch too;
            // a yaw-only approach can preserve the previous combat target's pitch.
            Face(player, cmd, center, true);
            viewYaw = ViewYaw(player, cmd);
        }
        float ground[3];
        GoalOnLevel(player, ent, center, ground);
        Walk(player, ground, ent->s.number, cmd, viewYaw);
        if (level.time % 5000 == 0)
            Com_Printf(16, "bo1_plan: time %d approach ent %d flat %.1f hint %d/%d origin %.0f %.0f %.0f goal %.0f %.0f %.0f\n",
                level.time, ent->s.number, flat, ps->cursorHint, ps->cursorHintEntIndex,
                player->r.currentOrigin[0], player->r.currentOrigin[1], player->r.currentOrigin[2], ground[0], ground[1], ground[2]);
        return false;
    }
    // Buy/take a box weapon while distant enemies approach, retaining close-range
    // defence; otherwise a steady stream of zombies starves the brief pickup window.
    const bool usingBox = s_index < (int)s_plan.size() && s_steps[s_plan[s_index]].kind == STEP_BOX;
    const bool usingTrap = s_index < (int)s_plan.size() && !strcmp(s_steps[s_plan[s_index]].name, "trap_qr");
    // L20: not with every bullet gun dry. L141 760150: the wait's resupply walked to the m14 wall with the ak74u dry
    // (0 + 0), stood at the chalk (reach 30) knifing a 950-health zombie until downed; the buy is the fight.
    if (target && targetDistance < (usingBox || usingTrap ? 100.0f : 256.0f) && HasLoadedGun(ps))
        return true; // fight first
    Face(player, cmd, center, true);
    cmd->button_bits.resetBit(0);
    cmd->button_bits.resetBit(11);
    if (!press)
        return true;
    // Hold use for 3 frames, release for 7: the engine's activate is an edge, door_buy reads
    // UseButtonPressed while the notify is handled.
    if (level.time - s_lastUse >= 500)
    {
        s_lastUse = level.time;
        ++s_uses;
        char detail[192];
        sprintf_s(detail, "ent %d candidate %d (hint ent %d %s) flat %.1f score %d origin %.0f %.0f %.0f", ent->s.number,
            candidate, ps->cursorHintEntIndex, ps->cursorHintEntIndex < 1023 ? Str(g_entities[ps->cursorHintEntIndex].targetname) : "", flat, player->client->sess.cs.score.score, player->r.currentOrigin[0],
            player->r.currentOrigin[1], player->r.currentOrigin[2]);
        if (s_uses <= 3 || s_uses % 10 == 0)
            Log("use", detail);
    }
    if (level.time - s_lastUse < 150)
    {
        cmd->button_bits.setBit(3);
        // L40: a coast zipline starts on a jump, not use: is_ok_to_zipline wants the jump button or a jump within 800 ms
        // (jump_button_monitor's self.jumptime) while touching player_zipline (maps\_zombiemode_player_zipline.gsc:287-331,
        // zombie_coast_patch). z1: 89 use presses on the upper deck in contact, no ride.
        if (!strcmp(Str(ent->targetname), "player_zipline"))
            cmd->button_bits.setBit(10); // jump
    }
    return true;
}

// k1 harness only: after a route, replenish on the current floor. The old labs goal
// kept walking toward the downstairs AK after a portal carried the player upstairs,
// stalling round 16. Reuse the plan's path/unstick/use policy; purchases remain scripted.
bool Resupply(gentity_s *player, usercmd_s *cmd)
{
    const int floor = Floor(player->r.currentOrigin[2]);
    const char *name = floor == 1 ? "wall_m14" : floor == 2 ? "wall_mp5k" : "wall_ak74u";
    for (const PlanStep &step : s_steps)
    {
        if (strcmp(step.name, name))
            continue;
        const unsigned int weapon = BG_FindWeaponIndexForName(step.weapon);
        if (!weapon)
            return false;
        const playerState_s *ps = &player->client->ps;
        if (BG_PlayerHasWeapon(ps, weapon)
            && BG_GetAmmoInClip(ps, weapon) + BG_GetAmmoNotInClip(ps, weapon) > 30)
            return false;
        // Keep enough ammunition to get back to the wall. Require the full gun price
        // (also covers the cheaper ammo purchase); never top up score after the route.
        if (player->client->sess.cs.score.score < step.cost)
            return false;
        gentity_s *ent = FindTarget(step, player, true);
        float center[3];
        if (!ent)
            return false;
        Center(ent, center);
        if (Floor(center[2]) != floor)
            return false;
        // Buying must not be starved by an endless queue of melee targets at the wall.
        cmd->button_bits.resetBit(2);
        GoUse(player, cmd, ent, nullptr, -1.0f, true);
        if (level.time % 5000 == 0)
            Com_Printf(16, "bo1_resupply: time %d floor %d weapon %s ent %d ammo %d\n", level.time,
                floor, step.weapon, ent->s.number, BG_GetAmmoInClip(ps, weapon) + BG_GetAmmoNotInClip(ps, weapon));
        return true;
    }
    return false;
}

// r1 arm policy (test-client play only; every purchase is the scripts' own wall buy through the ordinary use press).
// The route must not depend on the box's ammo or on the thief: keep a second loaded gun (the box takes the gun
// without a wall buy, see BoxGiveUp), buy ammo for a low wall gun when its wall is on this floor, replace a dry
// gun that cannot be refilled here by this floor's wall gun, and after a steal buy a second gun again.
bool IsPrimaryGun(unsigned int weapon)
{
    // Any primary slot (r2a: the box's knife_ballistic_zm is no bullet gun but still fills a slot).
    return weapon && BG_GetWeaponDef(weapon)->inventoryType == WEAPINVENTORY_PRIMARY;
}

int PrimaryGuns(const playerState_s *ps, unsigned int *guns)
{
    int count = 0;
    for (int i = 0; i < 15; ++i)
        if (IsPrimaryGun(ps->heldWeapons[i].weapon) && count < 15)
            guns[count++] = ps->heldWeapons[i].weapon;
    return count;
}

// Fraction of a full load (clip + the player's stock maximum).
float AmmoFraction(const playerState_s *ps, unsigned int weapon)
{
    const int full = BG_GetClipSize(weapon) + BG_GetAmmoPlayerMax(ps, weapon, 0);
    return full > 0 ? (float)TotalAmmo(ps, weapon) / (float)full : 1.0f;
}

// The plan's wall-buy step (or its ammo step) for this exact weapon; null for box guns and the start pistol.
int WallStepIndex(unsigned int weapon, bool ammo)
{
    const char *name = BG_WeaponName(weapon);
    for (int i = 0; i < (int)ARRAY_COUNT(s_steps); ++i)
    {
        const PlanStep &step = s_steps[i];
        if (step.kind == STEP_USE && step.key && !strcmp(step.key, "zombie_weapon_upgrade") && step.ammo == ammo
            && step.value && !strcmp(step.value, name) && strcmp(step.name, "wall_frag"))
            return i;
    }
    return -1;
}

// The step's enabled trigger on the player's floor, else null.
gentity_s *TriggerOnFloor(int stepIndex, const gentity_s *player)
{
    gentity_s *ent = stepIndex >= 0 ? FindTarget(s_steps[stepIndex], player, true) : nullptr;
    if (!ent)
        return nullptr;
    float center[3];
    Center(ent, center);
    return Floor(center[2]) == Floor(player->r.currentOrigin[2]) ? ent : nullptr;
}

// This floor's wall gun the player does not hold (the walls Resupply already reaches, nearest first).
int FloorWallGun(const gentity_s *player)
{
    static const char *const walls[4][2] = { { nullptr, nullptr }, { "wall_m14", nullptr },
        { "wall_mp5k", "wall_pm63" }, { "wall_ak74u", "wall_m16" } };
    const int floor = Floor(player->r.currentOrigin[2]);
    if (floor < 1 || floor > 3)
        return -1;
    int best = -1;
    float bestDistance = 0.0f;
    for (const char *name : walls[floor])
    {
        for (int i = 0; name && i < (int)ARRAY_COUNT(s_steps); ++i)
        {
            if (strcmp(s_steps[i].name, name))
                continue;
            char upgraded[64];
            sprintf_s(upgraded, "%.*s_upgraded_zm", (int)strlen(s_steps[i].weapon) - 3, s_steps[i].weapon);
            if (HasWeaponNamed(&player->client->ps, s_steps[i].weapon) || HasWeaponNamed(&player->client->ps, upgraded))
                continue;
            gentity_s *ent = TriggerOnFloor(i, player);
            if (!ent)
                continue;
            float center[3];
            Center(ent, center);
            const float distance = Vec3Distance(center, player->r.currentOrigin);
            if (best < 0 || distance < bestDistance)
            {
                best = i;
                bestDistance = distance;
            }
        }
    }
    return best;
}

// Put a recovery step before the current one (only between steps: s_stepStart is 0).
void InsertStep(int stepIndex, unsigned int replace, const char *why)
{
    s_plan.insert(s_plan.begin() + s_index, stepIndex);
    s_planFlags.insert(s_planFlags.begin() + s_index, (unsigned char)(PLAN_INSERTED | PLAN_ARMED));
    s_planReplace.insert(s_planReplace.begin() + s_index, replace);
    Com_Printf(16, "bo1_plan: time %d insert step %d %s before %s: %s\n", level.time, s_index, s_steps[stepIndex].name,
        s_index + 1 < (int)s_plan.size() ? s_steps[s_plan[s_index + 1]].name : "-", why);
}

// Between steps: decide whether the player must buy before the step. Inserts at most one purchase.
void ArmCheck(gentity_s *player)
{
    const playerState_s *ps = &player->client->ps;
    unsigned int guns[15];
    const int count = PrimaryGuns(ps, guns);
    if (count >= 2)
        s_hadTwoGuns = true;
    // r1 c3: no primary gun at all (weapon none) is not bought for here: _zombiemode_weapons::can_buy_weapon refuses
    // every wall buy with weapon none (r3d: 53 s at the ak74u wall, downed). STEP_PAP holds before a thief round.
    if (s_recoveryFails >= 2 || player->client->lastStand || !ps->weapon)
        return;
    const PlanStep &next = s_steps[s_plan[s_index]];
    if (next.kind == STEP_USE && next.key && !strcmp(next.key, "zombie_weapon_upgrade"))
        return; // the route buys here itself
    // r1 c5 (r5d): not before pap. portal_pack has just put the player on a pad (the ride starts after the step),
    // the pack room has no wall, and Pack-a-Punch fills the gun it upgrades. r5d 283850: ammo_mp5k inserted before
    // pap rode in, walked back out to the floor 2 wall, and pap timed out outside the pack room.
    if (next.kind == STEP_PAP || next.kind == STEP_PAP_VIEW)
        return;
    const bool boxNext = next.kind == STEP_BOX || next.kind == STEP_PAP; // the box / PaP changes a gun anyway
    // q1 c9: a long wait (wait110 holds one spot for 110 s at round 8+) empties what the route steps leave. a11m: m14
    // 96 + m16 90 at its start, both dry well before its end; knife fights lost Juggernog and the player bled out.
    // Before it: refill a gun whose wall is on this floor from under 75%, replace a non-refillable one under 50%.
    const bool longWait = next.kind == STEP_WAIT && next.count >= 60000;
    const float refillBelow = longWait ? 0.75f : 0.34f;
    const float replaceBelow = longWait ? 0.5f : 0.1f;
    char why[192];
    // After a steal (or a gun lost otherwise): a second gun from this floor's wall.
    if (count < 2 && s_hadTwoGuns)
    {
        const int wall = FloorWallGun(player);
        if (wall >= 0)
        {
            sprintf_s(why, "%d primary gun(s) held (%s); buy a second gun", count, WeaponList(ps).c_str());
            InsertStep(wall, 0, why);
            return;
        }
    }
    // A low gun whose wall is on this floor: its ammo.
    for (int i = 0; i < count; ++i)
    {
        const float fraction = AmmoFraction(ps, guns[i]);
        const int ammoStep = WallStepIndex(guns[i], true);
        if (fraction < refillBelow && ammoStep >= 0 && TriggerOnFloor(ammoStep, player))
        {
            sprintf_s(why, "%s ammo %d (%.0f%% of full)", BG_WeaponName(guns[i]), TotalAmmo(ps, guns[i]), fraction * 100.0f);
            InsertStep(ammoStep, 0, why);
            return;
        }
    }
    // A nearly dry gun that cannot be refilled here, beside another: this floor's wall gun replaces it.
    // r1 c6 (r5c): a Pack-a-Punched gun is kept down to the wait's own resupply line (15%, the requeue in the wait
    // step). 446850: python_upgraded_zm at 38% (41 rounds, one-shot kills at round 6) was swapped for the floor 1
    // m14 (315 a hit, 3 hits on round 6's 650, 8-round clip); at 521250-522050 three zombies within 91 units hit
    // five times (5 x 60 = 300 > Juggernog's 250) while it reloaded, and the player went down in wait110.
    for (int i = 0; count >= 2 && !boxNext && i < count; ++i)
    {
        const float fraction = AmmoFraction(ps, guns[i]);
        const int ammoStep = WallStepIndex(guns[i], true);
        const float replaceLine =
            strstr(BG_WeaponName(guns[i]), "_upgraded") && replaceBelow > 0.15f ? 0.15f : replaceBelow;
        if (fraction >= replaceLine || (ammoStep >= 0 && TriggerOnFloor(ammoStep, player)))
            continue;
        const int wall = FloorWallGun(player);
        if (wall < 0)
            return;
        sprintf_s(why, "%s ammo %d (%.0f%% of full), no refill on this floor; replace it", BG_WeaponName(guns[i]),
            TotalAmmo(ps, guns[i]), fraction * 100.0f);
        InsertStep(wall, guns[i], why);
        return;
    }
}

// r1: a Bonfire Sale (a thief kill's drop, or a natural one) pulls every defcon switch, and its expiry resets the
// level to 1 (r2d: two of the route's four defcon steps passed during the sale, the level went 5 -> 1 at its end and
// portal_pack teleported without the pack room). Before portal_pack: pull the switches again up to defcon 5.
void PackCheck(gentity_s *player)
{
    const PlanStep &next = s_steps[s_plan[s_index]];
    if (next.kind != STEP_TOUCH || !next.notify || strcmp(next.notify, "open_pack_hideaway") || s_recoveryFails >= 2)
        return;
    const int defcon = LevelInt(F_DEFCON, -1);
    if (defcon < 1 || defcon >= 5 || ZombieVar(F_BONFIRE) == 1)
        return;
    char why[96];
    sprintf_s(why, "defcon level %d before the pack room portal; pull to 5", defcon);
    for (int i = 0; i < (int)ARRAY_COUNT(s_steps); ++i)
        if (!strcmp(s_steps[i].name, "defcon"))
            for (int n = defcon; n < 5; ++n)
                InsertStep(i, 0, why);
    // r1 c3: the defcon step walks its floor only (q111m: inserted on floor 3 after a thief round, it stood at the
    // elevator for 50 s). Ride to the switches' floor first; inserted last, so it runs first.
    const int playerFloor = Floor(player->r.currentOrigin[2]);
    for (int i = level.maxclients; i < level.num_entities; ++i)
    {
        if (!g_entities[i].r.inuse || strcmp(Str(g_entities[i].targetname), "punch_switch"))
            continue;
        float center[3];
        Center(&g_entities[i], center);
        const int floor = Floor(center[2]);
        if (floor == playerFloor)
            break;
        char name[16];
        sprintf_s(name, "floor%d", floor);
        sprintf_s(why, "the defcon switches are on floor %d, the player on floor %d", floor, playerFloor);
        for (int n = 0; n < (int)ARRAY_COUNT(s_steps); ++n)
            if (!strcmp(s_steps[n].name, name))
                InsertStep(n, 0, why);
        break;
    }
}

// The box replaces the held gun when two are held: hold the one worth least (no wall buy: the start pistol or an
// earlier box gun; else the emptier one), so the wall gun stays as the loaded second gun.
unsigned int BoxGiveUp(const playerState_s *ps)
{
    unsigned int guns[15];
    const int count = PrimaryGuns(ps, guns);
    if (count < 2)
        return 0;
    unsigned int best = 0;
    float bestScore = 0.0f;
    for (int i = 0; i < count; ++i)
    {
        const float score = (WallStepIndex(guns[i], false) >= 0 ? 10.0f : 0.0f) + AmmoFraction(ps, guns[i]);
        if (!best || score < bestScore)
        {
            best = guns[i];
            bestScore = score;
        }
    }
    return best;
}

// Raise `weapon` through the ordinary usercmd (as STEP_EQUIP); true once it is up and ready.
bool HoldWeapon(gentity_s *player, client_t *client, usercmd_s *cmd, unsigned int weapon)
{
    const playerState_s *ps = &player->client->ps;
    s_holdWeapon = weapon;
    if (ps->weapon == weapon && ps->weaponstate == WEAPON_READY)
        return true;
    botInfos[client - svs.clients].weapon = weapon;
    cmd->weapon = weapon;
    return false;
}

void UpdateMeasurement(gentity_s *player)
{
    playerState_s *ps = &player->client->ps;
    const unsigned int weapon = ps->weapon;
    const int clip = weapon ? BG_GetAmmoInClip(ps, weapon) : -1;
    if (weapon != s_measureWeapon)
    {
        s_measureWeapon = weapon;
        s_lastClip = clip;
        s_lastShotTime = 0;
        s_intervalSum = s_intervalCount = 0;
        s_reloadSum = s_reloadCount = 0;
        s_reloadStart = 0;
        return;
    }
    if (clip >= 0 && s_lastClip >= 0 && clip == s_lastClip - 1)
    {
        ++s_shots;
        if (s_lastShotTime && level.time - s_lastShotTime < 1000)
        {
            s_intervalSum += level.time - s_lastShotTime;
            ++s_intervalCount;
        }
        s_lastShotTime = level.time;
    }
    s_lastClip = clip;
    if ((ps->weaponstate == WEAPON_FIRING || ps->weaponstate == WEAPON_RECHAMBERING) && ps->weaponstate == s_prevWeaponState
        && ps->weaponTime > 0 && ps->weaponTime < s_prevWeaponTime && level.time > s_prevTime)
    {
        s_fireDec += s_prevWeaponTime - ps->weaponTime;
        s_fireDt += level.time - s_prevTime;
    }
    s_prevWeaponTime = ps->weaponTime;
    s_prevWeaponState = ps->weaponstate;
    s_prevTime = level.time;
    // KB weapon states 10..16 are the reload states PM_Weapon's timer scaling tests.
    const bool reloading = ps->weaponstate >= 10 && ps->weaponstate <= 16;
    if (reloading && !s_reloadStart)
        s_reloadStart = level.time;
    else if (!reloading && s_reloadStart)
    {
        s_reloadSum += level.time - s_reloadStart;
        ++s_reloadCount;
        char detail[128];
        sprintf_s(detail, "weapon %s reload %d ms", BG_WeaponName(weapon), level.time - s_reloadStart);
        Log("measure", detail);
        s_reloadStart = 0;
    }
}

// powerup_grab (maps\_zombiemode_powerups) deletes the powerup before it notifies "powerup_grabbed", so
// the notify never reaches an entity: a powerup deleted while the test player stood within 64 units of it
// (the script's grab distance) counts as grabbed.
void UpdatePowerups(gentity_s *player)
{
    for (Powerup &powerup : s_powerups)
    {
        if (powerup.ent < 0)
            continue;
        gentity_s *ent = &g_entities[powerup.ent];
        if (!ent->r.inuse || ent->useCount != powerup.useCount)
        {
            if (!powerup.grabbed && powerup.nearest < 64.0f)
            {
                powerup.grabbed = true;
                ++s_powerupsGrabbed;
            }
            Com_Printf(16, "bo1_plan: time %d powerup %s ent %d gone after %d ms grabbed %d\n", level.time,
                powerup.name.c_str(), powerup.ent, level.time - powerup.dropTime, powerup.grabbed);
            LogBoardCounts("at_powerup");
            powerup.ent = -1;
            continue;
        }
        if (powerup.name.empty())
            EntField(powerup.ent, F_POWERUP, &powerup.name, nullptr);
        const float dx = ent->r.currentOrigin[0] - player->r.currentOrigin[0];
        const float dy = ent->r.currentOrigin[1] - player->r.currentOrigin[1];
        const float d = sqrtf(dx * dx + dy * dy);
        if (d < powerup.nearest)
            powerup.nearest = d;
    }
}

// Read-only effect observations. Compare these with script notifications and damage events;
// entity disappearance near a player alone is not proof of a powerup's effect.
void UpdateEffects(gentity_s *player)
{
    // Harness observation only: script-owned sale flags and all box/PaP trigger prices/positions.
    // Log transitions every frame so a 30-second sale's end is not rounded to a polling interval.
    const int current[4] = { ZombieVar(F_FIRESALE), ZombieVar(F_BONFIRE), LevelInt(F_PHONE, -1), LevelInt(F_MUSIC, -1) };
    if (memcmp(s_featureState, current, sizeof(current)) || level.time % 1000 == 0)
    {
        Com_Printf(16, "bo1_feature: time %d fire_sale %d bonfire_sale %d phones %d music_override %d\n",
            level.time, current[0], current[1], current[2], current[3]);
        for (int i = level.maxclients; i < level.num_entities && i < 1024; ++i)
        {
            const gentity_s *ent = &g_entities[i];
            if (!ent->r.inuse || (strcmp(Str(ent->targetname), "treasure_chest_use")
                && strcmp(Str(ent->targetname), "zombie_vending_upgrade")))
                continue;
            int cost = -1;
            EntField(i, !strcmp(Str(ent->targetname), "zombie_vending_upgrade") ? F_PAPCOST : F_COST, nullptr, &cost);
            Com_Printf(16, "bo1_feature: time %d price ent %d %s cost %d origin %.0f %.0f %.0f\n",
                level.time, i, Str(ent->targetname), cost, ent->r.currentOrigin[0], ent->r.currentOrigin[1], ent->r.currentOrigin[2]);
        }
        memcpy(s_featureState, current, sizeof(current));
    }
    const int accessed = LevelInt(F_CHESTACCESSED, -1), moves = LevelInt(F_CHESTMOVES, -1);
    if (accessed != s_chestAccessed || moves != s_chestMoves)
        Com_Printf(16, "bo1_effect: time %d chest_accessed %d chest_moves %d\n", level.time, accessed, moves);
    s_chestAccessed = accessed;
    s_chestMoves = moves;
    const playerState_s *ps = &player->client->ps;
    for (int i = 0; i < 15; ++i)
    {
        const unsigned int weapon = ps->heldWeapons[i].weapon;
        const int reserve = weapon ? BG_GetAmmoNotInClip(ps, weapon) : 0;
        if (weapon && weapon == s_ammoWeapons[i] && reserve > s_reserves[i])
            Com_Printf(16, "bo1_effect: time %d reserve %s %d -> %d clip %d\n", level.time,
                BG_WeaponName(weapon), s_reserves[i], reserve, BG_GetAmmoInClip(ps, weapon));
        s_ammoWeapons[i] = weapon;
        s_reserves[i] = reserve;
    }
    const int score = player->client->sess.cs.score.score;
    if (score != s_effectScore)
        Com_Printf(16, "bo1_effect: time %d score %d -> %d delta %d insta %d scalar %d\n", level.time,
            s_effectScore, score, score - s_effectScore, ZombieVar(F_INSTA), ZombieVar(F_SCALAR));
    s_effectScore = score;
    s_boardCounts[0] = s_boardCounts[1] = s_boardCounts[2] = 0;
    for (int i = level.maxclients; i < level.num_entities && i < 1024; ++i)
    {
        std::string boardState, parameters;
        // Retail update_states writes a string, not the obsolete destroyed boolean.
        if (!g_entities[i].r.inuse || !EntField(i, F_BOARDSTATE, &boardState, nullptr)
            || !EntField(i, F_PARAMETERS, &parameters, nullptr)
            || (parameters != "board" && parameters != "repair_board"))
        {
            s_boardState[i] = 0;
            continue;
        }
        const int state = boardState == "repaired" ? 1 : boardState == "destroyed" ? 2 : 3;
        ++s_boardCounts[state - 1];
        if (s_boardState[i] && state != s_boardState[i])
            Com_Printf(16, "bo1_effect: time %d board ent %d state %s\n", level.time, i, boardState.c_str());
        s_boardState[i] = state;
    }
}

// Nearest live powerup on this floor within reach of a short walk.
// L20: does the straight walk from `from` to `to` cross a live trap volume (zombie_trap trigger switched on, 24 units
// around it)? v1-2 476000: a powerup dropped at -497 2048 behind the just-bought elevator trap (-648 1943 .. -608 2136);
// the chase walked through it and the trap (mod 15, 16 a tick) downed the player at 477600.
bool CrossesLiveTrap(const float *from, const float *to)
{
    for (int i = level.maxclients; i < level.num_entities; ++i)
    {
        const gentity_s *trap = &g_entities[i];
        if (!trap->r.inuse || !IsTrigger(trap) || strcmp(Str(trap->targetname), "zombie_trap") || !TriggerEnabled(trap)
            || from[2] + 70.0f < trap->r.absmin[2] || from[2] > trap->r.absmax[2])
            continue;
        float t0 = 0.0f, t1 = 1.0f;
        bool miss = false;
        for (int axis = 0; axis < 2 && !miss; ++axis)
        {
            const float lo = trap->r.absmin[axis] - 24.0f, hi = trap->r.absmax[axis] + 24.0f;
            const float d = to[axis] - from[axis];
            if (fabsf(d) < 0.001f)
            {
                miss = from[axis] < lo || from[axis] > hi;
                continue;
            }
            float a = (lo - from[axis]) / d, c = (hi - from[axis]) / d;
            if (a > c) { const float s = a; a = c; c = s; }
            t0 = a > t0 ? a : t0;
            t1 = c < t1 ? c : t1;
            miss = t0 > t1;
        }
        if (!miss)
            return true;
    }
    return false;
}

gentity_s *PowerupGoal(gentity_s *player)
{
    gentity_s *best = nullptr;
    float bestDistance = 900.0f;
    for (const Powerup &powerup : s_powerups)
    {
        if (powerup.ent < 0)
            continue;
        gentity_s *ent = &g_entities[powerup.ent];
        if (fabsf(ent->r.currentOrigin[2] - player->r.currentOrigin[2]) > 100.0f
            || CrossesLiveTrap(player->r.currentOrigin, ent->r.currentOrigin))
            continue;
        const float distance = Vec3Distance(ent->r.currentOrigin, player->r.currentOrigin);
        if (distance < bestDistance)
        {
            best = ent;
            bestDistance = distance;
        }
    }
    return best;
}

bool CheckUseSuccess(gentity_s *player, const PlanStep &step, const char **why)
{
    if (!strncmp(step.name, "phone", 5) && s_uses && LevelInt(F_PHONE, -1) == step.count)
    {
        *why = "phone_counter";
        return true;
    }
    playerState_s *ps = &player->client->ps;
    const int spent = s_startScore - player->client->sess.cs.score.score;
    if (step.notify && NotifyTime(step.notify, s_stepStart) >= 0)
    {
        *why = "notify";
        return true;
    }
    if (step.perk && G_SP_HasPerk(player->client, step.perk))
    {
        char detail[160];
        const unsigned int bit = BG_GetPerkIndexForName(step.perk);
        sprintf_s(detail, "%s engine bit %u set %d reload multiplier %.3f", step.perk, bit,
            bit < 52 && (ps->perks[bit >> 5] & (1u << (bit & 31))) != 0,
            Dvar_GetFloat("perk_weapReloadMultiplier"));
        Log("perk", detail);
        *why = "perk";
        return true;
    }
    if (!step.ammo && step.weapon && HasWeaponNamed(ps, step.weapon) && WeaponsChanged(ps))
    {
        *why = "weapon";
        return true;
    }
    const unsigned int ammoWeapon = step.ammo && step.weapon ? BG_FindWeaponIndexForName(step.weapon) : ps->weapon;
    if (step.ammo && spent > 0 && TotalAmmo(ps, ammoWeapon) > s_startAmmo)
    {
        Com_Printf(16, "bo1_effect: time %d ammo purchase %s %d -> %d spent %d\n", level.time,
            BG_WeaponName(ammoWeapon), s_startAmmo, TotalAmmo(ps, ammoWeapon), spent);
        *why = "ammo";
        return true;
    }
    if (!strncmp(step.name, "battery", 7) || !strcmp(step.name, "trap_fix"))
    {
        int piece = 0;
        EntField(player->s.number, F_TRAPPIECE, nullptr, &piece);
        if ((piece == 1) == !strncmp(step.name, "battery", 7) && s_uses)
        {
            *why = "_trap_piece";
            return true;
        }
    }
    // r1 c5 (r4c): s_defconUseTime alone also counts. The notify hook sets it only for a player's pull of this
    // step's own switch; the testclient's reload (usercmd bit 5, usereload) pulls it through Player_ActivateCmd
    // (SP Player_UpdateUse 0x00559690) while walking through the trigger (r4c 184350: 117 pulled at -1067 2513
    // with the freezegun reloading, not credited, the step pulled 109 too and the last defcon step found level 5).
    if (!strcmp(step.name, "defcon") && (s_uses || s_defconUseTime))
    {
        const int defcon = LevelInt(F_DEFCON, -1);
        if (s_defconUseTime && defcon > s_phase && s_phase > 0 && s_phase < 5
            && s_ent >= 0 && !DefconSwitchReady(&g_entities[s_ent])
            && NotifyTime("powerup bonfire sale", s_defconUseTime) < 0)
        {
            *why = "defcon_level";
            return true;
        }
    }
    // L28 Kino traps: the trap (zombie_trap whose target is this trigger's targetname) runs: _trap_in_use 1.
    const bool kinoTrap = !strncmp(step.name, "k_trap_", 7);
    if (kinoTrap && s_uses)
    {
        for (int i = 0; i < level.num_entities; ++i)
        {
            const gentity_s *trap = &g_entities[i];
            int inUse = 0;
            if (trap->r.inuse && !strcmp(Str(trap->targetname), "zombie_trap") && !strcmp(Str(trap->target), step.targetname)
                && EntField(i, F_TRAPINUSE, nullptr, &inUse) && inUse == 1)
            {
                *why = "_trap_in_use";
                return true;
            }
        }
    }
    // L28 Kino teleporter: count 1 any teleport since the step started; count 2 a landing on the theater's return
    // points since that teleport out (the script returns the player after 30 s, maybe during the next step).
    if (!strncmp(step.name, "k_tp_", 5) && step.count == 1 && s_stepStart && s_teleportTime > s_stepStart)
    {
        s_kinoOutTime = s_teleportTime;
        *why = "teleport";
        return true;
    }
    if (!strncmp(step.name, "k_tp_", 5) && step.count == 2 && s_kinoOutTime && s_kinoReturnTime > s_kinoOutTime)
    {
        *why = "teleport_back";
        return true;
    }
    // L35 Der Riese traps: the damage trigger (targetname = this use trigger's target) has in_use 1
    // (zombie_cod5_factory.gsc electric_trap_think / electric_trap_activate).
    const bool factoryTrap = !strncmp(step.name, "f_trap_", 7);
    if (factoryTrap && s_uses && s_ent >= 0 && g_entities[s_ent].target)
    {
        for (int i = 0; i < level.num_entities; ++i)
        {
            const gentity_s *dmg = &g_entities[i];
            int inUse = 0;
            if (dmg->r.inuse && dmg->targetname == g_entities[s_ent].target && EntField(i, F_INUSE, nullptr, &inUse) && inUse == 1)
            {
                *why = "in_use";
                return true;
            }
        }
    }
    // L35 Der Riese pads: the countdown sets no level-visible state; the use press passes, f_tp_link<n> proves it.
    if (!strncmp(step.name, "f_tp_pad_", 9) && s_uses)
    {
        *why = "used (proof: the next f_tp_link step)";
        return true;
    }
    if (!strcmp(step.name, "f_tp_go") && s_stepStart && s_teleportTime > s_stepStart)
    {
        *why = "teleport";
        return true;
    }
    if (kinoTrap || factoryTrap || !strcmp(step.name, "f_tp_go"))
        return false;
    if (!step.notify && !step.perk && !step.weapon && !step.ammo && step.cost && spent >= step.cost)
    {
        *why = "score";
        return true;
    }
    // r1 c3: one payment of the full cost in a single frame, whatever the step earned before it.
    if (!step.notify && !step.perk && !step.weapon && !step.ammo && step.cost && s_largestDrop >= step.cost)
    {
        *why = "payment";
        return true;
    }
    return false;
}

// r1: the Pentagon thief round (maps\_zombiemode_ai_thief). thief_round_tracker sets level flag "thief_round" at
// between_round_over and thief_round_spawning spawns one thief (actor_zombie_electrician) instead of zombies. It
// hunts the player, thief_steal locks him (FreezeControls), thief_take_loot takes the held gun and
// thief_take_player SetOrigins him to the "thief_player_pos" struct by the power room. thief_end_game then runs
// through the bottom portals and thief_exit_level deletes it (escape: loot lost, full_ammo at thief_start); a kill
// runs thief_zombie_die (thief_return_loot, fire_sale or bonfire_sale, full_ammo). Both set "last_thief_down".
// Harness policy only: while the thief lives the plan's step is suspended (logged as "thief round" lines, never
// PASS/FAIL), the fight code shoots the thief first, the plan chases it after the steal when a gun has ammo, or
// waits it out when none has; then the thief's drops are collected and the interrupted step starts again.
bool s_thiefMode;
int s_thiefStart;
int s_thiefEnt = -1;
int s_thiefLogTime;
int s_thiefMissing;   // level.time the thief was first missing while the thief round lasts
int s_thiefKilled;    // level.time of the thief's death notify (health <= 0), 0 while it lives
int s_thiefRounds;    // thief rounds met on this map
int s_thiefDropsUntil; // after the thief: collect its drops until this time
int s_thiefLastHealth;
std::string s_thiefState;

// level.flag[name] (maps\_utility flag_init / flag_set / flag_clear), find-only: -1 when missing.
int LevelFlag(const char *name)
{
    const unsigned int flagKey = s_fieldKeys[F_LEVELFLAG];
    const unsigned int id = flagKey ? FindVariable(SCRIPTINSTANCE_SERVER, gScrVarPub[SCRIPTINSTANCE_SERVER].levelId, flagKey) : 0;
    if (!id || GetValueType(SCRIPTINSTANCE_SERVER, id) != VAR_POINTER)
        return -1;
    // Array indices are string IDs, as in ZombieVar.
    const unsigned int key = SL_FindString(name, SCRIPTINSTANCE_SERVER);
    const unsigned int entry = key ? FindVariable(SCRIPTINSTANCE_SERVER,
        GetVariableValueAddress(SCRIPTINSTANCE_SERVER, id)->u.pointerValue, key) : 0;
    if (!entry || GetValueType(SCRIPTINSTANCE_SERVER, entry) != VAR_INTEGER)
        return -1;
    return GetVariableValueAddress(SCRIPTINSTANCE_SERVER, entry)->u.intValue;
}

gentity_s *ThiefActor()
{
    for (int i = 0; level.actors && i < MAX_ACTORS; ++i)
    {
        actor_s *actor = &level.actors[i];
        gentity_s *ent = actor->ent;
        if (actor->inuse && ent && ent->health > 0 && actor->Physics.bIsAlive && ent->classname
            && !strcmp(Str(ent->classname), "actor_zombie_electrician"))
            return ent;
    }
    return nullptr;
}

bool HasLoadedGun(const playerState_s *ps)
{
    for (int i = 0; i < 15; ++i)
    {
        const unsigned int held = ps->heldWeapons[i].weapon;
        if (held && BG_GetWeaponDef(held)->weapType == WEAPTYPE_BULLET
            && BG_GetWeaponDef(held)->inventoryType == WEAPINVENTORY_PRIMARY
            && BG_GetAmmoInClip(ps, held) + BG_GetAmmoNotInClip(ps, held) > 0)
            return true;
    }
    return false;
}

void ThiefLog(gentity_s *player, const char *event, const char *detail)
{
    Com_Printf(16, "bo1_plan: time %d thief round %d %s step %d %s: %s weapon %s weapons %s origin %.0f %.0f %.0f\n",
        level.time, s_thiefRounds, event, s_index, s_index < (int)s_plan.size() ? s_steps[s_plan[s_index]].name : "-",
        detail, player->client->ps.weapon ? BG_WeaponName(player->client->ps.weapon) : "none",
        WeaponList(&player->client->ps).c_str(), player->r.currentOrigin[0], player->r.currentOrigin[1],
        player->r.currentOrigin[2]);
}

// -1: no thief round business, run the step; 0: the fight code alone (return false); 1: the plan moved (return true).
int ThiefRound(gentity_s *player, usercmd_s *cmd, gentity_s *target, bool melee)
{
    const int round = LevelFlag("thief_round");
    gentity_s *thief = round == 1 ? ThiefActor() : nullptr;
    char detail[256];
    if (!s_thiefMode)
    {
        if (!thief || s_index >= (int)s_plan.size())
        {
            if (level.time < s_thiefDropsUntil && s_index < (int)s_plan.size())
            {
                // After the thief: its fire_sale / bonfire_sale / full_ammo drops (and the escape's full_ammo at
                // thief_start) on this floor, at any distance on it, before the step starts again.
                gentity_s *best = nullptr;
                float bestDistance = 3000.0f;
                for (const Powerup &powerup : s_powerups)
                {
                    if (powerup.ent < 0 || powerup.grabbed || powerup.dropTime < s_thiefStart)
                        continue;
                    gentity_s *ent = &g_entities[powerup.ent];
                    if (fabsf(ent->r.currentOrigin[2] - player->r.currentOrigin[2]) > 100.0f
                        || CrossesLiveTrap(player->r.currentOrigin, ent->r.currentOrigin)) // L20
                        continue;
                    const float distance = Vec3Distance(ent->r.currentOrigin, player->r.currentOrigin);
                    if (distance < bestDistance)
                    {
                        best = ent;
                        bestDistance = distance;
                    }
                }
                if (!best)
                {
                    s_thiefDropsUntil = 0;
                    ThiefLog(player, "drops done", "no drop left on this floor; the step starts again");
                    return -1;
                }
                if (player->client->lastStand || melee)
                    return 0;
                float viewYaw = ViewYaw(player, cmd);
                if (!target)
                {
                    Face(player, cmd, best->r.currentOrigin, false);
                    viewYaw = ViewYaw(player, cmd);
                }
                Walk(player, best->r.currentOrigin, 2000 + best->s.number, cmd, viewYaw);
                return 1;
            }
            return -1;
        }
        s_thiefMode = true;
        s_thiefStart = level.time;
        s_thiefEnt = thief->s.number;
        s_thiefLogTime = 0;
        s_thiefMissing = 0;
        s_thiefKilled = 0;
        s_thiefState.clear();
        ++s_thiefRounds;
        // r1 arm policy: the steal teleports the player to the power room; the step goes on from its own floor.
        s_thiefStepFloor = Floor(s_stepStart ? s_startOrigin[2] : player->r.currentOrigin[2]);
        sprintf_s(detail, "thief ent %d health %d at %.0f %.0f %.0f, round %d; step suspended, retried after the thief",
            s_thiefEnt, thief->health, thief->r.currentOrigin[0], thief->r.currentOrigin[1], thief->r.currentOrigin[2],
            LevelInt(F_ROUND, -1));
        ThiefLog(player, "start", detail);
    }
    if (!thief)
    {
        // Gone: killed (thief_zombie_die) or deleted by thief_exit_level; both flag_set "last_thief_down".
        // Also a thief missing for 3 s with the round flag still on (never seen; guard only).
        if (!s_thiefMissing)
            s_thiefMissing = level.time;
        if (round == 1 && LevelFlag("last_thief_down") != 1 && level.time - s_thiefMissing < 3000)
        {
            cmd->forwardmove = cmd->rightmove = 0;
            return 1;
        }
        s_thiefMode = false;
        sprintf_s(detail, "after %d ms: thief ent %d %s (last health %d), last_thief_down %d, thief_round %d",
            level.time - s_thiefStart, s_thiefEnt, s_thiefKilled ? "killed" : "escaped", s_thiefLastHealth,
            LevelFlag("last_thief_down"), round);
        ThiefLog(player, "end", detail);
        // Retry the interrupted step from wherever the player is now (StartStep resets its state and timeout;
        // teleports before the new start do not count for it).
        s_stepStart = 0;
        s_thiefDropsUntil = level.time + 30000;
        s_thiefReturn = true;
        return ThiefRound(player, cmd, target, melee);
    }
    s_thiefMissing = 0;
    s_thiefLastHealth = thief->health;
    std::string state;
    EntField(thief->s.number, F_BOARDSTATE, &state, nullptr);
    const playerState_s *ps = &player->client->ps;
    const float distance = Vec3Distance(thief->r.currentOrigin, player->r.currentOrigin);
    if (state != s_thiefState || level.time >= s_thiefLogTime)
    {
        sprintf_s(detail, "thief state %s health %d distance %.0f at %.0f %.0f %.0f targeted %d loaded %d laststand %d",
            state.c_str(), thief->health, distance, thief->r.currentOrigin[0], thief->r.currentOrigin[1],
            thief->r.currentOrigin[2], target == thief ? 1 : 0, HasLoadedGun(ps) ? 1 : 0, player->client->lastStand ? 1 : 0);
        ThiefLog(player, state != s_thiefState ? "state" : "status", detail);
        s_thiefState = state;
        s_thiefLogTime = level.time + 2000;
    }
    if (player->client->lastStand || melee)
        return 0;
    // Before the steal the thief comes to the player (thief_zombie_hunt / thief_portal_to_victim): stand and let the
    // fight code shoot it. After the steal (state "exiting", thief_end_game) chase it with a loaded gun; with none,
    // wait it out where the script put the player.
    const bool exiting = state == "exiting";
    // L18 TEST SWITCH: the weapons the thief took come back only if it dies; the harness's guns rarely manage that, so
    // deal the player's lethal damage once it is leaving with the loot (the death and the return are the scripts').
    if (exiting && s_thiefKillDvar && s_thiefKillDvar->current.enabled && thief->health > 0)
    {
        Com_Printf(16, "bo1_plan: time %d TEST SWITCH thiefkill ent %d health %d weapon %s\n", level.time,
            thief->s.number, thief->health, BG_WeaponName(ps->weapon));
        bgs_t *const savedBgs = bgs;
        bgs = &level_bgs;
        G_Damage(thief, player, player, nullptr, thief->r.currentOrigin, thief->health + 1000, 0, MOD_RIFLE_BULLET,
            ps->weapon, HITLOC_HEAD, 0, 0, 0);
        bgs = savedBgs;
        return 1;
    }
    if (exiting && HasLoadedGun(ps) && (target != thief || distance > 600.0f))
    {
        float viewYaw = ViewYaw(player, cmd);
        if (!target)
        {
            Face(player, cmd, thief->r.currentOrigin, false);
            viewYaw = ViewYaw(player, cmd);
        }
        Walk(player, thief->r.currentOrigin, 3000 + thief->s.number, cmd, viewYaw);
        return 1;
    }
    cmd->forwardmove = cmd->rightmove = 0;
    // L10 TEST SWITCH: circle-strafe (still aiming) until the grab, so the steal catches a moving player.
    if (!exiting && s_thiefStrafeDvar && s_thiefStrafeDvar->current.enabled)
        cmd->rightmove = 127;
    if (!target)
        Face(player, cmd, thief->r.currentOrigin, false);
    return 1;
}
} // namespace

// w1 c11 pap_thief: the script side while the gun is in the machine, also while ThiefRound suspends the step.
// vending_weapon_upgrade: flag_set pack_machine_in_use at the purchase, flag_clear after pap_taken / pap_timeout
// (wait_for_timeout: packapunch_timeout 15 s after third_person_weapon_upgrade); pack_hideaway_init waits for the
// flag before it turns the machine away. The step ends when the flag clears.
static void PapThiefObserve(gentity_s *player)
{
    if (s_index >= (int)s_plan.size() || !s_stepStart || s_phase != 1 || strncmp(s_steps[s_plan[s_index]].name, "pap_thief", 9))
        return;
    static int s_from = -1, s_inUse, s_thief, s_trigger, s_upgraded, s_room, s_hideaway;
    static std::string s_weapons;
    const playerState_s *ps = &player->client->ps;
    const int inUse = LevelFlag("pack_machine_in_use"), thief = LevelFlag("thief_round");
    const int trigger = s_ent >= 0 ? (TriggerEnabled(&g_entities[s_ent]) ? 1 : 0) : -1;
    const int upgraded = HasUpgradedWeapon(ps) ? 1 : 0, room = InPackRoom(player->r.currentOrigin) ? 1 : 0;
    const int hideaway = LevelFlag("open_pack_hideaway");
    const std::string weapons = WeaponList(ps);
    if (s_from != s_phaseTime)
    {
        s_from = s_phaseTime;
        s_inUse = s_thief = s_trigger = s_upgraded = s_room = s_hideaway = -2;
        s_weapons.clear();
    }
    if (inUse != s_inUse || thief != s_thief || trigger != s_trigger || upgraded != s_upgraded || room != s_room
        || hideaway != s_hideaway || weapons != s_weapons)
    {
        char detail[320];
        sprintf_s(detail, "paid+%d: pack_machine_in_use %d thief_round %d trigger %d upgraded %d packroom %d hideaway %d weapons %s",
            level.time - s_phaseTime, inUse, thief, trigger, upgraded, room, hideaway, weapons.c_str());
        Log("observe", detail);
    }
    const bool cleared = s_inUse == 1 && inUse == 0;
    s_inUse = inUse;
    s_thief = thief;
    s_trigger = trigger;
    s_upgraded = upgraded;
    s_room = room;
    s_hideaway = hideaway;
    s_weapons = weapons;
    if (cleared)
        EndStep(player, "PASS", upgraded ? "pack_machine_in_use cleared: the upgraded gun was taken"
            : "pack_machine_in_use cleared: timed out, the gun stayed in the machine");
}

void G_SP_TestPlanRegister()
{
    if (!Sys_IsHeadless())
        return;
    s_planDvar = _Dvar_RegisterString("bo1_testclient_plan", "", 0,
        "Headless test client feature plan: step names or 'all' (needs bo1_testclient_fight)");
    s_planAltDvar = _Dvar_RegisterString("bo1_testclient_plan_alt", "", 0,
        "Harness only: the plan used instead when the player's start y is above bo1_testclient_plan_alty");
    s_planAltYDvar = _Dvar_RegisterFloat("bo1_testclient_plan_alty", 0.0f, -131072.0f, 131072.0f, 0,
        "Harness only: start y threshold for bo1_testclient_plan_alt");
    s_resupplyDvar = _Dvar_RegisterBool("bo1_testclient_resupply", false, 0,
        "Harness only: after the feature plan, walk to a wall weapon on the current floor to buy ammo");
    s_dryAmmoDvar = _Dvar_RegisterInt("bo1_testclient_dryammo", 0, 0, 0x7FFFFFFF, 0,
        "TEST SWITCH (headless soaks only, 0 = off): after the held weapon has been dry this many level ms, apply the retail "
        "'give ammo' cheat (Cmd_Give_f: Add_Ammo 998 to every held weapon), so a pinned god-mode player keeps killing");
    s_scoreDvar = _Dvar_RegisterInt("bo1_testclient_score", 0, 0, 0x7FFFFFFF, 0, // zombies: SP's score field has no 30000 cap
        "TEST SWITCH (headless test client only): raise the test player's score field to this before a purchase it cannot pay");
    s_quitDoneDvar = _Dvar_RegisterInt("bo1_testclient_quitdone", 0, 0, 0x7FFFFFFF, 0,
        "Headless test client: quit this many level ms after the feature plan completes (0 = off; regression runs)");
    s_shotsDvar = _Dvar_RegisterBool("bo1_testclient_shots", false, 0,
        "Headless test client with -Client: box_leave / pap / pap_view take their evidence screenshots");
    s_thiefKillDvar = _Dvar_RegisterBool("bo1_testclient_thiefkill", false, 0,
        "TEST SWITCH (headless test client): once the thief has stolen (state exiting), the player's damage kills it, so the retail thief_zombie_die / thief_return_loot run");
    s_thiefStrafeDvar = _Dvar_RegisterBool("bo1_testclient_thiefstrafe", false, 0,
        "TEST SWITCH (headless test client): strafe sideways at full speed while the thief comes for the player (the grab of a moving player)");
}

void G_SP_TestPlanReset()
{
    s_boxViewValid = false;
    s_boxViewStraight = false;
    s_plan.clear();
    s_planFlags.clear();
    s_planReplace.clear();
    s_holdWeapon = 0;
    s_hadTwoGuns = false;
    s_recoveryFails = 0;
    s_floorRequeues = 0;
    s_waitResupplies = 0;
    s_papRequeues = 0;
    s_dryStart = -1;
    s_lastTrapUse = -1;
    s_lastTrapUseTime = -1;
    s_thiefReturn = false;
    s_planParsed = false;
    s_index = 0;
    s_stepStart = 0;
    s_ent = -1;
    s_complete = false;
    s_quitSent = false;
    s_usedEnts.clear();
    s_crawlersSeen.clear();
    s_levelNotifies.clear();
    memset(s_triggerNotify, 0, sizeof(s_triggerNotify));
    s_powerups.clear();
    for (int &value : s_featureState)
        value = -2;
    s_powerupsGrabbed = 0;
    memset(s_ammoWeapons, 0, sizeof(s_ammoWeapons));
    memset(s_reserves, 0, sizeof(s_reserves));
    memset(s_boardState, 0, sizeof(s_boardState));
    s_effectScore = 0;
    s_chestAccessed = s_chestMoves = -1;
    s_measureWeapon = 0;
    s_lastClip = -1;
    s_shots = 0;
    s_teleportTime = 0;
    s_kinoOutTime = 0;
    s_kinoReturnTime = 0;
    s_unstickUntil = 0;
    s_pathGoal = -1;
    s_thiefMode = false;
    s_thiefEnt = -1;
    s_thiefRounds = 0;
    s_thiefDropsUntil = 0;
    s_thiefKilled = 0;
}

void G_SP_TestPlanFields(scriptInstance_t inst)
{
    if (inst != SCRIPTINSTANCE_SERVER || !Sys_IsHeadless())
        return;
    for (int i = 0; i < F_COUNT; ++i)
    {
        const unsigned int string = SL_FindString(s_fieldNames[i], inst);
        s_fieldKeys[i] = string ? gScrCompilePub[inst].canonicalStrings[string] : 0;
    }
}

// L41: harness observation of a script effect that has no notify (GScr_SetClientFlag on a script mover: Shangri-La's
// spear-trap spikes set clientflag 3). Recorded like a level notify.
void G_SP_TestPlanObserve(const char *name)
{
    if (!Enabled())
        return;
    if (NotifyTime(name, level.time) < 0)
        Com_Printf(16, "bo1_plan: time %d observe %s\n", level.time, name);
    s_levelNotifies.push_back({ name, level.time });
}

void G_SP_TestPlanNotify(scriptInstance_t inst, unsigned int owner, unsigned int name, const VariableValue *top)
{
    if (inst != SCRIPTINSTANCE_SERVER || !Enabled())
        return;
    const char *notify = Str(name);
    if (owner == gScrVarPub[inst].levelId)
    {
        s_levelNotifies.push_back({ notify, level.time });
        // Harness observation: thief round / power / round transitions (flag_set and flag_clear notify level).
        static const char *const s_loggedLevelNotifies[] = { "thief_round", "last_thief_down", "death_in_pre_game",
            "stop_thief_alarms", "between_round_over", "start_of_round", "end_of_round", "power_on", "power_off",
            "teleporter_powered", "defcon_reset" };
        for (const char *logged : s_loggedLevelNotifies)
            if (!strcmp(notify, logged))
                Com_Printf(16, "bo1_feature: time %d level notify %s round %d next_thief_round %d\n", level.time, notify,
                    LevelInt(F_ROUND, -1), LevelInt(F_NEXTTHIEF, -1));
        if (!strcmp(notify, "powerup_dropped") && top->type == VAR_POINTER
            && GetObjectType(inst, top->u.pointerValue) == VAR_ENTITY)
        {
            scr_entref_t ref = Scr_GetEntityIdRef(inst, top->u.pointerValue);
            if (!ref.classnum && ref.entnum < (unsigned)level.num_entities)
            {
                s_powerups.push_back({ (int)ref.entnum, g_entities[ref.entnum].useCount, level.time, "", false, 1.0e9f });
                Com_Printf(16, "bo1_plan: time %d powerup dropped ent %d at %.0f %.0f %.0f\n", level.time, ref.entnum,
                    g_entities[ref.entnum].r.currentOrigin[0], g_entities[ref.entnum].r.currentOrigin[1],
                    g_entities[ref.entnum].r.currentOrigin[2]);
            }
        }
        return;
    }
    if (GetObjectType(inst, owner) != VAR_ENTITY)
        return;
    scr_entref_t ref = Scr_GetEntityIdRef(inst, owner);
    if (ref.classnum || ref.entnum >= 1024)
        return;
    if (name == scr_const.trigger)
        s_triggerNotify[ref.entnum] = level.time;
    // L42: a Wave Gun kill. The gun hurts zombies only through its script sizzle (DoDamage with the player as attacker,
    // _zombiemode_weap_microwavegun.gsc:317-334); "microwaved" is only for non-AI _microwaveable_objects (:293). The
    // spawner's death event notifies the attacker "zom_kill" (_zombiemode_spawner.gsc:3850): with the Wave Gun raised and
    // not meleeing, that kill is the gun's.
    if (!strcmp(notify, "zom_kill") && ref.entnum < level.maxclients && g_entities[ref.entnum].client)
    {
        const playerState_s *zps = &g_entities[ref.entnum].client->ps;
        const char *held = zps->weapon ? BG_WeaponName(zps->weapon) : "";
        if ((!strcmp(held, "microwavegun_zm") || !strcmp(held, "microwavegun_upgraded_zm"))
            && (zps->weaponstate < WEAPON_MELEE_INIT || zps->weaponstate > WEAPON_MELEE_END))
            G_SP_TestPlanObserve("wavegun_kill");
    }
    // L42: the moon astronaut's headbutt notetracks (maps/_zombiemode_ai_astro.gsc:540-557): "grabbed", then "fire".
    if (!strcmp(notify, "headbutt_anim") && top->type == VAR_STRING)
    {
        const char *track = Str(top->u.stringValue);
        if (!strcmp(track, "grabbed") || !strcmp(track, "fire"))
            G_SP_TestPlanObserve(!strcmp(track, "fire") ? "astro_headbutt_fire" : "astro_grabbed");
    }
    // L40 harness observation: the coast director's (George's) own entity notifies and a rider's exit_zipline are plan evidence.
    if (!strncmp(notify, "director_", 9) || !strcmp(notify, "exit_zipline") || !strcmp(notify, "trap_done")
        // L41: Shangri-La: a slide rider's water_slide_exit, a shrunk zombie's shrink / unshrink, a PaP plate's pap_active.
        || !strcmp(notify, "water_slide_exit") || !strcmp(notify, "shrink") || !strcmp(notify, "unshrink")
        || !strcmp(notify, "pap_active") || !strcmp(notify, "minecart_exit")
        // L41: a shrunk zombie's kicked / stepped_on (_zombiemode_weap_shrink_ray.gsc:539-566).
        || !strcmp(notify, "kicked") || !strcmp(notify, "stepped_on")
        // L42: the P.E.S. put on: the equipment slot watcher's player notify (_zombiemode_equipment.gsc:228-275).
        || !strcmp(notify, "equip_gasmask_zm_activate")
        // L42: moon evidence: a player going down (oxygen), a zombie hit by the Wave Gun.
        || !strcmp(notify, "player_downed") || !strcmp(notify, "microwaved"))
        s_levelNotifies.push_back({ notify, level.time });
    // Harness: Bonfire Sale notifies all four switches without a user. Only a
    // player-triggered pull followed by the script's increase can pass this step.
    if (name == scr_const.trigger && ref.entnum == s_ent && s_index < (int)s_plan.size()
        && !strcmp(s_steps[s_plan[s_index]].name, "defcon") && DefconSwitchReady(&g_entities[ref.entnum])
        && top->type == VAR_POINTER && GetObjectType(inst, top->u.pointerValue) == VAR_ENTITY)
    {
        const scr_entref_t user = Scr_GetEntityIdRef(inst, top->u.pointerValue);
        if (!user.classnum && user.entnum < level.maxclients && g_entities[user.entnum].client)
        {
            s_phase = LevelInt(F_DEFCON, 1);
            s_defconUseTime = level.time;
        }
    }
    if (!strcmp(notify, "explode") && (int)ref.entnum == s_quantumMissile)
    {
        s_quantumMissile = -1;
        s_levelNotifies.push_back({ "quantum_bomb_explode", level.time });
    }
    if (!strcmp(notify, "grenade_fire") && top->type == VAR_POINTER
        && GetObjectType(inst, top->u.pointerValue) == VAR_ENTITY)
    {
        const scr_entref_t grenade = Scr_GetEntityIdRef(inst, top->u.pointerValue);
        Com_Printf(16, "bo1_effect: time %d grenade_fire player %d missile %d weapon %s trType %d gravity %d\n", level.time,
            ref.entnum, grenade.entnum, (top - 1)->type == VAR_STRING ? Str((top - 1)->u.stringValue) : "",
            g_entities[grenade.entnum].s.lerp.pos.trType, // L42: 15 = TR_MOON_GRAVITY (G_GetGrenadeTrType, SP 0x007e68a0)
            g_entities[ref.entnum].client ? g_entities[ref.entnum].client->ps.gravity : 0);
        if ((top - 1)->type == VAR_STRING && !strcmp(Str((top - 1)->u.stringValue), "zombie_quantum_bomb"))
            s_quantumMissile = grenade.entnum;
    }
    if (name == scr_const.trigger && !strcmp(Str(g_entities[ref.entnum].targetname), "zombie_trap")
        && top->type == VAR_POINTER && GetObjectType(inst, top->u.pointerValue) == VAR_ENTITY)
    {
        const scr_entref_t toucher = Scr_GetEntityIdRef(inst, top->u.pointerValue);
        Com_Printf(16, "bo1_effect: time %d trap_touch ent %d toucher %d actor %d\n", level.time,
            ref.entnum, toucher.entnum, !toucher.classnum && toucher.entnum < 1024 && g_entities[toucher.entnum].actor != nullptr);
    }
    if (!strcmp(notify, "trap_activate") || !strcmp(notify, "trap_done") || !strcmp(notify, "available"))
        Com_Printf(16, "bo1_effect: time %d notify %s ent %d targetname %s\n", level.time,
            notify, ref.entnum, Str(g_entities[ref.entnum].targetname));
    // Scripts also use death() to cancel threads during setup; retain actual damage/death evidence.
    if (g_entities[ref.entnum].actor && ((name == scr_const.damage && top->type == VAR_INTEGER)
        || (name == scr_const.death && g_entities[ref.entnum].health <= 0)))
    {
        const VariableValue *source = name == scr_const.damage && top->type == VAR_INTEGER ? top - 1 : top;
        if (name == scr_const.death && s_thiefMode && (int)ref.entnum == s_thiefEnt)
            s_thiefKilled = level.time;
        int attacker = -1;
        if (source->type == VAR_POINTER && GetObjectType(inst, source->u.pointerValue) == VAR_ENTITY)
            attacker = Scr_GetEntityIdRef(inst, source->u.pointerValue).entnum;
        Com_Printf(16, "bo1_effect: time %d actor %d %s health %d damage %d source %d source_name %s insta %d scalar %d\n",
            level.time, ref.entnum, notify, g_entities[ref.entnum].health,
            name == scr_const.damage && top->type == VAR_INTEGER ? top->u.intValue : 0, attacker,
            attacker >= 0 && attacker < 1024 ? Str(g_entities[attacker].targetname) : "", ZombieVar(F_INSTA), ZombieVar(F_SCALAR));
        // SP actor death carries attacker, meansOfDeath, weapon; setup death notifies
        // have no arguments. Check each preceding argument before reading the next.
        if (name == scr_const.death && (top->type == VAR_POINTER || top->type == VAR_UNDEFINED)
            && (top - 1)->type == VAR_STRING)
        {
            Com_Printf(16, "bo1_effect: time %d actor %d death mod %s weapon %s classname %s origin %.0f %.0f %.0f\n", level.time,
                ref.entnum, Str((top - 1)->u.stringValue),
                (top - 2)->type == VAR_STRING ? Str((top - 2)->u.stringValue) : "", Str(g_entities[ref.entnum].classname),
                g_entities[ref.entnum].r.currentOrigin[0], g_entities[ref.entnum].r.currentOrigin[1],
                g_entities[ref.entnum].r.currentOrigin[2]);
            if ((top - 2)->type == VAR_STRING && !strcmp(Str((top - 2)->u.stringValue), "claymore_zm"))
                s_claymoreKillTime = level.time;
        }
    }
    if (!strcmp(notify, "zmb_max_ammo"))
        s_maxAmmoTime = level.time;
    if (!strcmp(notify, "pap_taken") || !strcmp(notify, "pap_timeout")
        || !strcmp(notify, "zmb_max_ammo") || !strcmp(notify, "nuke_triggered")
        || !strcmp(notify, "trap_piece_returned") || !strcmp(notify, "_piece_placed"))
        Com_Printf(16, "bo1_plan: time %d notify %s ent %d\n", level.time, notify, ref.entnum);
    if (!strcmp(notify, "powerup_grabbed"))
        for (Powerup &powerup : s_powerups)
            if (powerup.ent == (int)ref.entnum)
            {
                powerup.grabbed = true;
                EntField(powerup.ent, F_POWERUP, &powerup.name, nullptr);
                ++s_powerupsGrabbed;
                Com_Printf(16, "bo1_plan: time %d powerup grabbed %s ent %d\n", level.time, powerup.name.c_str(), ref.entnum);
            }
}

// r1: the thief the fight code shoots before other zombies while the plan's thief round policy runs.
gentity_s *G_SP_TestPlanThief(gentity_s *player)
{
    if (!Enabled() || !s_thiefMode || !HasLoadedGun(&player->client->ps))
        return nullptr;
    return ThiefActor();
}

bool G_SP_TestPlanActive()
{
    return Enabled();
}

bool G_SP_TestPlanComplete()
{
    return Enabled() && s_complete;
}

unsigned int G_SP_TestPlanHoldWeapon()
{
    return Enabled() ? s_holdWeapon : 0;
}

// L20 harness policy: the fighting harness backs away from a zombie inside 128 units only without a plan
// (g_sp_client.cpp), so a plan step walked into zombies or held its spot while the gun reloaded. Downs in the
// regression logs (notes/L20-report.md): b1b3 729950, base1 841900 (wait110 spot, m14 reloading at 23-34 units,
// move 0 0), b111 763750 (rpk reloading at 43 units, then the wait walk through three zombies), L141 775600 (the
// walk back to the wait spot, m14 reloading at 37 units). 2 for the long wait (a hold of a spot, as the free fight),
// 1 for other steps (only while the gun cannot fire: the walk resumes once it can), 0 where the step must hold
// still: an elevator panel pressed or the ride (STEP_FLOOR phase 2/3), a paid box / PaP weapon's pickup window,
// deploys, throws, views, measured fire, a thief round (ThiefRound moves the player), and next to a live trap volume
// (r6h: the electric trap downs the player; the wait's spot is 160 units outside it).
int G_SP_TestPlanMayRetreat(const gentity_s *player)
{
    if (!Enabled() || s_complete || s_index < 0 || s_index >= (int)s_plan.size())
        return 2;
    if (s_thiefMode)
        return 0;
    const PlanStep &step = s_steps[s_plan[s_index]];
    switch (step.kind)
    {
    case STEP_FLOOR:
        if (s_phase >= 2)
            return 0;
        break;
    case STEP_BOX:
    case STEP_PAP:
        if (s_phase == 1)
            return 0;
        break;
    case STEP_USE:
        if (!strcmp(step.name, "wall_frag") && s_phase)
            return 0;
        break;
    case STEP_WAIT:
    case STEP_TOUCH:
        break;
    default:
        return 0; // views, fire / equip measurement, deploy, grenade, crawler, glass, devgui drops
    }
    for (int i = level.maxclients; i < level.num_entities; ++i)
    {
        const gentity_s *trap = &g_entities[i];
        if (!trap->r.inuse || !IsTrigger(trap) || strcmp(Str(trap->targetname), "zombie_trap") || !TriggerEnabled(trap)
            || player->r.currentOrigin[2] + 70.0f < trap->r.absmin[2] || player->r.currentOrigin[2] > trap->r.absmax[2])
            continue;
        const float *o = player->r.currentOrigin;
        const float dx = o[0] < trap->r.absmin[0] ? trap->r.absmin[0] - o[0] : o[0] > trap->r.absmax[0] ? o[0] - trap->r.absmax[0] : 0.0f;
        const float dy = o[1] < trap->r.absmin[1] ? trap->r.absmin[1] - o[1] : o[1] > trap->r.absmax[1] ? o[1] - trap->r.absmax[1] : 0.0f;
        if (dx * dx + dy * dy < 96.0f * 96.0f)
            return 0;
    }
    return step.kind == STEP_WAIT && s_phase ? 2 : 1;
}

// q1 c15 harness only: the retail script's self.has_legs is false (a crawler, zombie_gib_on_damage).
bool G_SP_TestPlanActorLegless(int entnum)
{
    int legs = 1;
    return EntField(entnum, F_HASLEGS, nullptr, &legs) && !legs;
}

bool G_SP_TestPlanHoldingGrenade()
{
    if (!Enabled() || s_complete || s_index < 0 || s_index >= (int)s_plan.size())
        return false;
    const PlanStep &step = s_steps[s_plan[s_index]];
    return step.kind == STEP_GRENADE && s_phase && level.time - s_stepStart < (step.count ? step.count : 300) + 100;
}

// r1: a regression run ends with its route (r2f: an idle player died 47 s after "complete" once the run was
// lengthened for thief rounds; that death is no part of the route). q1 c9: checked every server frame of the test
// client (G_SP_HeadlessFightCommand), not only when the plan runs: a11m completed in last stand, the plan never ran
// again (last stand / bleed-out / intermission return before it) and the game over's fast restart ran a second map.
void G_SP_TestPlanQuitCheck()
{
    if (!Enabled() || !s_complete || s_quitSent || s_quitDoneDvar->current.integer <= 0
        || level.time - s_completeTime < s_quitDoneDvar->current.integer)
        return;
    s_quitSent = true;
    Com_Printf(16, "bo1_plan: time %d quit %d ms after complete (bo1_testclient_quitdone)\n", level.time,
        level.time - s_completeTime);
    Cbuf_AddText(0, "quit\n");
}

bool G_SP_TestPlanCommand(gentity_s *player, client_t *client, usercmd_s *cmd, gentity_s *target,
    float targetDistance, bool melee)
{
    if (!Enabled())
        return false;
    if (!s_planParsed)
        ParsePlan(player);
    playerState_s *ps = &player->client->ps;
    UpdateMeasurement(player);
    UpdatePowerups(player);
    UpdateEffects(player);
    if (level.time % 5000 == 0)
        LogBoardCounts("tick");

    // q1 c15 harness only (off by default): a god-mode soak player pinned by a crowd away from its wall runs dry and
    // the round then ends by round_spawn_failsafe churn. Apply what the retail `give ammo` cheat does (Cmd_Give_f).
    if (s_dryAmmoDvar && s_dryAmmoDvar->current.integer > 0 && !player->client->lastStand)
    {
        if (ps->weapon && TotalAmmo(ps, ps->weapon) > 0)
            s_dryStart = -1;
        else if (s_dryStart < 0 || s_dryStart > level.time)
            s_dryStart = level.time;
        else if (level.time - s_dryStart >= s_dryAmmoDvar->current.integer)
        {
            for (int slot = 0; slot < 15; ++slot)
                Add_Ammo(player, ps->heldWeapons[slot].weapon, 0, 998, 1);
            Com_Printf(16, "bo1_testclient: time %d TEST SWITCH give ammo (dry %d ms, weapon %s, origin %.0f %.0f %.0f)\n",
                level.time, level.time - s_dryStart, ps->weapon ? BG_WeaponName(ps->weapon) : "none",
                player->r.currentOrigin[0], player->r.currentOrigin[1], player->r.currentOrigin[2]);
            s_dryStart = -1;
        }
    }

    // Teleport detection (portals): the origin jumps between two commands.
    if (Vec3Distance(s_lastOrigin, player->r.currentOrigin) > 200.0f && s_lastOrigin[0] != 0.0f)
    {
        s_teleportTime = level.time;
        if (!s_isFive && fabsf(player->r.currentOrigin[0]) < 150.0f && fabsf(player->r.currentOrigin[1] + 1270.0f) < 150.0f)
            s_kinoReturnTime = level.time;
        char detail[128];
        sprintf_s(detail, "from %.0f %.0f %.0f to %.0f %.0f %.0f", s_lastOrigin[0], s_lastOrigin[1], s_lastOrigin[2],
            player->r.currentOrigin[0], player->r.currentOrigin[1], player->r.currentOrigin[2]);
        Log("teleported", detail);
        const int astroFire = LastNotifyTime("astro_headbutt_fire");
        if (astroFire >= 0 && level.time - astroFire <= 1000)
            G_SP_TestPlanObserve("astro_teleport");
    }
    Vec3Copy(player->r.currentOrigin, s_lastOrigin);

    // Diagnostics: where every step's time goes (combat, last stand, powerup detours).
    if (level.time % 10000 == 0 && s_index < (int)s_plan.size() && s_ent >= 0 && s_ent < ENTITYNUM_WORLD)
    {
        // The step target through the retail use-list checks: reach from the view origin and sight.
        gentity_s *use = &g_entities[s_ent];
        float view[3], pos[3], dir[3], forward[3];
        G_GetPlayerViewOrigin(&player->client->ps, view);
        Center(use, pos);
        BG_GetPlayerViewDirection(&player->client->ps, forward, 0, 0);
        Vec3Sub(pos, view, dir);
        const float reach = Vec3Normalize(dir);
        col_context_t sight(17);
        sight.passEntityNum0 = player->s.number;
        sight.passEntityNum1 = use->s.number;
        int hitNum = -1;
        const bool visible = SV_SightTracePoint(&hitNum, view, pos, &sight);
        Com_Printf(16, "bo1_plan: time %d status use ent %d %s reach %.1f dot %.2f lookat %d visible %d hit %d\n", level.time,
            s_ent, Str(use->classname), reach, Vec3Dot(dir, forward), use->trigger.requireLookAt ? 1 : 0, visible ? 1 : 0, hitNum);
        if (use->s.index.brushmodel && (use->r.contents & 0x40000000)) // L37: touch-trigger contact (v_trap_touch)
        {
            const cmodel_t *cmod = CM_ClipHandleToModel(use->s.index.brushmodel);
            Com_Printf(16, "bo1_plan: time %d status touch ent %d contents 0x%x linked %d sector %d leaf brush 0x%x terrain 0x%x contact %d\n",
                level.time, s_ent, use->r.contents, use->r.linked, sv.svEntities[s_ent].worldSector, cmod->leaf.brushContents,
                cmod->leaf.terrainContents, SV_EntityContact(player->r.absmin, player->r.absmax, use) ? 1 : 0);
            // L38: the geometry behind that contact test (trap volume never touching on Verruckt).
            Com_Printf(16, "bo1_plan: time %d status touchgeo org %.1f %.1f %.1f ang %.1f %.1f %.1f abs %.1f %.1f %.1f .. %.1f %.1f %.1f cmod %.1f %.1f %.1f .. %.1f %.1f %.1f leaf %.1f %.1f %.1f .. %.1f %.1f %.1f node %d aabb %d/%d player %.1f %.1f %.1f .. %.1f %.1f %.1f svflags 0x%x\n",
                level.time, use->r.currentOrigin[0], use->r.currentOrigin[1], use->r.currentOrigin[2], use->r.currentAngles[0],
                use->r.currentAngles[1], use->r.currentAngles[2], use->r.absmin[0], use->r.absmin[1], use->r.absmin[2], use->r.absmax[0],
                use->r.absmax[1], use->r.absmax[2], cmod->mins[0], cmod->mins[1], cmod->mins[2], cmod->maxs[0], cmod->maxs[1], cmod->maxs[2],
                cmod->leaf.mins[0], cmod->leaf.mins[1], cmod->leaf.mins[2], cmod->leaf.maxs[0], cmod->leaf.maxs[1], cmod->leaf.maxs[2],
                cmod->leaf.leafBrushNode, cmod->leaf.firstCollAabbIndex, cmod->leaf.collAabbCount, player->r.absmin[0], player->r.absmin[1],
                player->r.absmin[2], player->r.absmax[0], player->r.absmax[1], player->r.absmax[2], use->r.svFlags);
            // L38: every brush under the model's leaf brush node (local bounds, contents, non-axial sides).
            const cbrush_t *brushes[24];
            const int count = TriggerBrushes(use, brushes, 24);
            for (int k = 0; k < count; ++k)
                Com_Printf(16, "bo1_plan: touchbrush %d contents 0x%x sides %d bounds %.1f %.1f %.1f .. %.1f %.1f %.1f\n",
                    (int)(brushes[k] - cm.brushes), brushes[k]->contents, brushes[k]->numsides, brushes[k]->mins[0],
                    brushes[k]->mins[1], brushes[k]->mins[2], brushes[k]->maxs[0], brushes[k]->maxs[1], brushes[k]->maxs[2]);
        }
        if (hitNum > 0 && hitNum <= (int)cm.numBrushes)
        {
            const cbrush_t *brush = &cm.brushes[hitNum - 1];
            Com_Printf(16, "bo1_plan: time %d status sight brush %d contents 0x%x bounds %.1f %.1f %.1f .. %.1f %.1f %.1f target %.1f %.1f %.1f\n",
                level.time, hitNum - 1, brush->contents, brush->mins[0], brush->mins[1], brush->mins[2], brush->maxs[0],
                brush->maxs[1], brush->maxs[2], pos[0], pos[1], pos[2]);
        }
    }
    if (level.time % 10000 == 0 && s_index < (int)s_plan.size())
        Com_Printf(16, "bo1_plan: time %d status step %d %s phase %d origin %.0f %.0f %.0f laststand %d melee %d target %d dist %.0f hint %d/%d view yaw %.0f pitch %.0f\n",
            level.time, s_index, s_steps[s_plan[s_index]].name, s_phase, player->r.currentOrigin[0], player->r.currentOrigin[1],
            player->r.currentOrigin[2], player->client->lastStand ? 1 : 0, melee ? 1 : 0, target ? target->s.number : -1,
            target ? targetDistance : -1.0f, player->client->ps.cursorHint, player->client->ps.cursorHintEntIndex,
            player->client->ps.viewangles[1], player->client->ps.viewangles[0]);

    if (!player->client->lastStand && s_index >= (int)s_plan.size()
        && s_resupplyDvar->current.enabled && Resupply(player, cmd))
        return true;
    PapThiefObserve(player);
    // r1: the thief round suspends the step (see ThiefRound).
    const int thiefRound = ThiefRound(player, cmd, target, melee);
    if (thiefRound >= 0)
        return thiefRound != 0;
    const bool visualView = s_index < (int)s_plan.size()
        && (s_steps[s_plan[s_index]].kind == STEP_BOX_VIEW || s_steps[s_plan[s_index]].kind == STEP_TRAP_VIEW);
    // L20: a dry player at its wall buy presses use instead of knifing (GoUse; L141 760150).
    const bool dryWallBuy = melee && s_index < (int)s_plan.size() && s_steps[s_plan[s_index]].kind == STEP_USE
        && s_steps[s_plan[s_index]].targetname && !strcmp(s_steps[s_plan[s_index]].targetname, "weapon_upgrade")
        && !HasLoadedGun(ps);
    // L40: water_calm leads an angry George into the water instead of knifing him (w2: melee on George at 32-41
    // units held the player on the sand for the whole step; his damage only resets his engine health).
    const bool leadWater = melee && s_index < (int)s_plan.size() && !strcmp(s_steps[s_plan[s_index]].name, "water_calm")
        && LastNotifyTime("director_activated") >= 0;
    // L42: a running step's timeout and its notify evidence are checked before the fight branch and the powerup
    // chase. In No Man's Land the dogs and the endless horde keep a zombie in knife range: r5 sampled melee 1 in 57 of
    // 60 status lines, and m_nml_dogs (dog_clips notified 35 times) ran 600 s with neither its PASS nor its timeout.
    // L42g: the thrown QED's explode notify did not reach the notify hook (r15: bo1_grenade explode ent 661 at 14400,
    // no explode notify for 661 in the log); the missile's exploded flag (eFlags 0x20, set before the notify in
    // G_ExplodeMissile) or its freeing is the evidence.
    if (s_quantumMissile >= 0)
    {
        const gentity_s *qed = &g_entities[s_quantumMissile];
        if (!qed->r.inuse || (qed->s.lerp.eFlags & 0x20))
        {
            s_quantumMissile = -1;
            s_levelNotifies.push_back({ "quantum_bomb_explode", level.time });
        }
    }
    if (!player->client->lastStand && s_stepStart && s_index < (int)s_plan.size())
    {
        const PlanStep &running = s_steps[s_plan[s_index]];
        if (level.time - s_stepStart > running.timeoutMs)
        {
            EndStep(player, "FAIL", "timeout");
            return false;
        }
        if (running.notify && StepNotifyTime(running) >= 0
            && (running.kind == STEP_WAIT || running.kind == STEP_EQUIP || (running.kind == STEP_TOUCH && !strstr(running.name, "trap_touch"))))
        {
            EndStep(player, "PASS", "notify");
            return false;
        }
    }
    // L42g: a STEP_WAIT needs no walking, so it may start while the fight holds the player (r6: in the NML horde no
    // step started in 800 s; the weapon rows m_qed / m_wavegun wait there too). Its arm check is skipped.
    if (!player->client->lastStand && melee && !s_stepStart && s_index < (int)s_plan.size()
        && s_steps[s_plan[s_index]].kind == STEP_WAIT)
        StartStep(player);
    // L42g r17: the route's last step (m_nml_dogs) ends in the NML horde, where melee never stops: the plan's completion
    // and the quitdone check run before the fight branch too (r17 never printed complete).
    if (melee && !s_complete && s_index >= (int)s_plan.size())
    {
        s_complete = true;
        s_completeTime = level.time;
        Com_Printf(16, "bo1_plan: time %d complete, powerups grabbed %d\n", level.time, s_powerupsGrabbed);
    }
    if (melee && s_complete)
        G_SP_TestPlanQuitCheck();
    if (player->client->lastStand || (melee && !visualView && !dryWallBuy && !leadWater))
        return false;
    const bool riding = s_index < (int)s_plan.size() && s_steps[s_plan[s_index]].kind == STEP_FLOOR && s_phase == 3;
    // Harness: stay for a paid weapon's short pickup window instead of chasing a powerup.
    const bool taking = s_index < (int)s_plan.size() && s_phase == 1
        && (s_steps[s_plan[s_index]].kind == STEP_PAP || s_steps[s_plan[s_index]].kind == STEP_BOX);
    const bool observingTrap = s_index < (int)s_plan.size()
        && (s_steps[s_plan[s_index]].kind == STEP_WAIT || s_steps[s_plan[s_index]].kind == STEP_GLASS
            || s_steps[s_plan[s_index]].kind == STEP_CRAWLER);
    const bool placing = s_index < (int)s_plan.size() && (s_steps[s_plan[s_index]].kind == STEP_DEPLOY
        || (!strcmp(s_steps[s_plan[s_index]].name, "wall_frag") && s_phase != 0));
    if (!riding && !taking && !observingTrap && !placing && !visualView)
    {
        // Powerups the zombies drop: walk over them (the script grabs within 64 units).
        gentity_s *powerup = PowerupGoal(player);
        if (powerup)
        {
            float viewYaw = ViewYaw(player, cmd);
            if (!target)
            {
                Face(player, cmd, powerup->r.currentOrigin, false);
                viewYaw = ViewYaw(player, cmd);
            }
            Walk(player, powerup->r.currentOrigin, 2000 + powerup->s.number, cmd, viewYaw);
            return true;
        }
    }
    if (s_index >= (int)s_plan.size())
    {
        if (!s_complete)
        {
            s_complete = true;
            s_completeTime = level.time;
            Com_Printf(16, "bo1_plan: time %d complete, powerups grabbed %d\n", level.time, s_powerupsGrabbed);
        }
        G_SP_TestPlanQuitCheck();
        return false;
    }

    // r1 arm policy, between steps only. After a thief round: back to the interrupted step's floor (the steal
    // teleported the player), with the arm check (a second gun after a steal) before the ride.
    if (!s_stepStart && s_thiefReturn)
    {
        s_thiefReturn = false;
        const int floor = Floor(player->r.currentOrigin[2]);
        // q1 c9: Pack-a-Punch is only in the pack room, which only a pad reaches, at defcon 5. q19b: the thief took
        // the player out of it as pap started, the floor2 recovery rode to the war room and pap timed out. Go back
        // through portal_pack; its arm check (PackCheck) pulls the defcon switches again when the level has dropped.
        if (s_steps[s_plan[s_index]].kind == STEP_PAP && !InPackRoom(player->r.currentOrigin))
        {
            for (int i = 0; i < (int)ARRAY_COUNT(s_steps); ++i)
                if (!strcmp(s_steps[i].name, "portal_pack"))
                    InsertStep(i, 0, "the thief round took the player out of the pack room; back through a pad");
        }
        else if (s_steps[s_plan[s_index]].kind != STEP_FLOOR && s_thiefStepFloor >= 1 && s_thiefStepFloor <= 3
            && s_thiefStepFloor != floor)
        {
            // x2: one floor step per elevator ride, as the route does (floor2,floor1). b24/b27: a floor 3 -> 1 recovery in
            // one floor1 step (both cars, the thief_round call refusal, the walk) ran out of its 120 s. Inserts go in
            // front, so insert the target floor first and the floor next to the player last.
            char name[16], why[96];
            sprintf_s(why, "the thief round left the player on floor %d; the step started on floor %d", floor, s_thiefStepFloor);
            const int dir = floor < s_thiefStepFloor ? 1 : -1;
            for (int f = s_thiefStepFloor; f != floor; f -= dir)
            {
                sprintf_s(name, "floor%d", f);
                for (int i = 0; i < (int)ARRAY_COUNT(s_steps); ++i)
                    if (!strcmp(s_steps[i].name, name))
                        InsertStep(i, 0, why);
            }
        }
        s_planFlags[s_index] &= ~PLAN_ARMED;
    }
    if (!s_stepStart && !(s_planFlags[s_index] & PLAN_ARMED))
    {
        s_planFlags[s_index] |= PLAN_ARMED;
        PackCheck(player);
        ArmCheck(player);
    }
    const PlanStep &step = s_steps[s_plan[s_index]];
    if (!s_stepStart)
        StartStep(player);
    if (level.time - s_stepStart > step.timeoutMs)
    {
        EndStep(player, "FAIL", "timeout");
        return false;
    }
    {
        const int score = player->client->sess.cs.score.score;
        // Not the last stand's point penalty (player_reduce_points on the downed frame).
        if (!player->client->lastStand && s_lastScore - score > s_largestDrop)
            s_largestDrop = s_lastScore - score;
        s_lastScore = score;
    }
    // L37: after the drop is recorded. L37r4 v_trap_n: the gas trap charged 1460 -> 460 and the top-up to 6000 in
    // the same frame hid the drop (kills had raised the score 110 above the baseline, so net spent read 890).
    // L36: a step on any zombie_vending (no perk named: sumpf's huts get a random machine, perks.gsc:40-55) passes on
    // score spent, but the machine may be a 2500-3000 perk. Top up to the dearest perk's cost (4000, mule kick).
    TestSwitchScore(player, !step.perk && step.targetname && !strcmp(step.targetname, "zombie_vending") && step.cost < 4000
        ? 4000 : step.cost);
    s_lastScore = player->client->sess.cs.score.score;

    const char *why = "";
    switch (step.kind)
    {
    case STEP_DEVGUI_POWERUP:
    {
        if (!Dvar_GetBool("developer_script"))
        {
            EndStep(player, "FAIL", "retail devgui requires developer_script before map load");
            return false;
        }
        if (!strcmp(step.value, "thief_round"))
        {
            if (NotifyTime("thief_round", s_stepStart) >= 0)
            {
                EndStep(player, "PASS", "retail thief_round flag set");
                return false;
            }
        }
        else if (!strcmp(step.value, "full_ammo"))
        {
            if (s_maxAmmoTime >= s_stepStart)
            {
                EndStep(player, "PASS", "retail zmb_max_ammo notify");
                return false;
            }
        }
        else if (ZombieVar(!strcmp(step.value, "fire_sale") ? F_FIRESALE : F_BONFIRE) == 1)
        {
            EndStep(player, "PASS", "retail sale flag activated");
            return false;
        }
        cmd->forwardmove = cmd->rightmove = 0;
        cmd->button_bits.resetBit(0);
        cmd->button_bits.resetBit(11);
        Face(player, cmd, player->r.currentOrigin, true);
        // Give the usercmd view one frame before the retail bullettrace reads GetPlayerAngles.
        if (!s_phase)
        {
            s_phase = 1;
            s_phaseTime = level.time;
        }
        else if (s_phase == 1 && level.time - s_phaseTime >= 250)
        {
            Dvar_SetStringByName("zombie_devgui", const_cast<char *>(step.value));
            s_phase = 2;
            Log("devgui", step.value);
        }
        else if (s_phase == 2 && level.time % 1000 == 0)
        {
            // Harness observation only: did the retail devgui thread consume the dvar and call powerup_drop?
            Com_Printf(16, "bo1_plan: time %d devgui dvar '%s' devcheater %d devgui_power %d drop_count %d\n", level.time,
                Dvar_GetString("zombie_devgui"), LevelInt(F_DEVCHEATER, -1), LevelInt(F_DEVGUIPOWER, -1), LevelInt(F_DROPCOUNT, -1));
        }
        return true;
    }
    case STEP_BOX_VIEW:
    {
        // Observe without purchasing, or use the centre saved before a preceding
        // purchase hides the trigger below the map. Never select a disabled trigger.
        if (!s_boxViewValid)
        {
            gentity_s *box = FindTarget(step, player, true);
            if (!box)
            {
                EndStep(player, "FAIL", "no enabled box trigger");
                return false;
            }
            Center(box, s_boxViewCenter);
            s_boxViewValid = true;
            s_ent = box->s.number;
            Com_Printf(16, "bo1_plan: time %d box view ent %d center %.0f %.0f %.0f\n", level.time,
                s_ent, s_boxViewCenter[0], s_boxViewCenter[1], s_boxViewCenter[2]);
        }
        float center[3];
        Vec3Copy(s_boxViewCenter, center);
        if (!s_phase && Vec3Distance(player->r.currentOrigin, center) > 280.0f)
        {
            Face(player, cmd, center, true);
            cmd->button_bits.resetBit(0);
            cmd->button_bits.resetBit(11);
            float approach[3] = { center[0], center[1], player->r.currentOrigin[2] };
            Walk(player, approach, 3001, cmd, ViewYaw(player, cmd));
            return true;
        }
        if (!s_phase)
        {
            // Harness: a standing point the player reaches in a straight line (hull trace from here, same floor)
            // whose eye sees the chest top (the retail sight mask 17). c7-box3's fixed 220-unit radial point was
            // behind the lab3 cabinet and the path planner never reached it.
            float aim[3] = { center[0], center[1], center[2] + 30.0f };
            float best = 1.0e9f;
            int blockedWalk = 0, blockedEye = 0, noFloor = 0;
            bool found = false;
            for (int radius = 120; radius <= 260; radius += 20)
                for (int i = 0; i < 24; ++i)
                {
                    const float angle = i * (3.14159265f / 12.0f);
                    float top[3] = { center[0] + radius * cosf(angle), center[1] + radius * sinf(angle), player->r.currentOrigin[2] + 40.0f };
                    float down[3] = { top[0], top[1], top[2] - 128.0f };
                    trace_t trace;
                    col_context_t context;
                    G_TraceCapsule(&trace, top, vec3_origin, vec3_origin, down, player->s.number, player->clipmask, &context);
                    if (trace.startsolid || trace.fraction == 1.0f)
                    {
                        ++noFloor;
                        continue;
                    }
                    float ground[3] = { top[0], top[1], top[2] + trace.fraction * (down[2] - top[2]) + 1.0f };
                    if (fabsf(ground[2] - player->r.currentOrigin[2]) > 24.0f)
                    {
                        ++noFloor;
                        continue;
                    }
                    // Straight walk: the player's hull, raised by a step, from here to the point.
                    float from[3] = { player->r.currentOrigin[0], player->r.currentOrigin[1], player->r.currentOrigin[2] + 18.0f };
                    float to[3] = { ground[0], ground[1], ground[2] + 18.0f };
                    G_TraceCapsule(&trace, from, player->r.mins, player->r.maxs, to, player->s.number, player->clipmask, &context);
                    if (trace.startsolid || trace.allsolid || trace.fraction < 1.0f)
                    {
                        ++blockedWalk;
                        continue;
                    }
                    float eye[3] = { ground[0], ground[1], ground[2] + player->client->ps.viewHeightCurrent };
                    col_context_t sight(17);
                    sight.passEntityNum0 = player->s.number;
                    sight.passEntityNum1 = s_ent;
                    int hitNum = -1;
                    if (!SV_SightTracePoint(&hitNum, eye, aim, &sight))
                    {
                        ++blockedEye;
                        continue;
                    }
                    // The sight mask passes the labs' scanner apparatus (c8-box1 framed it, not the chest): also
                    // the bullet mask (0x280E833, as the fight aim) on three rays - the aim and 20 units either
                    // side - each reaching within 40 units of its point.
                    float side[2] = { -(aim[1] - eye[1]), aim[0] - eye[0] };
                    const float sideLength = sqrtf(side[0] * side[0] + side[1] * side[1]);
                    bool clear = sideLength >= 1.0f;
                    for (int ray = -1; clear && ray <= 1; ++ray)
                    {
                        float point[3] = { aim[0] + ray * 20.0f * side[0] / sideLength,
                            aim[1] + ray * 20.0f * side[1] / sideLength, aim[2] };
                        G_LocationalTrace(&trace, eye, point, player->s.number, 0x280E833, bulletPriorityMap, nullptr);
                        clear = trace.fraction >= 1.0f || (1.0f - trace.fraction) * Vec3Distance(eye, point) <= 40.0f;
                    }
                    if (!clear)
                    {
                        ++blockedEye;
                        continue;
                    }
                    // Prefer about 180 units out (the whole chest and its beam in frame), then the nearest walk.
                    const float score = fabsf((float)radius - 180.0f) * 4.0f + Vec3Distance(player->r.currentOrigin, ground);
                    if (score < best)
                    {
                        best = score;
                        Vec3Copy(ground, s_stand);
                        found = true;
                    }
                }
            char detail[192];
            if (found)
            {
                sprintf_s(detail, "stand %.0f %.0f %.0f, %.0f from box (rejected floor %d walk %d eye %d); aim at the trigger centre",
                    s_stand[0], s_stand[1], s_stand[2], Vec3Distance(s_stand, center), noFloor, blockedWalk, blockedEye);
            }
            else
            {
                float away[3] = { player->r.currentOrigin[0] - center[0], player->r.currentOrigin[1] - center[1], 0.0f };
                const float distance = Vec3Normalize(away);
                if (distance < 1.0f)
                    away[0] = 1.0f;
                for (int i = 0; i < 2; ++i)
                    s_stand[i] = center[i] + away[i] * 220.0f;
                s_stand[2] = player->r.currentOrigin[2];
                sprintf_s(detail, "no clear standing point (rejected floor %d walk %d eye %d); retreat 220 units from box",
                    noFloor, blockedWalk, blockedEye);
            }
            s_phase = 1;
            s_boxViewStraight = found;
            Log("view", detail);
        }
        // Aim at the trigger centre: c8-box2 at 30 above it (143 units out) left the chest at the frame's bottom edge.
        Face(player, cmd, center, true);
        cmd->forwardmove = cmd->rightmove = 0;
        cmd->button_bits.resetBit(0);
        cmd->button_bits.resetBit(11);
        if (Vec3Distance(player->r.currentOrigin, s_stand) > 24.0f)
        {
            if (s_boxViewStraight)
            {
                // Straight line: the hull trace above cleared it, so no path planning.
                float dir[2] = { s_stand[0] - player->r.currentOrigin[0], s_stand[1] - player->r.currentOrigin[1] };
                const float length = sqrtf(dir[0] * dir[0] + dir[1] * dir[1]);
                const float yaw = ViewYaw(player, cmd) * (3.14159265f / 180.0f);
                if (length >= 1.0f)
                {
                    const float scale = length > 64.0f ? 1.0f : 0.5f;
                    const float forward = (dir[0] * cosf(yaw) + dir[1] * sinf(yaw)) / length * scale;
                    const float right = (dir[0] * sinf(yaw) - dir[1] * cosf(yaw)) / length * scale;
                    cmd->forwardmove = (char)(int)(127.0f * forward);
                    cmd->rightmove = (char)(int)(127.0f * right);
                }
            }
            else
                Walk(player, s_stand, 3001, cmd, ViewYaw(player, cmd));
        }
        else if (s_phase == 1)
        {
            s_phase = 2;
            s_phaseTime = level.time;
            Log("view", "holding box beam view for 15 seconds");
        }
        if (s_phase == 2 && level.time - s_phaseTime >= 15000)
            EndStep(player, "PASS", "box beam observation interval elapsed");
        return true;
    }
    case STEP_GLASS:
    {
        const unsigned int id = (unsigned int)Dvar_GetInt("bo1_testclient_glass");
        if (id >= svGlasses.numGlasses)
        {
            EndStep(player, "FAIL", "glass index out of range");
            return true;
        }
        const GlassServer &pane = svGlasses.glasses[id];
        float aim[3];
        for (int axis = 0; axis < 3; ++axis)
            aim[axis] = (pane.glass->absmin[axis] + pane.glass->absmax[axis]) * 0.5f;
        Face(player, cmd, aim, true);
        cmd->forwardmove = cmd->rightmove = 0;
        cmd->button_bits.resetBit(0);
        cmd->button_bits.resetBit(11);
        const int elapsed = level.time - s_stepStart;
        const int shots = s_shots - s_stepShots;
        if (elapsed >= 20000)
        {
            Com_Printf(16, "bo1_glass: plan id %u shots %d state %u health %.1f\n",
                id, shots, pane.state.val.i & 0xF, pane.health);
            EndStep(player, shots > 1 && (pane.state.val.i & 0xF) == GlassState::SHATTERED
                ? "PASS" : "FAIL", "glass shots and state");
            return true;
        }
        // Five seconds intact, five seconds after one real weapon shot, then follow-up shots.
        if (elapsed >= 5000 && (!shots || (elapsed >= 10000 && elapsed % 1000 < 100))
            && !ps->weaponTime && !client->lastUsercmd.button_bits.testBit(0))
            cmd->button_bits.setBit(0);
        if (level.time % 1000 == 0)
            Com_Printf(16, "bo1_glass: plan time %d elapsed %d id %u shots %d state %u health %.1f aim %.1f %.1f %.1f\n",
                level.time, elapsed, id, shots, pane.state.val.i & 0xF, pane.health, aim[0], aim[1], aim[2]);
        return true;
    }
    case STEP_TRAP_VIEW:
    case STEP_WAIT:
    {
        // L41: a wait with a level notify is a ride (Shangri-La's minecart, level notify minecart_end): no trap volume;
        // hold the ride, PASS on the notify (a rider's minecart_exit) (the step timeout FAILs it).
        if (step.kind == STEP_WAIT && step.notify)
        {
            if (StepNotifyTime(step) >= 0)
            {
                EndStep(player, "PASS", "notify");
                return false;
            }
            return true;
        }
        if (!s_phase)
        {
            // q1 c9: the trap is the one nearest the last trap switch the route used (on this floor), else the one
            // nearest the player. a11m: a thief round took the player to floor 3, the recovery ride came back by
            // the other elevator and the wait watched the elevator trap at -628 2040 instead of the QR trap.
            float ref[3];
            Vec3Copy(player->r.currentOrigin, ref);
            bool fromSwitch = false;
            if (step.kind == STEP_WAIT && s_lastTrapUse >= 0 && g_entities[s_lastTrapUse].r.inuse)
            {
                float center[3];
                Center(&g_entities[s_lastTrapUse], center);
                if (Floor(center[2]) == Floor(player->r.currentOrigin[2]))
                {
                    ref[0] = center[0];
                    ref[1] = center[1];
                    fromSwitch = true;
                }
            }
            gentity_s *trap = nullptr;
            float nearest = 1.0e9f;
            for (int i = level.maxclients; i < level.num_entities; ++i)
            {
                gentity_s *ent = &g_entities[i];
                if (!ent->r.inuse || !IsTrigger(ent) || strcmp(Str(ent->targetname), "zombie_trap"))
                    continue;
                float center[3];
                Center(ent, center);
                const float dx = center[0] - ref[0], dy = center[1] - ref[1];
                if (dx * dx + dy * dy < nearest)
                {
                    nearest = dx * dx + dy * dy;
                    trap = ent;
                }
            }
            if (!trap)
            {
                EndStep(player, "FAIL", "no trap volume");
                return false;
            }
            Vec3Copy(player->r.currentOrigin, s_stand);
            s_waitTrap = trap->s.number;
            if (step.kind == STEP_TRAP_VIEW)
                s_ent = trap->s.number;
            // Lure across the thin dimension, on the side opposite the nearest live zombie.
            const int axis = trap->r.absmax[0] - trap->r.absmin[0] < trap->r.absmax[1] - trap->r.absmin[1] ? 0 : 1;
            const float middle = (trap->r.absmin[axis] + trap->r.absmax[axis]) * 0.5f;
            if (fromSwitch)
            {
                // Stand across from the switch, within the volume's long side (the player may be far from it).
                s_stand[axis] = ref[axis];
                s_stand[1 - axis] = ref[1 - axis] < trap->r.absmin[1 - axis] ? trap->r.absmin[1 - axis]
                    : ref[1 - axis] > trap->r.absmax[1 - axis] ? trap->r.absmax[1 - axis] : ref[1 - axis];
            }
            gentity_s *zombie = nullptr;
            float distance = 1.0e9f;
            for (int i = level.maxclients; i < level.num_entities; ++i)
                if (g_entities[i].r.inuse && g_entities[i].actor && g_entities[i].health > 0
                    && fabsf(g_entities[i].r.currentOrigin[2] - s_stand[2]) < 100.0f)
                {
                    const float d = Vec3DistanceSq(g_entities[i].r.currentOrigin, s_stand);
                    if (d < distance) { distance = d; zombie = &g_entities[i]; }
                }
            const bool side = (zombie ? zombie->r.currentOrigin[axis] : s_stand[axis]) < middle;
            if (step.kind == STEP_TRAP_VIEW)
            {
                // Keep the camera on its current side, centred across the doorway.
                s_stand[axis] = s_stand[axis] < middle ? trap->r.absmin[axis] - 180.0f : trap->r.absmax[axis] + 180.0f;
                s_stand[1 - axis] = (trap->r.absmin[1 - axis] + trap->r.absmax[1 - axis]) * 0.5f;
            }
            else
                s_stand[axis] = side ? trap->r.absmax[axis] + 160.0f : trap->r.absmin[axis] - 160.0f;
            // L20: not across a trap the route has just bought (trap_activate_electric runs 40 s): stay on the player's
            // side of it. v3-1 556150: trap_qr paid at 556100, the nearest zombie was on the player's side, so the
            // stand was the far side; the walk went through the volume (-801 .. -761) and the trap (mod 15, 16 a
            // tick) downed the player at 558600 - a game over.
            if (step.kind == STEP_WAIT && (TriggerEnabled(trap)
                || (s_lastTrapUseTime >= 0 && level.time - s_lastTrapUseTime < 45000)))
                s_stand[axis] = player->r.currentOrigin[axis] < middle ? trap->r.absmin[axis] - 160.0f
                    : trap->r.absmax[axis] + 160.0f;
            s_phase = 1;
            Com_Printf(16, "bo1_plan: time %d trap observation ent %d bounds %.0f %.0f .. %.0f %.0f retreat %.0f %.0f %.0f\n",
                level.time, trap->s.number, trap->r.absmin[0], trap->r.absmin[1], trap->r.absmax[0], trap->r.absmax[1],
                s_stand[0], s_stand[1], s_stand[2]);
        }
        if (level.time - s_stepStart >= step.count)
        {
            EndStep(player, "PASS", "observation interval elapsed");
            return false;
        }
        // q1 c9: every gun nearly dry during a wait: buy ammo (or this floor's wall gun) and restart the wait, as the
        // floor requeue does; a player knifing round-8 zombies for the rest of the wait goes down (a11m, q18a).
        // L40 (b451): up to 4 restarts (was 2): two thief rounds in one wait110 used both, and the last wait (ak74u
        // 107 rounds at 1236000) ran dry at 1268000 with no resupply left; FAIL at 1439000 knifing.
        if (step.kind == STEP_WAIT && s_waitResupplies < 4 && !player->client->lastStand)
        {
            unsigned int guns[15];
            const int count = PrimaryGuns(ps, guns);
            int ammoStep = -1;
            unsigned int emptiest = 0;
            float best = 1.0f, low = 1.0f;
            // r1 c4: an explosive projectile gun is no fighting gun at wait range (g_sp_client never fires it inside
            // its blast and its dry-gun switch raises only bullet guns). r4a: crossbow_explosive_zm (loaded) + a dry
            // pm63 at the QR wait spot: best stayed over 15%, no resupply, the player knifed zombies and went down.
            // Such a gun counts only when every held gun is one.
            int fighting = 0;
            for (int i = 0; i < count; ++i)
            {
                const WeaponDef *def = BG_GetWeaponDef(guns[i]);
                if (!(def->weapType == WEAPTYPE_PROJECTILE && def->iExplosionRadius > 0))
                    ++fighting;
            }
            bool first = true;
            for (int i = 0; i < count; ++i)
            {
                const float fraction = AmmoFraction(ps, guns[i]);
                if (fraction < low)
                {
                    low = fraction;
                    emptiest = guns[i];
                }
                const WeaponDef *def = BG_GetWeaponDef(guns[i]);
                if (!fighting || !(def->weapType == WEAPTYPE_PROJECTILE && def->iExplosionRadius > 0))
                {
                    best = first ? fraction : (fraction > best ? fraction : best);
                    first = false;
                }
                const int refill = WallStepIndex(guns[i], true);
                if (ammoStep < 0 && refill >= 0 && TriggerOnFloor(refill, player))
                    ammoStep = refill;
            }
            // L20: 30% (was 15%): the resupply walk needs ammunition too. v4-2 855050: wait110 on floor 3 requeued at
            // m14 16 rounds (14%) for the ak74u wall 1100 units away; dry at 859300 (8-round clip, round 9), knifed
            // on the way and downed at 864550, then the game over.
            // L40 (b451, b38): 50% (was 30%). 899950: requeued at m14 30%, dry on the walk, knifed at the m14 wall
            // for 60 s without a use press (ammo_m14 FAIL at 960050); the walk must start with enough to fight.
            if (!count || best < 0.5f)
            {
                const int insert = ammoStep >= 0 ? ammoStep : FloorWallGun(player);
                if (insert >= 0)
                {
                    char why[160];
                    sprintf_s(why, "every gun under 50%% of full (%s, best %.0f%%); resupply, then restart the wait",
                        WeaponList(ps).c_str(), best * 100.0f);
                    Log("requeue", why);
                    ++s_waitResupplies;
                    s_stepStart = 0;
                    s_phase = 0;
                    InsertStep(insert, ammoStep >= 0 || count < 2 ? 0 : emptiest, why);
                    return false;
                }
            }
        }
        if (step.kind == STEP_TRAP_VIEW)
        {
            float center[3];
            Center(&g_entities[s_ent], center);
            Face(player, cmd, center, true);
            cmd->forwardmove = cmd->rightmove = 0;
            cmd->button_bits.resetBit(0);
            cmd->button_bits.resetBit(11);
        }
        if (Vec3Distance(player->r.currentOrigin, s_stand) > 24.0f)
            Walk(player, s_stand, 3000, cmd, ViewYaw(player, cmd));
        if (level.time % 5000 == 0)
            Com_Printf(16, "bo1_plan: time %d trap lure player %.0f %.0f %.0f target %d at %.0f %.0f %.0f distance %.0f\n",
                level.time, player->r.currentOrigin[0], player->r.currentOrigin[1], player->r.currentOrigin[2],
                target ? target->s.number : -1, target ? target->r.currentOrigin[0] : 0.0f,
                target ? target->r.currentOrigin[1] : 0.0f, target ? target->r.currentOrigin[2] : 0.0f, targetDistance);
        // Let distant zombies approach the trap, defending only against those already nearby.
        // r1 c6: only while the trap is live. maps\_zombiemode_traps::trap_use_think trigger_on's the trap volume
        // for the activation and trigger_off's it at "trap_done" (40 s, trap_activate_electric); after that the wait
        // is a plain hold of the spot. r5c / r6c: round 6 at the QR spot with the wall m14 (3 hits a kill, 8-round
        // clip), zombies were let in to 100 units for the whole 110 s; at 544550 (r6c) it reloaded at 70 units and
        // four hits of 60 later it went down.
        const bool trapLive = s_waitTrap >= 0 && g_entities[s_waitTrap].r.inuse && TriggerEnabled(&g_entities[s_waitTrap]);
        if (level.time % 5000 == 0)
            Com_Printf(16, "bo1_plan: time %d trap ent %d live %d\n", level.time, s_waitTrap, trapLive ? 1 : 0);
        if (targetDistance > 100.0f && (step.kind == STEP_TRAP_VIEW || trapLive))
            cmd->button_bits.resetBit(0);
        return true;
    }
    case STEP_CRAWLER:
    {
        // Harness only. Brief suggested leg shots for live crawlers; retail
        // maps/_zombiemode_spawner::zombie_gib_on_damage only selects bullet leg
        // refs at health <= 0. Explosives use derive_damage_refs even while alive.
        // Never set health, has_legs, gib refs, or damage: use the offhand command.
        // zombie_damage adds another 100..199 + round damage after a grenade hit;
        // wait for round five (550 HP) so even a close blast can survive both callbacks.
        if (!s_phase && LevelInt(F_ROUND, 0) < 5)
            return false;
        const auto fightBits = cmd->button_bits;
        cmd->button_bits.resetBit(0);
        cmd->button_bits.resetBit(2);
        cmd->button_bits.resetBit(11);
        const unsigned int weapon = BG_FindWeaponIndexForName(step.weapon);
        for (int i = level.maxclients; i < level.num_entities; ++i)
        {
            int legs = 1;
            gentity_s *ent = &g_entities[i];
            if (ent->r.inuse && ent->actor && ent->health > 0
                && EntField(i, F_HASLEGS, nullptr, &legs) && !legs)
            {
                // q1 c15: a crawler already observed for a whole step is left to the fight harness.
                if (std::find(s_crawlersSeen.begin(), s_crawlersSeen.end(), i) != s_crawlersSeen.end())
                    continue;
                float point[3];
                Vec3Copy(ent->r.currentOrigin, point);
                point[2] += 16.0f;
                Face(player, cmd, point, true);
                if (s_phase != 3)
                {
                    s_phase = 3;
                    s_phaseTime = level.time;
                    Com_Printf(16, "bo1_crawler: time %d LIVE ent %d health %d origin %.1f %.1f %.1f\n",
                        level.time, i, ent->health, point[0], point[1], ent->r.currentOrigin[2]);
                }
                const float distance = Vec3Distance(player->r.currentOrigin, ent->r.currentOrigin);
                if (distance > 140.0f)
                    Walk(player, ent->r.currentOrigin, 4000 + i, cmd, ViewYaw(player, cmd));
                else if (distance < 100.0f)
                    cmd->forwardmove = -80;
                if (level.time - s_phaseTime >= 20000)
                {
                    s_crawlersSeen.push_back(i);
                    EndStep(player, "PASS", "living actor has_legs=0 observed for 20 seconds");
                }
                return true;
            }
        }
        // q1 c15: forget observed crawlers once their entity is dead or has legs again (a new life).
        for (size_t k = 0; k < s_crawlersSeen.size();)
        {
            int legs = 1;
            const gentity_s *ent = &g_entities[s_crawlersSeen[k]];
            if (!ent->r.inuse || !ent->actor || ent->health <= 0 || !EntField(s_crawlersSeen[k], F_HASLEGS, nullptr, &legs) || legs)
                s_crawlersSeen.erase(s_crawlersSeen.begin() + k);
            else
                ++k;
        }
        if (!s_phase)
        {
            // q1 c15: without a frag the fight harness plays on (rounds end, the scripts hand out frags again).
            if (!weapon || !BG_PlayerHasWeapon(ps, weapon) || !TotalAmmo(ps, weapon))
            {
                cmd->button_bits = fightBits;
                return false;
            }
            if (!target)
                return true;
            if (targetDistance < 380.0f)
            {
                cmd->forwardmove = -100;
                return true;
            }
            if (targetDistance > 400.0f)
                return true;
            s_startAmmo = TotalAmmo(ps, weapon);
            s_phase = 1;
            s_phaseTime = level.time;
            Com_Printf(16, "bo1_crawler: time %d throw target %d distance %.1f health %d ammo %d\n",
                level.time, target->s.number, targetDistance, target->health, s_startAmmo);
        }
        if (s_phase == 1)
        {
            cmd->angles[0] = (unsigned short)(int)((85.0f - ps->delta_angles[0]) * 182.04445f);
            if (level.time - s_phaseTime < 300)
                cmd->button_bits.setBit(14);
            if (TotalAmmo(ps, weapon) < s_startAmmo && level.time - s_phaseTime >= 4000)
            {
                s_phase = 2;
                s_phaseTime = level.time;
                Log("thrown", "frag consumed; awaiting live crawler, no gunfire");
            }
        }
        else if (s_phase == 2 && level.time - s_phaseTime > 10000)
        {
            EndStep(player, "FAIL", "frag consumed but no living crawler observed");
        }
        return true;
    }
    case STEP_GRENADE:
    case STEP_DEPLOY:
    {
        const unsigned int weapon = BG_FindWeaponIndexForName(step.weapon);
        cmd->button_bits.resetBit(0);
        cmd->button_bits.resetBit(11);
        if (!s_phase)
        {
            s_startAmmo = TotalAmmo(ps, weapon);
            if (!weapon || !BG_PlayerHasWeapon(ps, weapon) || !s_startAmmo)
            {
                EndStep(player, "FAIL", "weapon or ammo missing");
                return true;
            }
            s_phase = 1;
        }
        if (step.kind == STEP_GRENADE)
        {
            const int holdMs = step.count ? step.count : 300;
            if (step.value && !strcmp(step.value, "feet"))
                cmd->angles[0] = (unsigned short)(int)((85.0f - ps->delta_angles[0]) * 182.04445f);
            if (level.time - s_stepStart < holdMs)
                cmd->button_bits.setBit(14);
            if (step.count)
                cmd->button_bits.resetBit(2); // no knifing: a melee ends the held (cooking) grenade
            if (step.count && (level.time - s_stepStart) % 500 == 0)
                Com_Printf(16, "bo1_plan: time %d cook weaponstate %d grenadeTimeLeft %d offhand %d cookOffHold %d holdToThrow %d pm_flags 0x%x old14 %d cmd14 %d weaponTime %d delay %d\n",
                    level.time, ps->weaponstate, ps->grenadeTimeLeft, ps->offHandIndex,
                    BG_GetWeaponDef(weapon)->bCookOffHold, BG_GetWeaponDef(weapon)->holdButtonToThrow, ps->pm_flags,
                    (int)client->lastUsercmd.button_bits.testBit(14), (int)cmd->button_bits.testBit(14), ps->weaponTime, ps->weaponDelay);
            if (TotalAmmo(ps, weapon) < s_startAmmo && level.time - s_stepStart > holdMs + 1200)
                EndStep(player, "PASS", "frag ammo consumed; see grenade_fire");
            return true;
        }
        if (s_phase == 1)
        {
            botInfos[client - svs.clients].weapon = weapon;
            cmd->weapon = weapon;
            if (target && targetDistance < 500.0f && ps->weapon == weapon && ps->weaponstate == WEAPON_READY)
            {
                if (target) Face(player, cmd, target->r.currentOrigin, false);
                cmd->button_bits.setBit(0);
            }
            if (TotalAmmo(ps, weapon) < s_startAmmo)
            {
                const float yaw = ps->viewangles[1] * (3.14159265f / 180.0f);
                Vec3Copy(player->r.currentOrigin, s_stand);
                s_stand[0] -= 160.0f * cosf(yaw);
                s_stand[1] -= 160.0f * sinf(yaw);
                s_phase = 2;
                s_phaseTime = level.time;
                Log("placed", "claymore ammo consumed; see grenade_fire and actor damage");
                for (int i = 0; i < 15; ++i)
                    if (ps->heldWeapons[i].weapon && BG_GetWeaponDef(ps->heldWeapons[i].weapon)->weapType == WEAPTYPE_BULLET)
                    {
                        botInfos[client - svs.clients].weapon = ps->heldWeapons[i].weapon;
                        cmd->weapon = ps->heldWeapons[i].weapon;
                        break;
                    }
            }
        }
        else
        {
            if (Vec3Distance(player->r.currentOrigin, s_stand) > 24.0f)
                Walk(player, s_stand, 3001, cmd, ViewYaw(player, cmd));
            // Pass only on a kill whose death notify names the claymore; the harness does not shoot meanwhile.
            if (s_claymoreKillTime > s_phaseTime)
                EndStep(player, "PASS", "actor killed by claymore_zm (death notify weapon)");
            else if (level.time - s_stepStart > 85000)
                EndStep(player, "FAIL", "placed; no claymore kill observed");
        }
        return true;
    }
    case STEP_PAP_VIEW:
    {
        // w1 c11: hold the upgraded gun facing the lit Pack-a-Punch machine; a hip and an ADS screenshot
        // (bo1_testclient_shots) show the camo'd view model (weapon options from weaponoptions.csv, SP 0x004F4B30).
        unsigned int gun = 0;
        for (int i = 0; i < 15 && !gun; ++i)
            if (ps->heldWeapons[i].weapon && strstr(BG_WeaponName(ps->heldWeapons[i].weapon), "upgraded"))
                gun = ps->heldWeapons[i].weapon;
        gentity_s *ent = s_ent >= 0 ? &g_entities[s_ent] : FindTarget(step, player, false);
        if (!gun || !ent)
        {
            EndStep(player, "FAIL", !gun ? "no upgraded gun held" : "no Pack-a-Punch trigger");
            return false;
        }
        s_ent = ent->s.number;
        float center[3];
        Center(ent, center);
        cmd->forwardmove = cmd->rightmove = 0;
        cmd->button_bits.resetBit(0);
        cmd->button_bits.resetBit(3);
        Face(player, cmd, center, true);
        if (!HoldWeapon(player, client, cmd, gun))
            return true;
        if (s_phase == 0)
        {
            s_phase = 1;
            s_phaseTime = level.time;
        }
        else if (s_phase == 1 && level.time - s_phaseTime >= 1500)
        {
            Shot("papview_hip");
            s_phase = 2;
            s_phaseTime = level.time;
        }
        else if (s_phase == 2 && level.time - s_phaseTime >= 1500)
        {
            Shot("papview_ads");
            s_phase = 3;
            s_phaseTime = level.time;
        }
        else if (s_phase == 3 && level.time - s_phaseTime >= 500)
        {
            EndStep(player, "PASS", "upgraded gun viewed");
            return false;
        }
        if (s_phase >= 2)
            cmd->button_bits.setBit(11); // ADS
        return true;
    }
    case STEP_EQUIP:
    {
        const unsigned int weapon = BG_FindWeaponIndexForName(step.weapon);
        if (!weapon || !BG_PlayerHasWeapon(ps, weapon))
        {
            EndStep(player, "FAIL", "weapon not held");
            return false;
        }
        // L42: equipment (the P.E.S.): raising it is the activation; the script lowers it again (gasmask.gsc:120-230),
        // so the evidence is the step's notify, not the raised weapon.
        if (step.notify)
        {
            if (StepNotifyTime(step) >= 0)
            {
                EndStep(player, "PASS", "notify");
                return false;
            }
        }
        else if (ps->weapon == weapon && ps->weaponstate == WEAPON_READY)
        {
            EndStep(player, "PASS", "selected held weapon");
            return false;
        }
        botInfos[client - svs.clients].weapon = weapon;
        cmd->weapon = weapon;
        return true;
    }
    case STEP_JUMP:
    {
        // L42 measurement: an ordinary jump usercmd (button bit 10) from standing; the apex and air time show the
        // gravity Pmove used (Earth 800 vs the moon's setplayergravity 136, maps_zombie_moon_gravity.gsc:523).
        static int jumpStep = -1, jumpsDone, jumpStartTime;
        static float jumpZ, jumpApex;
        if (jumpStep != s_stepStart)
        {
            jumpStep = s_stepStart;
            jumpsDone = 0;
            s_phase = 0;
            s_phaseTime = level.time;
        }
        cmd->forwardmove = cmd->rightmove = 0;
        cmd->button_bits.resetBit(0);
        cmd->button_bits.resetBit(10);
        const bool onGround = ps->groundEntityNum != ENTITYNUM_NONE;
        const float z = player->r.currentOrigin[2];
        if (s_phase == 0)
        {
            if (!onGround || level.time - s_phaseTime < 500)
            {
                if (!onGround)
                    s_phaseTime = level.time;
                return true;
            }
            jumpZ = jumpApex = z;
            jumpStartTime = level.time;
            s_phase = 1;
        }
        if (z > jumpApex)
            jumpApex = z;
        if (s_phase == 1)
        {
            cmd->button_bits.setBit(10);
            if (!onGround)
                s_phase = 2;
            else if (level.time - jumpStartTime > 1000)
            {
                s_phase = 0; // controls frozen (the level intro): settle and press again until the step times out
                s_phaseTime = level.time;
            }
            return true;
        }
        if (!onGround)
            return true;
        Com_Printf(16, "bo1_plan: time %d jump %d apex %.2f air %d ms gravity %d at %.0f %.0f %.0f\n", level.time,
            jumpsDone, jumpApex - jumpZ, level.time - jumpStartTime, ps->gravity, player->r.currentOrigin[0],
            player->r.currentOrigin[1], jumpZ);
        if (++jumpsDone >= step.count)
        {
            EndStep(player, "PASS", "jumps logged");
            return false;
        }
        s_phase = 0;
        s_phaseTime = level.time;
        return true;
    }
    case STEP_FIRE:
    {
        if (s_shots - s_stepShots >= step.count)
        {
            EndStep(player, "PASS", "shots");
            return false;
        }
        if (target)
            return false; // the fighting policy fires at the target
        const int clip = ps->weapon ? BG_GetAmmoInClip(ps, ps->weapon) : 0;
        if (!clip)
            return false; // the fighting policy reloads
        const WeaponDef *weapon = BG_GetWeaponDef(ps->weapon);
        if (!ps->weaponTime && (!weapon->fireType || !client->lastUsercmd.button_bits.testBit(0)))
            cmd->button_bits.setBit(0);
        return true;
    }
    case STEP_USE:
    {
        const bool defcon = !strcmp(step.name, "defcon");
        if ((strcmp(step.name, "wall_frag") || s_phase == 2) && CheckUseSuccess(player, step, &why))
        {
            EndStep(player, "PASS", why);
            return false;
        }
        if (defcon)
        {
            const int current = LevelInt(F_DEFCON, 1);
            if (current != s_phase)
            {
                char detail[128];
                sprintf_s(detail, "level %d -> %d; waiting for an armed switch, prompt %d", s_phase, current,
                    s_ent >= 0 ? g_entities[s_ent].s.un1.scale : -1);
                Log("defcon", detail);
                s_phase = current;
                s_defconUseTime = 0;
                s_ent = -1;
            }
            // Bonfire Sale pulls all four switches. Let its ordinary 30-second
            // expiry/reset finish, then measure an actual pull from the new level.
            // Previously the captured level stayed at 5 and could never increase.
            if (current >= 5)
            {
                // r1 c5 (r4c): defcon 5 without a Bonfire Sale is the goal of the defcon steps, whichever pull
                // got there (a reload through another switch's trigger). portal_pack's arm check (PackCheck)
                // still re-inserts pulls if the level drops before the pads.
                if (ZombieVar(F_BONFIRE) != 1)
                    EndStep(player, "PASS", "defcon 5 reached");
                return false;
            }
        }
        gentity_s *ent = s_ent >= 0 && g_entities[s_ent].r.inuse && TriggerEnabled(&g_entities[s_ent])
            && (!defcon || DefconSwitchReady(&g_entities[s_ent]))
            ? &g_entities[s_ent] : FindTarget(step, player, true);
        // L42 path rule: a door with a trigger on each side (moon airlock catacombs_west: y 547 and 741, closed doors
        // at 547 and 740). When the use prompt of another trigger of this step shows (the use list's cursor hint), that
        // one is on the player's side. r7: the walk to the far one (ent 491) stalled 90 s touching the near one (hint 490).
        if (ent && !defcon && ps->cursorHint && ps->cursorHintEntIndex != ent->s.number
            && ps->cursorHintEntIndex >= level.maxclients && ps->cursorHintEntIndex < level.num_entities)
        {
            gentity_s *hinted = &g_entities[ps->cursorHintEntIndex];
            if (hinted->r.inuse && IsPlanTarget(hinted, step) && Matches(hinted, step) && TriggerEnabled(hinted)
                && !(step.unique && Used(hinted->s.number)))
                ent = hinted;
        }
        if (!ent)
        {
            if (defcon)
                return false; // still waiting for pack_room_reset; retain the step timeout
            // r1 c5 (r4c): the step's door was bought earlier. The testclient's reload is usercmd bit 5
            // (usereload); Player_UpdateActivate (SP 0x00559690, KB player_use_mp.cpp) runs Player_ActivateCmd for
            // it first, so a reload inside a zombie_door use trigger buys that door, as it would for a player.
            // r4c: the power walk reloaded at -846 3639 inside lab3's trigger (71100, -1250); door_opened then
            // trigger_off'd it and the lab3 step found no trigger. The flag the step proves is already set.
            if (step.notify && LevelFlag(step.notify) == 1)
            {
                char detail[96];
                sprintf_s(detail, "flag %s already set (notify at %d)", step.notify, LastNotifyTime(step.notify));
                EndStep(player, "PASS", detail);
                return false;
            }
            // L28: k_tp_back waits for the script's own return (30 s in the projection room); the pad may be off.
            // L36: sumpf's zipline lever deletes its trigger after the lever animation (zipline.gsc:112-113); the step
            // then waits for the ride's level notify machine_off (zipline.gsc:642) until its timeout.
            if (level.time - s_stepStart > 5000 && !(!strncmp(step.name, "k_tp_", 5) && step.count == 2)
                && !(!strncmp(step.name, "s_zip_", 6) && s_ent >= 0)
                && !(!strcmp(step.name, "c_launch") && s_ent >= 0) // L39: trig_launch_rocket deletes itself on use (pack_a_punch.gsc:176)
                // L41: Shangri-La's power lever trigger goes away on use; power_on follows the lever animation
                // (s4a: use 68050, no trigger at 69250, power_on 69650). Wait for the notify until the timeout.
                && !(!strncmp(step.name, "t_power_", 8) && s_ent >= 0))
                EndStep(player, "FAIL", "no enabled trigger");
            return false;
        }
        if (ent->s.number != s_ent)
        {
            s_ent = ent->s.number;
            float center[3];
            Center(ent, center);
            char detail[256];
            sprintf_s(detail, "ent %d %s %s at %.0f %.0f %.0f bounds %.0f %.0f %.0f .. %.0f %.0f %.0f", s_ent,
                Str(ent->classname), Str(ent->targetname), center[0], center[1], center[2], ent->r.absmin[0],
                ent->r.absmin[1], ent->r.absmin[2], ent->r.absmax[0], ent->r.absmax[1], ent->r.absmax[2]);
            Log("target", detail);
        }
        // q1 c8: the step's trigger is on another floor (FindTarget prefers this floor, so there is none here). Five's
        // floors connect only by elevator or pad (no path links between them), so walking cannot reach it. A pad
        // touched on the way does this: during the 30 s defcon countdown (start_defcon_countdown) the script's
        // cooldown_portal_timer ends at once and every pad sends to the pack room, so walking off an arrival point
        // back over its pad teleports again (retail script behaviour); a thief round does it too. Ride to the
        // target's floor first (a recovery step), then restart this step.
        {
            float center[3];
            Center(ent, center);
            const int want = Floor(center[2]);
            const int floor = Floor(player->r.currentOrigin[2]);
            if (!defcon && want != floor && s_floorRequeues < 3)
            {
                char name[16], why[128];
                sprintf_s(name, "floor%d", want);
                sprintf_s(why, "the player is on floor %d, ent %d is on floor %d%s; restart the step after the ride", floor,
                    ent->s.number, want, s_teleportTime > s_stepStart ? " (teleported during the step)" : "");
                Log("requeue", why);
                for (int i = 0; i < (int)ARRAY_COUNT(s_steps); ++i)
                    if (!strcmp(s_steps[i].name, name))
                    {
                        ++s_floorRequeues;
                        s_stepStart = 0;
                        InsertStep(i, 0, why);
                        return false;
                    }
            }
        }
        // r1 arm policy: a wall buy replacing a dry gun raises that gun at the wall (the script takes the held one).
        const unsigned int replace = s_planReplace[s_index];
        unsigned int guns[15];
        if (replace && BG_PlayerHasWeapon(ps, replace) && PrimaryGuns(ps, guns) >= 2)
        {
            float center[3];
            Center(ent, center);
            // r1 c3: not while a zombie is in fight range. r3g raised the dry crossbow at the m14 wall, zombies came,
            // the held gun kept the empty-gun fallback off and the player knifed until downed. Drop the hold: the
            // fighting policy raises a loaded gun, and the dry one is raised again when the area is clear.
            // L20: unless no bullet gun has ammo (L141 760150: dry ak74u + china_lake at the m14 wall): then the buy
            // with the dry gun raised is the only way to fight.
            if (target && targetDistance < 256.0f && HasLoadedGun(ps))
            {
                s_holdWeapon = 0;
                GoUse(player, cmd, ent, target, targetDistance, false);
                return true;
            }
            if (Vec3Distance(center, player->r.currentOrigin) < 160.0f && !HoldWeapon(player, client, cmd, replace))
            {
                GoUse(player, cmd, ent, target, targetDistance, false);
                return true;
            }
            if (ps->weapon != replace)
            {
                GoUse(player, cmd, ent, target, targetDistance, false); // never buy with the wrong gun raised
                return true;
            }
        }
        // Harness: do not spend the QR trap's active window waiting for a remote
        // zombie to arrive. Walk to its real switch, then buy when an actor is near.
        if (!strcmp(step.name, "trap_qr"))
        {
            gentity_s *trap = nullptr;
            // The damage volume nearest this switch is the one we are arming.
            float distance = 1.0e9f;
            float switchCenter[3];
            Center(ent, switchCenter);
            for (int i = level.maxclients; i < level.num_entities; ++i)
                if (g_entities[i].r.inuse && !strcmp(Str(g_entities[i].targetname), "zombie_trap"))
                {
                    float volumeCenter[3];
                    Center(&g_entities[i], volumeCenter);
                    const float d = Vec3DistanceSq(volumeCenter, switchCenter);
                    if (d < distance) { distance = d; trap = &g_entities[i]; }
                }
            bool nearby = false;
            float center[3];
            if (trap)
            {
                Center(trap, center);
                for (int i = level.maxclients; i < level.num_entities; ++i)
                    if (g_entities[i].r.inuse && g_entities[i].actor && g_entities[i].health > 0
                        && Vec3Distance(g_entities[i].r.currentOrigin, center) < 200.0f)
                        nearby = true;
            }
            // L20: 25 s without one, buy anyway (the step's evidence is the purchase: 1000 spent). base2 379400: the
            // round's last zombie was a crawler 930 units away at 23 units/s; the step timed out (60 s) at the switch
            // with the hint on it and 0 uses: 37/1.
            if (!nearby && level.time - s_stepStart < 25000)
            {
                GoUse(player, cmd, ent, target, targetDistance, false);
                if (targetDistance > 100.0f)
                    cmd->button_bits.resetBit(0);
                return true;
            }
        }
        // Harness: consume a frag only after arriving at its wall buy. Earlier
        // throws can be replenished by a pickup/last stand on the walk here.
        if (!strcmp(step.name, "wall_frag") && s_phase < 2)
        {
            if (!s_phase)
            {
                if (!GoUse(player, cmd, ent, target, targetDistance, false)
                    || !ps->cursorHint || ps->cursorHintEntIndex != ent->s.number)
                    return true;
                s_startAmmo = TotalAmmo(ps, BG_FindWeaponIndexForName(step.weapon));
                s_phaseTime = level.time;
                s_phase = 1;
                Log("phase", "at frag wall; throwing before replenishment");
            }
            cmd->button_bits.resetBit(0);
            cmd->button_bits.resetBit(11);
            if (level.time - s_phaseTime < 300)
                cmd->button_bits.setBit(14);
            const int ammo = TotalAmmo(ps, BG_FindWeaponIndexForName(step.weapon));
            if (ammo < s_startAmmo && level.time - s_phaseTime > 1500)
            {
                s_startAmmo = ammo;
                s_startScore = player->client->sess.cs.score.score;
                s_phase = 2;
                Log("phase", "frag consumed beside buy; measuring payment and ammo increase");
            }
            return true;
        }
        GoUse(player, cmd, ent, target, targetDistance, true);
        return true;
    }
    case STEP_TOUCH:
    {
        if (strstr(step.name, "trap_touch")) // L37: also v_trap_touch_s/_n
        {
            if (player->client->ps.shellshockIndex && player->client->ps.shellshockTime >= s_stepStart)
            {
                EndStep(player, "PASS", "shellshocked");
                return false;
            }
        }
        else if ((step.notify && StepNotifyTime(step) >= 0)
            || (!step.notify && s_teleportTime > s_stepStart))
        {
            EndStep(player, "PASS", step.notify ? "notify" : "teleported");
            return false;
        }
        // L40: water_calm waits (fighting where it stands) until George is angry.
        if (!strcmp(step.name, "water_calm") && LastNotifyTime("director_activated") < 0)
            return false;
        // q1 c9: a recovery portal_pack (after a thief round) may run while the pack room is still open (defcon 5,
        // no new open_pack_hideaway notify): arriving in the pack room is the step's effect.
        if (s_teleportTime > s_stepStart && step.notify && !strcmp(step.notify, "open_pack_hideaway")
            && (s_planFlags[s_index] & PLAN_INSERTED) && InPackRoom(player->r.currentOrigin))
        {
            EndStep(player, "PASS", "teleported into the pack room");
            return false;
        }
        // L42g r13: an astronaut headbutt teleport (astro_teleport) is not the step's teleporter; walk on.
        const int astroTeleport = LastNotifyTime("astro_teleport");
        if (s_teleportTime > s_stepStart && step.notify && level.time - s_teleportTime > 3000
            && !(astroTeleport >= 0 && abs(astroTeleport - s_teleportTime) <= 1000))
        {
            EndStep(player, "FAIL", "teleported without the notify");
            return false;
        }
        gentity_s *ent = s_ent >= 0 ? &g_entities[s_ent] : FindTarget(step, player, true);
        if (!ent)
            return false;
        if (ent->s.number != s_ent)
        {
            s_ent = ent->s.number;
            float center[3];
            Center(ent, center);
            char detail[160];
            sprintf_s(detail, "ent %d %s at %.0f %.0f %.0f", s_ent, Str(ent->script_noteworthy), center[0], center[1], center[2]);
            Log("target", detail);
        }
        float center[3];
        // L38: off Five, the nearest of the trigger's brushes. L40: water_calm goes to the water trigger's centre (the
        // beach lagoon, where George rises): the brush's nearest edge is dry sand above the water (w1: z 27-48, top 0).
        TouchPoint(ent, player, !s_isFive && strcmp(step.name, "water_calm"), center);
        float ground[3];
        GoalOnLevel(player, ent, center, ground);
        float viewYaw = ViewYaw(player, cmd);
        if (!target)
        {
            Face(player, cmd, center, false);
            viewYaw = ViewYaw(player, cmd);
        }
        Walk(player, ground, ent->s.number, cmd, viewYaw);
        return true;
    }
    case STEP_FLOOR:
    {
        const int floor = Floor(player->r.currentOrigin[2]);
        if (s_phase != 3 && floor == step.count)
        {
            EndStep(player, "PASS", "floor");
            return false;
        }
        // Floors 1-2 are joined by elevator2, floors 2-3 by elevator1.
        const bool elev2 = floor == 1 || (floor == 2 && step.count == 1);
        gentity_s *car = s_phase == 3 && s_rideCar >= 0 ? &g_entities[s_rideCar]
            : FindByTargetname(elev2 ? "elevator2" : "elevator1", nullptr);
        if (!car)
        {
            EndStep(player, "FAIL", "no elevator car");
            return false;
        }
        const char *carName = Str(car->targetname);
        float carCenter[3];
        Center(car, carCenter);
        if (s_phase == 3)
        {
            // Riding: stand still until the car has moved and settled. The car (MoveTo 5 s) must also
            // have stopped for 500 ms: one still player frame mid-ride ended d3's floor2 ride 46 units
            // short, and the next step then read the rest of that ride as its own.
            if (fabsf(carCenter[2] - s_rideCarZ) > 0.1f)
                s_rideCarMoved = level.time;
            s_rideCarZ = carCenter[2];
            if (fabsf(player->r.currentOrigin[2] - s_startOrigin[2]) > 150.0f && level.time - s_phaseTime > 1500
                && fabsf(player->r.currentOrigin[2] - s_lastOrigin[2]) < 0.5f && level.time - s_rideCarMoved >= 500)
            {
                char detail[128];
                sprintf_s(detail, "arrived floor %d z %.0f after %d ms", floor, player->r.currentOrigin[2], level.time - s_phaseTime);
                Log("ride", detail);
                s_phase = 0;
                s_ridePanelUsed = false;
                Vec3Copy(player->r.currentOrigin, s_startOrigin);
            }
            else if (level.time - s_phaseTime > 20000)
            {
                char detail[128];
                sprintf_s(detail, "car %s z %.0f player z %.0f", carName, carCenter[2], player->r.currentOrigin[2]);
                Log("ride stuck", detail);
                s_phase = 0;
                s_ridePanelUsed = false;
                // Re-press from here: comparing with the step's start height re-entered "moving" at once.
                Vec3Copy(player->r.currentOrigin, s_startOrigin);
            }
            return true;
        }
        const bool carHere = Floor(car->r.absmin[2] + 40.0f) == floor;
        char buy[32], station[32];
        sprintf_s(buy, "%s_buy", carName);
        if (!carHere)
        {
            // Call it: the call box of this floor (elevatorN_up / elevatorN_down).
            PlanStep call = { "call", STEP_USE, nullptr, nullptr, nullptr, 0 };
            sprintf_s(station, "%s_call_box", carName);
            call.targetname = station;
            gentity_s *box = FindTarget(call, player, true);
            if (!box || Floor(box->r.absmin[2] + 10.0f) != floor)
                return false;
            if (s_ent != box->s.number)
            {
                s_ent = box->s.number;
                char detail[96];
                sprintf_s(detail, "call box ent %d %s", s_ent, Str(box->script_noteworthy));
                Log("target", detail);
            }
            GoUse(player, cmd, box, target, targetDistance, level.time - s_triggerNotify[box->s.number] > 6000);
            return true;
        }
        gentity_s *buyTrigger = FindByNoteworthy(buy);
        if (!buyTrigger)
            return false;
        // Step into the car and press its buy panel (elevator_buy_think: UseTriggerRequireLookAt). The panel sits
        // by the doors, out of the 72-unit use reach from the car's centre, and the station's call box (a
        // trigger_use_touch, which the use list ranks first) covers the doorway: stand on the line from the
        // panel to the car's centre, as far from the panel as the reach allows, clear of the call boxes.
        float panel[3];
        Center(buyTrigger, panel);
        if (s_ent != buyTrigger->s.number)
        {
            s_ent = buyTrigger->s.number;
            s_phase = 1;
            s_ridePanelUsed = false;
            s_rideCar = car->s.number;
            const float lx = carCenter[0] - panel[0], ly = carCenter[1] - panel[1];
            const float length = sqrtf(lx * lx + ly * ly);
            sprintf_s(station, "%s_call_box", carName);
            float d = 64.0f;
            for (; d >= 24.0f; d -= 4.0f)
            {
                const float x = panel[0] + lx / length * d, y = panel[1] + ly / length * d;
                bool clear = true;
                for (int i = level.maxclients; i < level.num_entities && clear; ++i)
                {
                    const gentity_s *box = &g_entities[i];
                    if (box->r.inuse && !strcmp(Str(box->targetname), station) && x + 20.0f >= box->r.absmin[0]
                        && x - 20.0f <= box->r.absmax[0] && y + 20.0f >= box->r.absmin[1] && y - 20.0f <= box->r.absmax[1]
                        && player->r.currentOrigin[2] + 70.0f >= box->r.absmin[2] && player->r.currentOrigin[2] <= box->r.absmax[2])
                        clear = false;
                }
                if (clear)
                    break;
            }
            if (d < 24.0f)
                d = 60.0f;
            s_stand[0] = panel[0] + lx / length * d;
            s_stand[1] = panel[1] + ly / length * d;
            // The war room has two elevations. Route to the panel's floor even when
            // starting at Juggernog downstairs, rather than to the point beneath the car.
            s_stand[2] = panel[2] - 48.0f;
            char detail[192];
            sprintf_s(detail, "car %s at %.0f %.0f %.0f, buy ent %d at %.0f %.0f %.0f, stand %.0f %.0f (%.0f from the panel)",
                carName, carCenter[0], carCenter[1], carCenter[2], s_ent, panel[0], panel[1], panel[2], s_stand[0], s_stand[1], d);
            Log("target", detail);
        }
        // r1 c3: the panel rides with the car. r3a took the target while the called car was still coming down (panel
        // at z -127 between floors 1 and 2), walked toward that height on floor 2 for 70 s and timed out.
        s_stand[2] = panel[2] - 48.0f;
        // Harness: a call-box use or a previous elevator ride is not a press of
        // this car's panel. Require this car to move, not the player on the stairs.
        if (s_ridePanelUsed && player->r.currentOrigin[0] >= car->r.absmin[0] && player->r.currentOrigin[0] <= car->r.absmax[0]
            && player->r.currentOrigin[1] >= car->r.absmin[1] && player->r.currentOrigin[1] <= car->r.absmax[1]
            && fabsf(carCenter[2] - s_rideCarZ) > 0.1f)
        {
            s_phase = 3;
            s_phaseTime = level.time;
            s_rideCarZ = carCenter[2];
            s_rideCarMoved = level.time;
            Log("ride", "moving");
            return true;
        }
        const float sx = s_stand[0] - player->r.currentOrigin[0], sy = s_stand[1] - player->r.currentOrigin[1];
        if (sx * sx + sy * sy > 8.0f * 8.0f)
        {
            float viewYaw = ViewYaw(player, cmd);
            if (!target)
            {
                Face(player, cmd, panel, false);
                viewYaw = ViewYaw(player, cmd);
            }
            Walk(player, s_stand, car->s.number, cmd, viewYaw);
            return true;
        }
        Face(player, cmd, panel, true);
        if (target && targetDistance < 256.0f)
            return false;
        cmd->button_bits.resetBit(0);
        cmd->button_bits.resetBit(11);
        if (level.time - s_lastUse >= 500)
        {
            Vec3Copy(player->r.currentOrigin, s_startOrigin);
            s_lastUse = level.time;
            if (!s_ridePanelUsed && ps->cursorHint && ps->cursorHintEntIndex == buyTrigger->s.number)
            {
                s_ridePanelUsed = true;
                s_rideCarZ = carCenter[2];
            }
            if (s_phase != 2)
            {
                Vec3Copy(player->r.currentOrigin, s_startOrigin);
                s_phase = 2; // the car's panel was used (calling the car does not arm ride detection)
            }
            ++s_uses;
            char detail[128];
            sprintf_s(detail, "ent %d candidate %d (hint ent %d %s) score %d", buyTrigger->s.number,
                ps->cursorHintEntIndex == buyTrigger->s.number, ps->cursorHintEntIndex,
                ps->cursorHintEntIndex < 1023 ? Str(g_entities[ps->cursorHintEntIndex].targetname) : "", player->client->sess.cs.score.score);
            if (s_uses <= 3 || s_uses % 10 == 0)
                Log("use", detail);
        }
        if (level.time - s_lastUse < 150)
            cmd->button_bits.setBit(3);
        return true;
    }
    case STEP_BOX:
    case STEP_PAP:
    {
        const bool box = step.kind == STEP_BOX;
        // Harness: wait for a naturally collected fire sale, then use the retail box.
        if (!strcmp(step.name, "box_sale") && s_phase == 0 && ZombieVar(F_FIRESALE) != 1)
        {
            s_startScore = player->client->sess.cs.score.score;
            return false;
        }
        if (box && step.notify && NotifyTime(step.notify, s_stepStart) >= 0 && s_phase != 1)
        {
            EndStep(player, "PASS", "notify");
            return false;
        }
        gentity_s *ent = s_ent >= 0 ? &g_entities[s_ent] : FindTarget(step, player, true);
        if (!ent)
        {
            if (level.time - s_stepStart > 5000 && s_ent < 0)
                EndStep(player, "FAIL", "no enabled trigger");
            return false;
        }
        if (ent->s.number != s_ent)
        {
            s_ent = ent->s.number;
            float center[3];
            Center(ent, center);
            if (box)
            {
                Vec3Copy(center, s_boxViewCenter);
                s_boxViewValid = true;
            }
            char detail[160];
            sprintf_s(detail, "ent %d %s %s at %.0f %.0f %.0f", s_ent, Str(ent->targetname), Str(ent->script_noteworthy),
                center[0], center[1], center[2]);
            Log("target", detail);
        }
        const int score = player->client->sess.cs.score.score;
        if (s_phase == 0)
        {
            // Pay: the script charges (its own self.cost: a fire sale / bonfire sale lowers the box and
            // Pack-a-Punch prices) and switches the trigger off while it works. Kill points earned
            // while walking to the trigger raise the baseline, so they cannot hide the charge.
            if (score > s_startScore)
                s_startScore = score;
            if (s_startScore - score > 0)
            {
                const int paid = s_startScore - score;
                s_phase = 1;
                s_phaseTime = level.time;
                s_startScore = score;
                for (int i = 0; i < 15; ++i)
                    s_startWeapons[i] = ps->heldWeapons[i].weapon;
                char detail[96];
                sprintf_s(detail, "paid %d, trigger enabled %d", paid, TriggerEnabled(ent));
                Log("phase", detail);
                if (!strcmp(step.name, "box_sale") && paid != 10)
                    EndStep(player, "FAIL", "fire sale requires a measured 10-point purchase");
                return false;
            }
            // L20: Pack-a-Punch is only reachable in the pack room, and only for a while after open_pack_hideaway
            // (maps\zombie_pentagon_teleporter pack_hideaway_init: trigger_on after the closet's 2.5 s turn, later
            // trigger_off and the closet turns back). v1-2: portal_pack reached the room during a Bonfire Sale at
            // 267800, a pad took the player out at 268800, it walked back in at 287100 after the closet had closed and
            // stood at the dark machine until the 120 s timeout (pap end FAIL, uses 0). v2-1: the thief round in the
            // room ended at 247750 and the game put the player out at 248800 (50 ms after pap restarted); pap walked to
            // the machine's nearest point on the war room floor above it until the timeout. Out of the room for 3 s,
            // or in it with the trigger off for 8 s: back through portal_pack (from inside, out by a pad first); its
            // arm check (PackCheck) pulls the defcon switches to 5 again when the level has dropped.
            static int s_papAwayFrom = -1; // level time pap was first seen unable to reach the machine (-1: able)
            const bool inRoom = InPackRoom(player->r.currentOrigin);
            if (box || (inRoom && TriggerEnabled(ent)) || level.time - s_stepStart < 1000)
                s_papAwayFrom = -1;
            else if (s_papAwayFrom < 0)
                s_papAwayFrom = level.time;
            if (s_isFive && s_papAwayFrom >= 0 && level.time - s_papAwayFrom >= (inRoom ? 8000 : 3000) && s_recoveryFails < 2
                && s_papRequeues < 2)
            {
                char why[160];
                sprintf_s(why, "Pack-a-Punch out of reach for %d s (in the pack room %d, trigger on %d, defcon %d); back through portal_pack",
                    (level.time - s_papAwayFrom) / 1000, inRoom ? 1 : 0, TriggerEnabled(ent) ? 1 : 0, LevelInt(F_DEFCON, -1));
                Log("requeue", why);
                ++s_papRequeues;
                s_papAwayFrom = -1;
                s_stepStart = 0;
                s_phase = 0;
                for (int i = 0; i < (int)ARRAY_COUNT(s_steps); ++i)
                    if (!strcmp(s_steps[i].name, "portal_pack"))
                        InsertStep(i, 0, why);
                s_planFlags[s_index] &= ~PLAN_ARMED; // portal_pack's arm check (PackCheck)
                if (inRoom)
                    for (int i = 0; i < (int)ARRAY_COUNT(s_steps); ++i)
                        if (!strcmp(s_steps[i].name, "portal"))
                            InsertStep(i, 0, why);
                return false;
            }
            if (!TriggerEnabled(ent))
                return false;
            // r1 c3: not into the machine just before a thief round. r3d paid at 225050 between rounds, the thief
            // round began at 225900 and teleported the player out with the gun still in the machine; the thief took
            // the other one, and with weapon none can_buy_weapon refuses every wall buy (downed with the knife).
            // w1 c11 pap_thief: the opposite on purpose. Ask the retail devgui for the thief round (goto_round
            // next_thief_round), then pay between rounds with the thief round next, as r3d did.
            if (!box && !strncmp(step.name, "pap_thief", 9) && !ThiefRoundComing())
            {
                static int s_devguiSent = -1;
                if (s_devguiSent != s_stepStart)
                {
                    if (!Dvar_GetBool("developer_script"))
                    {
                        EndStep(player, "FAIL", "pap_thief: the retail devgui requires developer_script before map load");
                        return false;
                    }
                    s_devguiSent = s_stepStart;
                    Dvar_SetStringByName("zombie_devgui", "thief_round");
                    char detail[96];
                    sprintf_s(detail, "devgui thief_round: round %d next_thief_round %d", LevelInt(F_ROUND, -1),
                        LevelInt(F_NEXTTHIEF, -1));
                    Log("devgui", detail);
                }
                // Stand at the machine meanwhile: the pack room's arrival pad is next to the exit pad (w1c11a: the
                // fight policy stepped onto it at 301800 and left the room).
                GoUse(player, cmd, ent, target, targetDistance, false);
                return true;
            }
            if (!box && strncmp(step.name, "pap_thief", 9) && ThiefRoundComing())
            {
                static int s_holdLogged = -1;
                if (s_holdLogged != s_stepStart)
                {
                    s_holdLogged = s_stepStart;
                    Log("hold", "between rounds before a thief round; the gun stays out of the machine");
                }
                return false;
            }
            GoUse(player, cmd, ent, target, targetDistance, true);
            return true;
        }
        // w1 c11 box_leave: pay, never take, walk away. treasure_chest_think: randomization_done -> enable_trigger and
        // treasure_chest_timeout (wait 12, notify trigger level) -> timedOut: disable_trigger, lid close, wait 3,
        // enable_trigger; the teddy refund is the only add_to_player_score. Evidence: trigger times, score, weapons.
        if (box && !strcmp(step.name, "box_leave"))
        {
            static int s_leaveState, s_leaveOpen, s_leaveClose, s_leaveScore, s_leaveRefund, s_leaveFrom = -1;
            if (s_leaveFrom != s_phaseTime)
            {
                s_leaveFrom = s_phaseTime;
                s_leaveState = s_leaveOpen = s_leaveClose = s_leaveRefund = 0;
                s_leaveScore = score;
            }
            if (score - s_leaveScore >= step.cost)
            {
                char detail[96];
                sprintf_s(detail, "score jump %d -> %d in one frame (refund?)", s_leaveScore, score);
                Log("leave", detail);
                ++s_leaveRefund;
            }
            s_leaveScore = score;
            if (NotifyTime("moving_chest_now", s_phaseTime) >= 0)
            {
                // The teddy: treasure_chest_move and the script's refund; not the timeout path. Pay at the next box.
                Log("leave", "teddy (moving_chest_now); pay again at the next box");
                s_phase = 0;
                s_ent = -1;
                s_startScore = score;
                return false;
            }
            const bool enabled = TriggerEnabled(ent);
            char detail[256];
            if (s_leaveState == 0 && enabled)
            {
                s_leaveState = 1;
                s_leaveOpen = level.time;
                sprintf_s(detail, "grab window open at paid+%d", level.time - s_phaseTime);
                Log("leave", detail);
                Shot("boxleave_open");
            }
            else if (s_leaveState == 1 && !enabled)
            {
                s_leaveState = 2;
                s_leaveClose = level.time;
                sprintf_s(detail, "timed out: trigger off at open+%d (paid+%d), chest_accessed %d", level.time - s_leaveOpen,
                    level.time - s_phaseTime, LevelInt(F_CHESTACCESSED, -1));
                Log("leave", detail);
            }
            else if (s_leaveState == 2 && enabled)
            {
                const bool weaponsKept = !WeaponsChanged(ps);
                sprintf_s(detail, "trigger back at close+%d (paid+%d); score %d; refunds %d; weapons %s %s; chest_accessed %d",
                    level.time - s_leaveClose, level.time - s_phaseTime, score, s_leaveRefund,
                    weaponsKept ? "unchanged" : "CHANGED", WeaponList(ps).c_str(), LevelInt(F_CHESTACCESSED, -1));
                Log("leave", detail);
                Shot("boxleave_back");
                EndStep(player, weaponsKept && !s_leaveRefund ? "PASS" : "FAIL", "timed-out box: trigger back, no refund");
                return false;
            }
            if (level.time - s_phaseTime > 30000)
            {
                EndStep(player, "FAIL", "box_leave: no timeout cycle within 30 s of paying");
                return false;
            }
            // Walk away: to 300 units from the box on the side the player stands, then hold (the fight policy).
            float center[3], away[3];
            Center(ent, center);
            Vec3Sub(player->r.currentOrigin, center, away);
            away[2] = 0.0f;
            const float distance = Vec3Normalize(away);
            if (distance > 250.0f || player->client->lastStand)
                return false;
            float goal[3] = { center[0] + away[0] * 300.0f, center[1] + away[1] * 300.0f, player->r.currentOrigin[2] };
            cmd->button_bits.resetBit(3);
            Walk(player, goal, 3002, cmd, ViewYaw(player, cmd));
            return true;
        }
        // w1 c12: the pap trigger stays usable while third_person_weapon_upgrade runs, in SP as in KB. disable_trigger's
        // origin -= (0,0,10000) lands on a trigger pack_hideaway_init LinkTo'd to the hideaway; the origin field setter
        // (SP 0x00530060: G_SetOrigin + SV_LinkEntity, no link-offset update) moves it for one frame and the link
        // (G_SetFixedLink, SP 0x00623840) puts it back. Player_FindUsableEntities (SP 0x00817DB0) has no enabled state
        // beyond bounds / eType / the per-client mask, so the machine stays in the use list and a press finds no
        // waiter (vending_weapon_upgrade is inside third_person_weapon_upgrade). The presses from paid+1000 on must
        // neither charge again nor hand the gun over before the upgrade is done.
        if (!box)
        {
            static int s_papFrom = -1, s_papHigh;
            if (s_papFrom != s_phaseTime)
            {
                s_papFrom = s_phaseTime;
                s_papHigh = score;
            }
            if (score > s_papHigh)
                s_papHigh = score;
            else if (s_papHigh - score >= 1000 && !player->client->lastStand && !HasUpgradedWeapon(ps))
            {
                char detail[96];
                sprintf_s(detail, "charged again mid-upgrade: %d -> %d at paid+%d", s_papHigh, score, level.time - s_phaseTime);
                EndStep(player, "FAIL", detail);
                return false;
            }
        }
        // w1 c11 pap_thief: the gun stays in the machine until the thief round has started (flag thief_round); the
        // observer (PapThiefObserve) ends the step when the script clears pack_machine_in_use (taken or timed out).
        const bool papThief = !box && !strncmp(step.name, "pap_thief", 9);
        if (papThief && (LevelFlag("thief_round") != 1 || HasUpgradedWeapon(ps) || !strcmp(step.name, "pap_thief_leave")))
        {
            if (HasUpgradedWeapon(ps) || !InPackRoom(player->r.currentOrigin))
                return false;
            GoUse(player, cmd, ent, target, targetDistance, false);
            return true;
        }
        // w1 c11: -Client evidence of the upgraded worldgun (the camo'd weapon options) in the machine before the take.
        // The linked pap trigger reads enabled right after the purchase (w1c11d2 took the shot at paid+750, the base gun
        // still rolling in): wait for third_person_weapon_upgrade's 3850 ms (w1 c10 measure) as well.
        if (!box && s_shotsDvar && s_shotsDvar->current.enabled && TriggerEnabled(ent) && level.time - s_phaseTime >= 3900)
        {
            static int s_papShotFrom = -1, s_papShotOpen;
            if (s_papShotFrom != s_phaseTime)
            {
                s_papShotFrom = s_phaseTime;
                s_papShotOpen = level.time;
            }
            if (level.time - s_papShotOpen < 1500)
            {
                if (level.time - s_papShotOpen == 750)
                    Shot("pap_worldgun");
                float center[3];
                Center(ent, center);
                cmd->forwardmove = cmd->rightmove = 0;
                cmd->button_bits.resetBit(3);
                Face(player, cmd, center, true);
                return true;
            }
        }
        // Take: the trigger comes back (box: weapon shown; pap: upgrade done); press use again.
        const bool taken = box ? WeaponsChanged(ps) : HasUpgradedWeapon(ps);
        if (taken)
        {
            ++s_spins;
            char detail[160];
            sprintf_s(detail, "took at paid+%d, weapons %s", level.time - s_phaseTime, WeaponList(ps).c_str());
            Log("phase", detail);
            if (!box || s_spins >= step.count)
            {
                EndStep(player, "PASS", box ? "weapon" : "upgraded weapon");
                return false;
            }
            s_phase = 0;
            s_startScore = score;
            s_uses = 0;
            return false;
        }
        if (box && step.notify && NotifyTime(step.notify, s_phaseTime) >= 0)
        {
            EndStep(player, "PASS", "notify");
            return false;
        }
        if (level.time - s_phaseTime > 20000 && !papThief)
        {
            // Timed out (box: the weapon sank back); pay again.
            Log("phase", "take window missed");
            s_phase = 0;
            s_startScore = score;
            return false;
        }
        if ((level.time - s_phaseTime) % 2000 == 0)
        {
            char detail[192];
            sprintf_s(detail, "take wait: trigger enabled %d weapon %d state %d hold %d target %d dist %.0f hint %d/%d",
                TriggerEnabled(ent), ps->weapon, ps->weaponstate, s_holdWeapon, target ? target->s.number : -1,
                targetDistance, ps->cursorHint, ps->cursorHintEntIndex);
            Log("phase", detail);
        }
        if (!TriggerEnabled(ent))
            return false;
        // r1 arm policy: the box takes the held gun when two are held; raise the one worth least (BoxGiveUp).
        if (box)
        {
            if (!s_holdWeapon || !BG_PlayerHasWeapon(ps, s_holdWeapon))
                s_holdWeapon = BoxGiveUp(ps);
            if (s_holdWeapon && !HoldWeapon(player, client, cmd, s_holdWeapon))
            {
                GoUse(player, cmd, ent, target, targetDistance, false);
                return true;
            }
        }
        GoUse(player, cmd, ent, target, targetDistance, level.time - s_phaseTime > 1000);
        return true;
    }
    }
    return false;
}
