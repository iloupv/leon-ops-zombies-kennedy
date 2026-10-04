#pragma once

enum profileWriteState_t : __int32
{                                       // XREF: ?GamerProfile_UpdateProfileFromDvars@@YAXHW4profileWriteState_t@@@Z/r
    PROFILE_NO_WRITE         = 0x0,
    PROFILE_WRITE_IF_CHANGED = 0x1,
};

struct __declspec(align(2)) GamerSettingState // sizeof=0xCC4
{                                       // XREF: .data:gamerSettings/r
                                        // ?GamerProfile_UpdateProfileFromDvars@@YAXHW4profileWriteState_t@@@Z/r
    bool isProfileLoggedIn;             // XREF: GamerProfile_IsProfileLoggedIn(int)+42/r
                                        // GamerProfile_UpdateProfileFromDvars(int,profileWriteState_t)+723/r ...
    bool isInitialized;                 // XREF: GamerProfile_UpdateDvarsFromProfile(int)+BB/r
                                        // GamerProfile_UpdateProfileFromDvars(int,profileWriteState_t)+7FB/w
    bool errorOnRead;                   // XREF: GamerProfile_WasErrorOnRead(int)+42/r
                                        // GamerProfile_UpdateProfileFromDvars(int,profileWriteState_t)+737/r ...
    // padding byte
    int firstTime;                      // XREF: GamerProfile_UpdateDvarsFromProfile(int)+F5/r
                                        // GamerProfile_UpdateProfileFromDvars(int,profileWriteState_t)+146/w
    bool zombietron_discovered;         // XREF: GamerProfile_UpdateDvarsFromProfile(int)+114/r
                                        // GamerProfile_UpdateProfileFromDvars(int,profileWriteState_t)+15D/w
    bool zombiefive_discovered;         // XREF: GamerProfile_UpdateDvarsFromProfile(int)+134/r
                                        // GamerProfile_UpdateProfileFromDvars(int,profileWriteState_t)+174/w
    bool invertPitch;                   // XREF: GamerProfile_UpdateDvarsFromProfile(int)+174/r
                                        // GamerProfile_UpdateProfileFromDvars(int,profileWriteState_t)+1A2/w
    // padding byte
    float viewSensitivity;
    bool autoAim;
    bool gpadEnabled;                   // XREF: GamerProfile_ExecControllerBindings(int)+4A/r
                                        // GamerProfile_UpdateDvarsFromProfile(int)+154/r ...
    // padding byte
    // padding byte
    int team_indicator;                 // XREF: GamerProfile_UpdateDvarsFromProfile(int)+1B5/r
                                        // GamerProfile_UpdateProfileFromDvars(int,profileWriteState_t)+1D1/w
    bool mature_content;                // XREF: GamerProfile_UpdateDvarsFromProfile(int)+1D3/r
                                        // GamerProfile_UpdateProfileFromDvars(int,profileWriteState_t)+1E9/w
    bool show_blood;                    // XREF: GamerProfile_UpdateDvarsFromProfile(int)+1F2/r
                                        // GamerProfile_UpdateProfileFromDvars(int,profileWriteState_t)+201/w
    bool targetAssist;
    bool lastSpectatorSelectedThirdPerson;
                                        // XREF: GamerProfile_UpdateDvarsFromProfile(int)+581/r
                                        // GamerProfile_UpdateProfileFromDvars(int,profileWriteState_t)+582/w
    char gpadButtonsConfig[256];        // XREF: GamerProfile_ExecControllerBindings(int)+62/r
                                        // GamerProfile_ExecControllerBindings(int)+76/o ...
    char gpadSticksConfig[256];         // XREF: GamerProfile_ExecControllerBindings(int)+D3/r
                                        // GamerProfile_ExecControllerBindings(int)+E7/o ...
    float gpadButtonLStickDeflect;
    float gpadButtonRStickDeflect;
    float gamma;
    float snd_menu_voice;
    float snd_menu_music;
    float snd_menu_sfx;
    float snd_menu_master;
    float snd_cinematic_volume;
    int snd_menu_speaker_setup;         // XREF: GamerProfile_UpdateDvarsFromProfile(int)+370/r
                                        // GamerProfile_UpdateProfileFromDvars(int,profileWriteState_t)+40E/w
    int snd_menu_listen_level;          // XREF: GamerProfile_UpdateDvarsFromProfile(int)+38F/r
                                        // GamerProfile_UpdateProfileFromDvars(int,profileWriteState_t)+426/w
    float snd_menu_left_channel;
    float snd_menu_right_channel;
    float snd_menu_center_channel;
    float snd_menu_left_surround;
    float snd_menu_right_surround;
    float snd_menu_lfe;
    float safeAreaHorizontal;
    float safeAreaVertical;
    int demoControllerConfig;           // XREF: GamerProfile_UpdateDvarsFromProfile(int)+535/r
                                        // GamerProfile_UpdateProfileFromDvars(int,profileWriteState_t)+32D/w
    bool basicTrainingPopup;            // XREF: GamerProfile_UpdateDvarsFromProfile(int)+478/r
                                        // GamerProfile_UpdateProfileFromDvars(int,profileWriteState_t)+4CC/w
    // padding byte
    // padding byte
    // padding byte
    int friends;                        // XREF: GamerProfile_UpdateDvarsFromProfile(int)+498/r
                                        // GamerProfile_UpdateProfileFromDvars(int,profileWriteState_t)+4E4/w
    int enemies;                        // XREF: GamerProfile_UpdateDvarsFromProfile(int)+4B7/r
                                        // GamerProfile_UpdateProfileFromDvars(int,profileWriteState_t)+4FC/w
    int difficulty;                     // XREF: GamerProfile_UpdateDvarsFromProfile(int)+4D6/r
                                        // GamerProfile_UpdateProfileFromDvars(int,profileWriteState_t)+514/w
    bool tips;                          // XREF: GamerProfile_UpdateDvarsFromProfile(int)+4F5/r
                                        // GamerProfile_UpdateProfileFromDvars(int,profileWriteState_t)+52C/w
    bool gamertags;                     // XREF: GamerProfile_UpdateDvarsFromProfile(int)+515/r
                                        // GamerProfile_UpdateProfileFromDvars(int,profileWriteState_t)+544/w
    char exeSettingsBuffer[1000];       // XREF: GamerProfile_CreateExeBindingsConfig+A6/o
                                        // GamerProfile_CreateExeBindingsConfig+C6/o ...
    char commonSettingsBuffer[1000];    // XREF: GamerProfile_CreateCommonBindingsConfig+8C/o
                                        // GamerProfile_CreateCommonBindingsConfig+AD/o ...
    char motd[256];                     // XREF: GamerProfile_UpdateDvarsFromProfile(int)+563/o
                                        // GamerProfile_UpdateProfileFromDvars(int,profileWriteState_t)+561/o
    bool mapPrefs[16];
    // padding byte
    // padding byte
    int playlist;                       // XREF: GamerProfile_GetPlaylistNum(int)+C/r
                                        // GamerProfile_SetPlaylistNum(int,int)+F/w ...
    int category;                       // XREF: GamerProfile_UpdateDvarsFromProfile(int)+5C0/r
                                        // GamerProfile_UpdateProfileFromDvars(int,profileWriteState_t)+5B2/w
    int categoryPlaylist;               // XREF: GamerProfile_UpdateDvarsFromProfile(int)+5DF/r
                                        // GamerProfile_UpdateProfileFromDvars(int,profileWriteState_t)+5CA/w
    int wagerCategory;                  // XREF: GamerProfile_UpdateDvarsFromProfile(int)+5FE/r
                                        // GamerProfile_UpdateProfileFromDvars(int,profileWriteState_t)+5E2/w
    int wagerCategoryPlaylist;          // XREF: GamerProfile_UpdateDvarsFromProfile(int)+61D/r
                                        // GamerProfile_UpdateProfileFromDvars(int,profileWriteState_t)+5FA/w
    char clanPrefix[5];                 // XREF: GamerProfile_UpdateDvarsFromProfile(int)+6EE/o
                                        // GamerProfile_UpdateDvarsFromProfile(int)+721/o ...
    char customClass[10][16];           // XREF: GamerProfile_UpdateDvarsFromProfile(int)+641/r
                                        // GamerProfile_UpdateDvarsFromProfile(int)+68F/o ...
    bool loadoutDirty;                  // XREF: GamerProfile_UpdateProfileFromDvars(int,profileWriteState_t)+69A/r
                                        // GamerProfile_UpdateProfileFromDvars(int,profileWriteState_t)+77E/w
    unsigned __int8 classLoadouts[5][34];
                                        // XREF: GamerProfile_CreateExeBindingsConfig+124/o
    unsigned __int8 killstreaks[3];     // XREF: GamerProfile_CreateExeBindingsConfig+151/o
    // padding byte
};

