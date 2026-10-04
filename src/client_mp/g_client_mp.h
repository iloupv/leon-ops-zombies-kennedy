#pragma once
#include <bgame/bg_public.h>
#include <server_mp/sv_voice_mp.h>
#include <qcommon/msg_mp.h>
#include <client_mp/client_mp.h>

enum sessionState_t : __int32
{                                       // XREF: clientSession_t/r
    SESS_STATE_PLAYING = 0x0,
    SESS_STATE_DEAD = 0x1,
    SESS_STATE_SPECTATOR = 0x2,
    SESS_STATE_INTERMISSION = 0x3,
};

enum clientConnected_t : __int32
{                                       // XREF: clientSession_t/r
    CON_DISCONNECTED = 0x0,
    CON_CONNECTING = 0x1,
    CON_CONNECTED = 0x2,
};

struct playerTeamState_t // sizeof=0x4
{                                       // XREF: clientSession_t/r
    int location;
};

struct __declspec(align(8)) clientSession_t // sizeof=0x1F0
{                                       // XREF: gclient_s/r
                                        // ?ClientSpawn@@YAXPAUgentity_s@@QBM1@Z/r
    sessionState_t sessionState;
    int forceSpectatorClient;
    int killCamEntity;
    int killCamTargetEntity;
    int archiveTime;
    unsigned int scriptPersId;
    clientConnected_t connected;
    usercmd_s cmd;
    usercmd_s oldcmd;
    int localClient;
    int predictItemPickup;
    char newnetname[32];
    int maxHealth;
    int enterTime;
    playerTeamState_t teamState;
    int voteCount;
    int teamVoteCount;
    float moveSpeedScaleMultiplier;
    int viewmodelIndex;
    int noSpectate;
    int teamInfo;
    clientState_s cs;
    int psOffsetTime;
    int scoreboardColumnCache[18];
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

void __cdecl SetClientViewAngle(gentity_s *ent, const float *angle);
void __cdecl G_GetPlayerViewOrigin(const playerState_s *ps, float *origin);
void __cdecl G_GetPlayerBodyViewOrigin(const playerState_s *ps, float *origin);
void __cdecl G_GetPlayerViewDirection(const gentity_s *ent, float *forward, float *right, float *up);
void __cdecl ClientUserinfoChanged(unsigned int clientNum);
void __cdecl ClientCleanName(const char *in, char *out, int outSize);
const char *__cdecl ClientConnect(unsigned int clientNum, unsigned int scriptPersId);
void __cdecl ClientClearFields(gclient_s *client);
void __cdecl ClientBegin(unsigned int clientNum);
void __cdecl ClientSpawn(gentity_s *ent, const float *spawn_origin, const float *spawn_angles);
//clientSession_t *__thiscall clientSession_t::clientSession_t(clientSession_t *this); // should be implicit
void __cdecl ClientDisconnect(unsigned int clientNum);
unsigned int __cdecl G_GetNonPVSPlayerInfo(gentity_s *pSelf, float *vPosition, int iLastUpdateEnt);
void __cdecl G_BroadcastVoice(gentity_s *talker, VoicePacket_t *voicePacket);
