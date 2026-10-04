#pragma once

// p1 TEST SWITCHES (headless only): bo1_testclient_move / fall / weapon.
struct client_t;
struct usercmd_s;
struct playerState_s;
void G_SP_HeadlessMoveRegister();
// Fills cmd for the test client and returns true while the movement lab is on.
bool G_SP_HeadlessMoveCommand(client_t *client, usercmd_s *cmd);
// p1 TEST SWITCH bo1_feeltrace (headless client): server-side lunge / knife trace for the local player.
void G_SP_FeelTraceServerFrame();
// k1 TEST SWITCH bo1_testclient_replay (headless test client): replay an exported retail recording's usercmds
// (tools/k1_replay_export.mjs). Runs the recorded cmds before this frame's time itself and leaves the last in cmd.
bool G_SP_HeadlessReplayCommand(client_t *client, usercmd_s *cmd);
// k1 c12: level notifies (the replay takes its clock alignment from "begin_spawning").
void G_SP_HeadlessReplayLevelNotify(const char *name);
// k1 c17 (headless test client): runs a client-dvar server command ('v') the way SP's local client would.
void G_SP_HeadlessTestClientServerCommand(const client_t *client, const char *cmd);
// a1 c9: a script weapon select ('a' server command) for the replayed test client (bo1_testclient_replay_buttons)
void G_SP_HeadlessReplaySelectWeapon(int weapon);
// a1 c9: the bullet spread seed time (Bullet_Endpos shotIndex + gameTime) of the replayed test client on the recording's
// cmd clock (bo1_testclient_replay_buttons); every other shooter gets gameTime back
struct gentity_s;
int G_SP_HeadlessReplayShotTime(const gentity_s *attacker, int gameTime);
// a1 c10: the replayed test client's recorded gun angles (SP usercmd gun pitch / yaw) for the cmd at cmdServerTime
bool G_SP_HeadlessReplayGunAngles(const gentity_s *ent, int cmdServerTime, float *pitch, float *yaw);
// a1 chunk 4 (-Client): the replay drives the LOCAL client. Server frame: god bit, drift snap, published clock.
bool G_SP_HeadlessReplayLocalFrame(client_t *client);
// Client thread (CL_CreateCmd): the recorded cmd at cmd->serverTime; false when the replay is off.
bool G_SP_HeadlessReplayLocalCommand(usercmd_s *cmd, const playerState_s *predictedPs);
