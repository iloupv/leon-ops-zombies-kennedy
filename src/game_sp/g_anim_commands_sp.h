#pragma once

struct gentity_s;
struct XAnimTree_s;
struct client_t;
struct msg_t;

// XAnim applies commands immediately; the history retains commands for snapshot replay.
int G_StoreAnimCommand(gentity_s *ent, XAnimTree_s *tree, int type, unsigned int anim,
    unsigned int root, float weight, float time, float rate, unsigned char flags);
void G_SP_ResetAnimCommands();
// SP 0x004fdf30: free ent's stored commands (G_SetAnimTree, G_FreeEntity). Zombiemode only.
void G_SP_FreeEntityAnimCommands(const gentity_s *ent);
int G_SP_GetAnimCommandSequence(int entnum);
void SV_SP_CollectAnimCommands();
void G_SP_GetAnimCommandUsage(int *referenced, int *protectedCommands, int *snapshotTime);
void SV_SP_WriteAnimCommands(client_t *client, msg_t *msg);

struct MemoryFile;
// zombies: the animation command sections of G_SaveState segment 3 (SP 0x007EDD8A, 0x007EDDDA,
// 0x007EA760) and their G_LoadGame readers (SP 0x007ED056, 0x007ED077, 0x007EA890).
void G_SP_WriteAnimCommandRecords(MemoryFile *memFile);
void G_SP_WriteAnimCommandList(MemoryFile *memFile);
void G_SP_WriteAnimCommandHeads(MemoryFile *memFile);
void G_SP_ReadAnimCommandRecords(MemoryFile *memFile);
void G_SP_ReadAnimCommandList(MemoryFile *memFile);
void G_SP_ReadAnimCommandHeads(MemoryFile *memFile);
// KB diagnostic: keep the live history across a check read (true saves, false restores).
void G_SP_BackupAnimCommands(bool save);

