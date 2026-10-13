#pragma once

// mod: sounds built at level load from loose WAV files (PCM 16-bit, mono or stereo) under the mod's sound/rt/, as
// the runtime models are (gfx_d3d/r_runtime_xmodel.cpp). Each file is a variant of the alias named by its stem without
// a trailing "_<number>" (sound/rt/leon_pain_1.wav and leon_pain_2.wav: alias "leon_pain", one picked at random).
void __cdecl SND_Runtime_LoadForLevel();
// plays a runtime alias as the local player's 2D sound; false when there is none by that name
bool __cdecl SND_Runtime_PlayLocal(int localClientNum, const char *alias);
// true for an alias the runtime voice replaces: with Leon's voice loaded ("leon_pain"), the zombies characters' own
// lines (vox_plr_*) play silent - dropped, a script waiting on their notify would wait forever
bool __cdecl SND_Runtime_MutesVoice(unsigned int id);
