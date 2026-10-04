#pragma once
#include "demo_common.h"

#include <qcommon/msg_mp.h>

struct demoTagPlayers;
struct playerState_s;

struct __declspec(align(4)) streamingState_s // sizeof=0x1C
{                                       // XREF: .data:streamingState/r
    unsigned __int8 *buffer;            // XREF: Demo_SetupClipStreamingState(void)+2E/w
                                        // Demo_SetupClipStreamingState(void)+6F/w ...
    int bufferSize;                     // XREF: Demo_SetupClipStreamingState(void)+3F/w
                                        // Demo_SetupClipStreamingState(void)+8D/w ...
    int phase;                          // XREF: Demo_MoveToNextPhase(void)+4/r
                                        // Demo_MoveToNextPhase(void):$LN7_165/w ...
    int segmentNo;                      // XREF: Demo_MoveToNextPhase(void)+37/r
                                        // Demo_MoveToNextPhase(void):$LN3_240/r ...
    int byteOffset;                     // XREF: Demo_SetupClipStreamingState(void):def_9C63CB/w
                                        // Demo_SendClipToStreamingBuffer(void *,uint,uint)+32/w ...
    int totalDuration;                  // XREF: Demo_SetupClipStreamingState(void)+A4/r
                                        // Demo_SetupClipStreamingState(void)+B0/w ...
    bool isEnteringState;               // XREF: Demo_MoveToNextPhase(void):def_9C6315/w
                                        // Demo_SetupClipStreamingState(void)+205/w ...
    // padding byte
    // padding byte
    // padding byte
};

