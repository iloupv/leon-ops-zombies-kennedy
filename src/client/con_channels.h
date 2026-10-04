#pragma once
#include "cl_console.h"

char __cdecl Con_OpenChannel(const char *name, bool allowScript);
bool __cdecl Con_ScriptHasPermission(unsigned int channel);
bool __cdecl Con_GetChannel(const char *name, int *channel_result);
bool __cdecl Con_IsChannelOpen(unsigned int channel);
bool __cdecl Con_IsChannelVisible(print_msg_dest_t dest, unsigned int channel, int errorflags);
void __cdecl Con_WriteFilterConfigString(int f);
void __cdecl Con_InitGameMsgChannels();
void __cdecl Con_InitChannelsForDestFromList(print_msg_dest_t dest, const char *channelNames);
void __cdecl Con_FilterShowChannel(print_msg_dest_t dest, const char *channelName, bool show);
void __cdecl Com_BitClearAssert(unsigned int *array, int bitNum, int size);
void __cdecl Con_InitChannels();
void __cdecl Con_ChannelList_f();
void __cdecl Con_FilterAdd_f();
void __cdecl Con_FilterAdd(bool show);
void __cdecl Con_FilterRemove_f();
void __cdecl Con_FilterList_f();
void __cdecl Con_ShutdownChannels();
void __cdecl Con_CloseChannelInternal(unsigned int channel);