void __cdecl SetDvarFromLocString_0(int controllerIndex, const char *dvarName, char *preLocalizedText);

//void __cdecl SetDvarFromLocString(int controllerIndex, const char *dvarName, char *preLocalizedText);
GamerSettingState *__cdecl GamerProfile_GetProfileSettings(int controllerIndex);
bool __cdecl GamerProfile_IsProfileLoggedIn(int controllerIndex);
bool __cdecl GamerProfile_WasErrorOnRead(int controllerIndex);
int __cdecl GamerProfile_GetPlaylistNum(int controllerIndex);
void __cdecl GamerProfile_SetPlaylistNum(int controllerIndex, int playlistNum);
void __cdecl GamerProfile_ExecControllerBindings(int controllerIndex);
void __cdecl GamerProfile_UpdateDvarsFromProfile(int controllerIndex);
void __cdecl DebugReportProfileDVars(const char *headerMsg);
void __cdecl GamerProfile_UpdateProfileFromDvars(int controllerIndex, profileWriteState_t profileWriteState);
void __cdecl GamerProfile_SaveProfile(int controllerIndex);
void __cdecl GamerProfile_CreateExeBindingsConfig(int controllerIndex);
void __cdecl GamerProfile_AppendDvarListToConfig(char *buffer, unsigned int size, const char **dvarNameList);
void __cdecl GamerProfile_AppendByteListToConfig(
                char *buffer,
                unsigned int bufSize,
                unsigned __int8 *data,
                unsigned __int16 dataSize);
void __cdecl GamerProfile_CreateCommonBindingsConfig(int controllerIndex);
bool __cdecl GamerProfile_IsNewLocalProfile(int controllerIndex);
void __cdecl GamerProfile_InitAllProfiles();
void __cdecl GamerProfile_LogInProfile(int controllerIndex);
void __cdecl GamerProfile_LogOutProfile(int controllerIndex);