void __cdecl Demo_AllocatePlaybackMemory(unsigned int location);
void __cdecl Demo_DeallocatePlaybackMemory();
void __cdecl Demo_Load();
void    Demo_HiResScreenshot(int localClientNum, int tiles);
void __cdecl Demo_TagPlayers(int localClientNum, demoTagPlayers *players);
void __cdecl Demo_TagPlayer(demoTagPlayers *players, unsigned __int64 playerXuid);
char __cdecl Demo_IsPlayerTagged(int localClientNum, demoTagPlayers *players, unsigned __int64 playerXuid);
void __cdecl Demo_SaveScreenshotSuccessful(int controllerIndex, unsigned __int64 fileID);
void __cdecl Demo_SaveScreenshot(int localClientNum, unsigned int fileSlot);
void __cdecl Demo_SetCmdInProgess();
void __cdecl Demo_SetCmdCompleted();
bool __cdecl Demo_ShouldProcessCmd(int cmd);
bool __cdecl Demo_IsPlaybackInited();
bool __cdecl Demo_IsPaused();
bool __cdecl Demo_KeyframeGenerated();
void __cdecl Demo_SetCompleted(bool flag);
bool __cdecl Demo_IsCompleted();
bool __cdecl Demo_IsShutdownInProgress();
char __cdecl Demo_IsClipRecording();
bool __cdecl Demo_IsClipPlaying();
bool __cdecl Demo_GetClipPausedState();
void __cdecl Demo_SetJumpTimeFlag(bool flag);
bool __cdecl Demo_GetJumpTimeFlag();
void __cdecl Demo_SetSwitchedPlayersFlag(bool flag);
bool __cdecl Demo_GetSwitchedPlayersFlag();
bool __cdecl Demo_ShouldEntityBeVisibleToClient(int entNum);
bool __cdecl Demo_HasClientSpawnedIn(int clientNum);
void __cdecl Demo_SetupForPlayerSwitch(int localClientNum);
char __cdecl Demo_SwitchPlayer(int localClientNum, bool reverse, int forceClientNum, bool forceToLivePlayer);
bool __cdecl Demo_ShouldUpdateViewAngles(int localClientNum);
int __cdecl Demo_GetKeyFrameTime(int index);
int __cdecl Demo_GetCurrentControllerConfig();
bool __cdecl Demo_IsPublicOnlineMatch();
bool __cdecl Demo_IsCombatTrainingMatch();
bool __cdecl Demo_IsWagerMatch();
bool __cdecl Demo_IsCustomGameMode();
int __cdecl Demo_GetKeyFrameForJumpBack(int localClientNum, int newServerTime);
int __cdecl Demo_GetKeyFrameForJumpForward(int localClientNum, int currentTime);
void __cdecl Demo_DrawTransitionScreen(int localClientNum);
void __cdecl Demo_ActivateTransitionScreen(int transition);
bool __cdecl Demo_IsTransitionScreenActive();
void __cdecl Demo_DrawMetaInformationScreen(int localClientNum);
bool __cdecl Demo_IsMetaInformationScreenActive();
void __cdecl Demo_ResetSnapshotData();
void __cdecl Demo_ResetTimeValues(int localClientNum, int time);
void __cdecl Demo_ResetSequenceNumbers(int localClientNum, int sequence);
void __cdecl Demo_ResetWorldInformation(int localClientNum, int time);
bool __cdecl Demo_IsGameHudHidden();
bool __cdecl Demo_IsDemoHudHidden();
int __cdecl Demo_GetCurrentCameraMode();
int __cdecl Demo_GetNextCameraMode(int cameraMode);
void __cdecl Demo_SwitchCameraMode(int localClientNum, unsigned int newCamMode);
void __cdecl Demo_UpdateVisibilityBitsForCameraMode(int localClientNum, int cameraMode);
bool __cdecl Demo_IsThirdPersonCamera();
bool __cdecl Demo_IsMovieCamera();
bool __cdecl Demo_ShouldUpdateMovieCamera(int localClientNum);
bool __cdecl Demo_IsRenderingScreenshot(int localClientNum);
bool __cdecl Demo_IsRenderingMovie();
void __cdecl Demo_SetClipState(int state);
int __cdecl Demo_GetClipState();
bool __cdecl Demo_IsClipPreviewRunning();
void __cdecl Demo_LerpClipCameraValues(int localClientNum);
void __cdecl Demo_UpdateClipTime(int msec);
int __cdecl Demo_GetClipTime();
int __cdecl Demo_GetTotalClipDuration();
void __cdecl Demo_SwitchTransition(int index, bool direction);
void __cdecl Demo_SetClipModified(bool flag);
bool __cdecl Demo_IsClipModified();
void __cdecl Demo_ProcessPlayback();
void __cdecl Demo_InitPlaybackData(int localClientNum);
void __cdecl Demo_RestorePreDemoSettings();
void __cdecl Demo_ReadHeader();
void __cdecl Demo_ReadGamestate(int localClientNum);
char __cdecl Demo_ReadDemoMessage(int localClientNum);
void __cdecl Demo_RunFirstFrameEvents(int localClientNum);
void __cdecl Demo_ParseServerMessage(int localClientNum, msg_t *msg, int msgType, int msgSequence);
void __cdecl Demo_ParseServerCommands(int localClientNum, msg_t *msg);
bool __cdecl Demo_ShouldFilterServerCommandForMovieCamera(const char *cmd);
void __cdecl Demo_ParseSnapshot(int localClientNum, msg_t *msg);
void __cdecl Demo_ReadMatchState(int localClientNum, msg_t *msg, int time);
playerState_s *__cdecl Demo_ReadPlayerStates(playerState_s *result, int localClientNum, msg_t *msg, int time);
int __cdecl Demo_ReadPacketEntities(int localClientNum, msg_t *msg, int time, int deltaNum);
int __cdecl Demo_ReadPacketClients(int localClientNum, msg_t *msg, int time, int deltaNum);
void __cdecl Demo_SetupPlaybackForClient(int localClientNum, playerState_s *ps);
void __cdecl Demo_RegisterHeliPatches();
char __cdecl Demo_ReadInfoData(char *name, int handle);
void __cdecl Demo_SetDefaultClient(int localClientNum, int defaultClient);
void __cdecl Demo_SetDefaultVisionSet(int localClientNum);
void __cdecl Demo_UpdateCurrentSnapshot(int localClientNum, playerState_s *ps);
void __cdecl Demo_EnableSnapshotProcessing();
void __cdecl Demo_DisableSnapshotProcessing();
void    Demo_GenerateUncompressedSnapshot(
                int localClientNum,
                msg_t *msg,
                int msgSequence,
                int cmdSequence,
                int serverTime,
                int physicsTime,
                int snapFlags,
                bool writeAllPlayerStates);
