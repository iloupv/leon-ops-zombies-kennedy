#include "live_steam_achievements.h"
#include <qcommon/common.h>
#include <qcommon/cmd.h>

Achievement_t g_Achievements[3] =
{
  { 0, "MP_FILM_CREATED", "", "", false, 0 },
  { 1, "MP_WAGER_MATCH", "", "", false, 0 },
  { 2, "MP_PLAY", "", "", false, 0 }
};



cmd_function_s LiveSteam_GiveAchievement_f_VAR;
cmd_function_s LiveSteam_ResetAchievement_f_VAR;

CSteamAchievements *g_liveSteamAchievements;

CSteamAchievements::CSteamAchievements(
    Achievement_t *Achievements,
    int NumAchievements)
    : m_needStatsStored(true),
    m_iAppID(0),
    m_bInitialized(false),
    m_CallbackUserStatsReceived(this, &CSteamAchievements::OnUserStatsReceived),
    m_CallbackUserStatsStored(this, &CSteamAchievements::OnUserStatsStored),
    m_CallbackAchievementStored(this, &CSteamAchievements::OnAchievementStored),
    m_pAchievements(Achievements),
    m_iNumAchievements(NumAchievements)
{
    if (SteamUtils())
        m_iAppID = SteamUtils()->GetAppID();

    RequestStats();
}


CSteamAchievements::~CSteamAchievements()
{

}

bool CSteamAchievements::RequestStats()
{
#if 0
    ISteamUserStats *v2; // eax
    ISteamUser *v3; // [esp+4h] [ebp-8h]

    if (!SteamUserStats() || !SteamUser())
        return 0;
    v3 = SteamUser();
    if (!v3->BLoggedOn())
        return 0;
    v2 = SteamUserStats();
    return (v2->RequestCurrentStats)();
#else
    return 0;
#endif
}

void CSteamAchievements::SetAchievement(const char *ID)
{
    ISteamUserStats *v2; // [esp+0h] [ebp-8h]

    if (this->m_bInitialized)
    {
        v2 = SteamUserStats();
        v2->SetAchievement(ID);
        this->m_needStatsStored = 1;
    }
}

void __thiscall CSteamAchievements::StoreStats()
{
    ISteamUserStats *v1; // eax

    if (this->m_needStatsStored && SteamUserStats()) // no-steam: no Steam client, nothing to store
    {
        v1 = SteamUserStats();
        this->m_needStatsStored = (v1->StoreStats)() == 0;
    }
}

void __thiscall CSteamAchievements::ResetStats()
{
    ISteamUserStats *v1; // [esp+0h] [ebp-Ch]

    v1 = SteamUserStats();
    if (!v1) // no-steam
        return;
    if (v1->ResetAllStats(1))
        Com_DPrintf(23, "STEAM: Resetting achievements\n");
    else
        Com_DPrintf(23, "STEAM: Unable to reset achievements\n");
    this->m_needStatsStored = 1;
}

void __thiscall CSteamAchievements::OnUserStatsReceived(UserStatsReceived_t *pCallback)
{
    const char *v2; // eax
    const char *v3; // eax
    ISteamUserStats *v4; // [esp+0h] [ebp-20h]
    ISteamUserStats *v5; // [esp+4h] [ebp-1Ch]
    ISteamUserStats *v6; // [esp+8h] [ebp-18h]
    Achievement_t *ach; // [esp+18h] [ebp-8h]
    int iAch; // [esp+1Ch] [ebp-4h]

    if (this->m_iAppID == pCallback->m_nGameID)
    {
        if (pCallback->m_eResult == k_EResultOK)
        {
            Com_DPrintf(23, "STEAM: Received stats and achievements from Steam\n");
            this->m_bInitialized = 1;
            for (iAch = 0; iAch < this->m_iNumAchievements; ++iAch)
            {
                ach = &this->m_pAchievements[iAch];
                v6 = SteamUserStats();
                v6->GetAchievement(ach->m_pchAchievementID, &ach->m_bAchieved);
                v5 = SteamUserStats();
                v2 = v5->GetAchievementDisplayAttribute(ach->m_pchAchievementID, "name");
                _snprintf(ach->m_rgchName, 0x80u, "%s", v2);
                v4 = SteamUserStats();
                v3 = v4->GetAchievementDisplayAttribute(ach->m_pchAchievementID, "desc");
                _snprintf(ach->m_rgchDescription, 0x100u, "%s", v3);
            }
        }
        else
        {
            Com_DPrintf(23, "STEAM: RequestStats - failed, %d\n", pCallback->m_eResult);
        }
    }
}

void __thiscall CSteamAchievements::OnUserStatsStored(UserStatsStored_t *pCallback)
{
    if (this->m_iAppID == pCallback->m_nGameID)
    {
        if ( pCallback->m_eResult == k_EResultOK )
            Com_DPrintf(23, "STEAM: Stored stats for Steam\n");
        else
            Com_DPrintf(23, "STEAM: StatsStored - failed, %d\n", pCallback->m_eResult);
    }
}

void __thiscall CSteamAchievements::OnAchievementStored(UserAchievementStored_t *pCallback)
{
    if (this->m_iAppID == pCallback->m_nGameID)
    {
        Com_DPrintf(23, "STEAM: Stored Achievement for Steam\n");
    }
}

void __cdecl LiveSteam_Achievements_Init()
{
    g_liveSteamAchievements = new CSteamAchievements(g_Achievements, 3);

    Cmd_AddCommandInternal("livesteam_giveachivement", LiveSteam_GiveAchievement_f, &LiveSteam_GiveAchievement_f_VAR);
    Cmd_AddCommandInternal("livesteam_resetachivement", LiveSteam_ResetAchievement_f, &LiveSteam_ResetAchievement_f_VAR);
}

void __cdecl LiveSteam_GiveAchievement_f()
{
    const char *v0; // eax
    const char *v1; // eax

    if ( Cmd_Argc() != 2 )
    {
        v0 = Cmd_Argv(0);
        Com_Printf(0, "Usage: %s <achievement name>\n", v0);
    }
    v1 = Cmd_Argv(1);
    LiveSteam_GiveAchievement(v1);
}

void __cdecl LiveSteam_ResetAchievement_f()
{
    //CSteamAchievements::ResetStats(g_liveSteamAchievements);
    g_liveSteamAchievements->ResetStats();
}

void __cdecl LiveSteam_Achievements_Shutdown()
{
    CSteamAchievements *v0; // [esp+8h] [ebp-4h]

    if ( g_liveSteamAchievements )
    {
        //v0 = g_liveSteamAchievements;
        //CSteamAchievements::~CSteamAchievements(g_liveSteamAchievements);
        //operator delete(v0);
        delete g_liveSteamAchievements;
        g_liveSteamAchievements = NULL;
    }
}

void __cdecl LiveSteam_Achievements_Frame()
{
    if ( g_liveSteamAchievements )
        g_liveSteamAchievements->StoreStats();
}

void __cdecl LiveSteam_GiveAchievement(const char *achievementName)
{
    if ( g_liveSteamAchievements )
        g_liveSteamAchievements->SetAchievement(achievementName);
}