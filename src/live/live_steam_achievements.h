#pragma once

#include <steam/steam_api.h>

struct Achievement_t // sizeof=0x190
{                                       // XREF: .data:Achievement_t * g_Achievements/r
    int m_eAchievementID;
    const char *m_pchAchievementID;
    char m_rgchName[128];
    char m_rgchDescription[256];
    bool m_bAchieved;
    // padding byte
    // padding byte
    // padding byte
    int m_iIconImage;
};

struct CSteamAchievements // sizeof=0x58
{
    bool m_needStatsStored;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    unsigned __int64 m_iAppID;
    Achievement_t *m_pAchievements;
    int m_iNumAchievements;
    bool m_bInitialized;
    // padding byte
    // padding byte
    // padding byte
    CCallback<CSteamAchievements,UserStatsReceived_t,0> m_CallbackUserStatsReceived;
    CCallback<CSteamAchievements,UserStatsStored_t,0> m_CallbackUserStatsStored;
    CCallback<CSteamAchievements,UserAchievementStored_t,0> m_CallbackAchievementStored;

    CSteamAchievements(Achievement_t *Achievements, int NumAchievements);
    ~CSteamAchievements();


    bool RequestStats();
    void SetAchievement(const char *ID);
    void StoreStats();
    void ResetStats();
    void OnUserStatsReceived(UserStatsReceived_t *pCallback);
    void OnUserStatsStored(UserStatsStored_t *pCallback);
    void OnAchievementStored(UserAchievementStored_t *pCallback);
};




void __cdecl LiveSteam_Achievements_Init();
void __cdecl LiveSteam_GiveAchievement_f();
void __cdecl LiveSteam_ResetAchievement_f();
void __cdecl LiveSteam_Achievements_Shutdown();
void __cdecl LiveSteam_Achievements_Frame();
void __cdecl LiveSteam_GiveAchievement(const char *achievementName);