void __cdecl Demo_WriteCustomSnapshotInformation(int localClientNum, msg_t *msg);
void __cdecl Demo_ReadCustomSnaphotInformation(int localClientNum, msg_t *msg);
void __cdecl Demo_UpdateDesiredTime(int localClientNum);
void __cdecl Demo_WriteConfigStrings(int localClientNum, msg_t *msg);
void __cdecl Demo_ReadConfigStrings(int localClientNum, msg_t *msg);
void __cdecl Demo_UpdateConfigStrings(int localClientNum);
bool __cdecl Demo_ShouldGenerateKeyFrameSnapshot(int localClientNum, int serverTime);
void __cdecl Demo_GenerateKeyFrameSnapshot(
                int localClientNum,
                int msgSequence,
                int cmdSequence,
                int serverTime,
                int physicsTime,
                int snapFlags);
void __cdecl Demo_WriteKeyFrameInformation(int localClientNum);
void __cdecl Demo_InitClipRecord(int localClientNum, bool newClip);
void __cdecl Demo_WriteUncompressedClipSnapshot(int localClientNum);
void __cdecl Demo_WriteClipCommands(int localClientNum, msg_t *msg, bool setClipRecordParams);
void __cdecl Demo_WriteClipCommandsInternal(int localClientNum, msg_t *msg, bool setClipRecordParams);
void __cdecl Demo_WriteEndOfFrameClipCommand(msg_t *msg);
void __cdecl Demo_WriteClipCommandsWhenPaused(int localClientNum);
int __cdecl Demo_WriteTransitionClipCommandBlockToBuffer(
                unsigned __int8 transition,
                unsigned __int8 *buffer,
                int maxBufferSize);
unsigned __int8 __cdecl Demo_ParseClipCommand(int localClientNum, msg_t *msg, bool setClipPlaybackParams);
void __cdecl Demo_WriteClipGameState(int localClientNum);
bool __cdecl Demo_ShouldFlushClipRecordMessage();
void __cdecl Demo_SaveClipRecordMessageToMainMemory(
                int localClientNum,
                unsigned __int8 *data,
                unsigned int size,
                bool forceWrite);
void __cdecl Demo_MoveToNextPhase();
void __cdecl Demo_SetupClipStreamingState();
void __cdecl Demo_PopulateClipPlayerTags(demoMetaInfo *info);
int __cdecl Demo_SendClipToStreamingBuffer(unsigned __int8 *data, int dataSize, unsigned int bytesUploaded);
void __cdecl Demo_UploadClipSuccess(int controllerIndex, unsigned __int64 fileID);
void __cdecl Demo_UploadClipFailure(int controllerIndex);
void __cdecl Demo_PopulateHeliPatches(demoMetaInfo *info);
void __cdecl Demo_WriteRecordedClip(int localClientNum, unsigned __int16 slot);
char *__cdecl Demo_AdjustTimeForConfigString(int configStringIndex, const char *string);
const char *__cdecl Demo_AdjustTimeForReliableCommand(const char *cmd);
void __cdecl Demo_AdjustTimeForMatchState(int currentTime, int clipTime, MatchState *matchstate);
void __cdecl Demo_AdjustTimeForPlayerState(int currentTime, int clipTime, playerState_s *ps);
void __cdecl Demo_AdjustTimeForPlayerStateHudElement(int currentTime, int clipTime, int index, hudelem_s *hudElem);
void __cdecl Demo_AdjustTimeForEntityState(int currentTime, int clipTime, entityState_s *state);
void __cdecl Demo_WriteDemoPreviewPoint(int localClientNum, int restorePoint);
void __cdecl Demo_RestoreDemoPreviewPoint(int localClientNum, int restorePoint);
void __cdecl Demo_RestoreUIStateAfterPreview(int localClientNum);
void __cdecl Demo_HandleInput(int localClientNum, int controllerIndex, int key, int pressed);
void __cdecl Demo_HandleInputForControllerConfig(int localClientNum, int controllerIndex, int key, int pressed);
void __cdecl Demo_SetButtonPressTime(unsigned int demoControlButtonType);
void __cdecl Demo_ActivateHoldInput(int localClientNum, int key);
void __cdecl Demo_HandleHoldInput(int localClientNum, int key, bool holdResult);
