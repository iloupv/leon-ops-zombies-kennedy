#pragma once

struct UiContext;

void __cdecl UI_AnimateLocalItem(
                int localClientNum,
                UiContext *dc,
                const char *menuName,
                const char *itemName,
                const char *newStateName,
                int milliseconds);
void __cdecl UI_SetLocalVarStringByName(char *inputName, char *inputValue, int localClientNum);
inline void __cdecl UI_SetLocalVarStringByName(const char *inputName, char *inputValue, int localClientNum)
{
    UI_SetLocalVarStringByName((char *)inputName, inputValue, localClientNum);
}
void __cdecl UI_SetFakeLocalDvarStringByName(const char *inputName, char *inputValue, unsigned int localClientNum);
void __cdecl CG_ClearPopUpUI(int localClientNum);
void __cdecl CG_ClearCenterPopUpUI(int localClientNum);
void __cdecl CG_ClearPlayerCardPopUpUI(int localClientNum);
void __cdecl CG_TeamMessagePopUpUI(int localClientNum);
void __cdecl CG_GameMessagePopUpUI(int localClientNum);
void __cdecl CG_DisplayCenterPopup(
                int localClientNum,
                const char *animationType,
                char *displayString1,
                char *displayString2,
                char *displayString3,
                char *displayMaterial,
                char *displayBacking);
void __cdecl CG_PlayPopUpSound(int localClientNum, char *sound);
void __cdecl CG_DisplayRankUp(int localClientNum);
void __cdecl CG_DisplayMilestoneComplete(int localClientNum);
void __cdecl CG_SetupEndGame(int localClientNum);
void __cdecl CG_SetupMilestoneEndGame(int localClientNum);
void __cdecl CG_ClearEndGame(int localClientNum);
void __cdecl CG_DisplayGameModeMessage(int localClientNum);
void __cdecl CG_DisplayTeamMessage(unsigned int localClientNum);
void __cdecl CG_DisplayWagerMedal(int localClientNum);
void __cdecl CG_DisplayMedal(int localClientNum);
void __cdecl CG_DisplayContract(int localClientNum);
void __cdecl CG_DisplayKillstreak(int localClientNum);
void __cdecl CG_DisplayWagerPopup(int localClientNum);
void __cdecl CG_DisplayHudAnim(int localClientNum);
