#pragma once

// zombies (a1): bo1_audio request trace. Measurement only - it adds no SP logic and changes no outcome.
//
// The SP exe has no null-output sound path: SND_PlayInternal (SP 0x00620AA0) and SND_FindAlias
// (SP 0x004258C0) drop every request while SND_Active() (SP 0x0045B490) is 0, exactly like BO1Zombies.
// A headless run never opens a device (nosnd), so this traces at the REQUEST level: every play request
// is logged with its alias, entity and origin before the SND_Active gate, and its outcome is what the
// alias tables say (the banks are registered by the zone loader with or without a device):
//   queued              - alias resolves and the sound system is active (voice limit / culling happen later,
//                         on the sound thread, and only with a device)
//   resolved_no_device  - alias resolves; the request is dropped because no sound device is open
//   alias_missing       - no loaded bank has the alias
//   no_alias            - the request carries alias id 0 (empty name)
// Lines: "BO1_AUDIO <side> <api> alias=<name> ent=<n> org=(x y z) t=<ms> -> <outcome>[ <extra>] lt=<level ms>[ 2d|3d][ nofile]"
// (lt = the server's level.time for sv lines, the client's cg time for cl lines; 2d/3d = the head alias's positioned
// flag; nofile = the head alias has no SoundFile). A client play is followed by one "secondary" line per link of the
// alias's secondary chain ("-> <outcome> of=<parent>"), which the exe starts with it.

void SND_BO1TraceInit();
// The level clock the trace lines carry: G_RunFrame (server) and CG_DrawActiveFrame (client) set it.
void SND_BO1TraceSetTime(bool client, int ms);
bool SND_BO1TraceEnabled();
void SND_BO1TraceName(unsigned int id, const char *name);
void SND_BO1Trace(const char *side, const char *api, unsigned int id, int ent, const float *origin, const char *extra = 0);
// Entity loop sounds are re-requested every client frame: log a (ent, alias) once until it has not been
// pumped for a second, and suppress the SND_PlayInternal line of the pumped request.
void SND_BO1TraceLoop(const char *side, unsigned int id, int ent, const float *origin);
void SND_BO1TraceSuppress(bool suppress);
bool SND_BO1TraceSuppressed();
void SND_BO1TraceBanks(const char *when);
// A free-form trace line ("BO1_AUDIO <text> t=<ms>"), e.g. the music / client-system state changes.
void SND_BO1TraceText(const char *fmt, ...);
// zombies (L16): bo1_audio_voices <ms> (default 0 = off): the per-voice position / falloff / pan trace of SND_UpdateVoice
// (snd.cpp SND_BO1TraceVoice). Measurement only.
int SND_BO1TraceVoiceInterval();

// zombies (a1): bo1_snd_nodevice_length (default 0). NOT trace-only: when set and no sound device is open, a notify
// play reports the length the voice would have reported (snd_public_async.cpp SND_BO1NoDeviceLength), so a headless
// run gets SP's "sl" sounddone timing instead of the 5000 ms fallback. Measurement switch; APPROXIMATED.
bool SND_BO1NoDeviceLengthEnabled();
// a1 chunk 4, same switch: SND_FindAlias returns 0 while no device is open (SND_FindAliasFromId tests SND_Active), so
// the script builtin SoundExists said "no" for every alias and the zombie scripts skipped every player vox line
// (vox_plr_*). With the switch set and no device, this answers from the loaded banks as a device build would.
bool SND_BO1NoDeviceAliasExists(const char *name);
