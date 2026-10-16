// mod: runtime sounds (see snd_runtime.h). The WAVs become "loaded" (in-memory PCM) aliases in a sound bank of our own,
// registered with SND_AddBank as a fastfile's bank is (Load_SndBankAsset); the sound thread finds them by the alias
// hash like any other, newest bank first. Each alias copies its settings (group, curves, bus, priorities) from a
// 2D loaded alias of the level's banks, then takes the file, a fixed pitch and full volume.
#include "snd_runtime.h"
#include "snd.h"
#include "snd_bank.h"
#include "snd_utils.h"
#include <universal/com_files.h>
#include <universal/q_shared.h>
#include <cgame/cg_sound.h>
#include <cgame_mp/cg_main_mp.h>
#include <qcommon/common.h>
#include <win32/win_shared.h>
#include "snd_public_async.h"

#include <string>
#include <vector>

struct SndRuntimeVariant
{
    std::string name;
    std::vector<char> pcm;
    LoadedSound loaded;
    SoundFile file;
};

struct SndRuntimeAlias
{
    std::string name;
    std::vector<SndRuntimeVariant *> variants;
    std::vector<snd_alias_t> aliases;
};

static std::vector<SndRuntimeAlias *> s_runtimeAliases;
static std::vector<snd_alias_list_t> s_runtimeLists;
static std::vector<SndIndexEntry> s_runtimeIndex;
static SndBank s_runtimeBank;
static bool s_runtimeBankAdded;

static void SND_Runtime_Free()
{
    if ( s_runtimeBankAdded )
    {
        SND_RemoveBank(&s_runtimeBank);
        s_runtimeBankAdded = false;
    }
    for ( SndRuntimeAlias *alias : s_runtimeAliases )
    {
        for ( SndRuntimeVariant *variant : alias->variants )
            delete variant;
        delete alias;
    }
    s_runtimeAliases.clear();
    s_runtimeLists.clear();
    s_runtimeIndex.clear();
}

static unsigned int SND_Runtime_Read32(const unsigned char *p)
{
    return p[0] | (p[1] << 8) | (p[2] << 16) | ((unsigned int)p[3] << 24);
}

// the PCM of a RIFF WAVE file: 16-bit, 1 or 2 channels
static bool SND_Runtime_ParseWav(const unsigned char *data, int size, int *channels, int *rate, const unsigned char **pcm,
                                 int *pcmSize)
{
    if ( size < 12 || memcmp(data, "RIFF", 4) || memcmp(data + 8, "WAVE", 4) )
        return false;
    *channels = 0;
    *pcm = 0;
    for ( int o = 12; o + 8 <= size; )
    {
        const unsigned int len = SND_Runtime_Read32(data + o + 4);
        if ( o + 8 + (int)len > size )
            break;
        if ( !memcmp(data + o, "fmt ", 4) && len >= 16 )
        {
            const int tag = data[o + 8] | (data[o + 9] << 8);
            const int bits = data[o + 22] | (data[o + 23] << 8);
            if ( tag != 1 || bits != 16 )
                return false;
            *channels = data[o + 10] | (data[o + 11] << 8);
            *rate = (int)SND_Runtime_Read32(data + o + 12);
        }
        else if ( !memcmp(data + o, "data", 4) )
        {
            *pcm = data + o + 8;
            *pcmSize = (int)len;
        }
        o += 8 + (int)((len + 1) & ~1u);
    }
    return *pcm && (*channels == 1 || *channels == 2) && *rate > 0;
}

// the level's first loaded, unspatialized alias: the settings the runtime ones copy
static const snd_alias_t *SND_Runtime_Template()
{
    const unsigned int count = SND_AliasCount();
    for ( unsigned int i = 0; i < count; ++i )
    {
        const snd_alias_list_t *list = SND_AliasByIndex(i);
        if ( !list || !list->head || !list->count )
            continue;
        const snd_alias_t *alias = list->head;
        if ( ((alias->flags >> 14) & 3) == 1 && !(alias->flags & 2) && alias->soundFile && alias->soundFile->type == 1
             && alias->dryCurve != 0xFF )
            return alias;
    }
    return 0;
}

// TEST SWITCH bo1_aliasdump "<word> <word>...": the level's aliases whose names hold one of the words, with their
// volume, group, distances and flags (the level's sound data, for setting levels)
static void SND_Runtime_DumpAliases()
{
    const dvar_s *dump = Dvar_FindVar("bo1_aliasdump");
    if ( !dump || dump->type != DVAR_TYPE_STRING || !dump->current.string || !dump->current.string[0] )
        return;
    char words[256];
    I_strncpyz(words, dump->current.string, sizeof(words));
    std::vector<const char *> list;
    for ( char *w = strtok(words, " "); w; w = strtok(0, " ") )
        list.push_back(w);
    const unsigned int count = SND_AliasCount();
    for ( unsigned int i = 0; i < count; ++i )
    {
        const snd_alias_list_t *alias = SND_AliasByIndex(i);
        if ( !alias || !alias->name || !alias->head )
            continue;
        bool match = false;
        for ( const char *w : list )
            match |= I_stristr(alias->name, w) != 0;
        if ( !match )
            continue;
        const snd_alias_t *a = alias->head;
        Com_Printf(0, "bo1_aliasdump: %s x%d vol %.2f-%.2f group %u dist %u-%u %s %s type %u snapshot %u limit %u\n",
            alias->name, alias->count, a->volMin / 65535.0f, a->volMax / 65535.0f, (a->flags >> 16) & 0x3F, a->distMin,
            a->distMax, (a->flags & 2) ? "3d" : "2d", (a->flags & 1) ? "loop" : "once", (a->flags >> 14) & 3,
            a->snapshotGroup, a->limitCount);
    }
}

void __cdecl SND_Runtime_LoadForLevel()
{
    SND_Runtime_Free();
    if ( !SND_Active() )
        return;
    SND_Runtime_DumpAliases();
    int fileCount = 0;
    const char **files = FS_ListFiles("sound/rt", (char *)"wav", FS_LIST_ALL, &fileCount);
    if ( !files || !fileCount )
    {
        if ( files )
            FS_FreeFileList(files);
        return;
    }
    const snd_alias_t *templ = SND_Runtime_Template();
    if ( !templ )
    {
        Com_PrintWarning(0, "runtime sounds: no 2D loaded alias on this level to copy\n");
        FS_FreeFileList(files);
        return;
    }
    for ( int f = 0; f < fileCount; ++f )
    {
        void *file = 0;
        const int size = FS_ReadFile(va("sound/rt/%s", files[f]), &file);
        if ( size <= 0 || !file )
            continue;
        int channels, rate, pcmSize;
        const unsigned char *pcm;
        if ( !SND_Runtime_ParseWav((const unsigned char *)file, size, &channels, &rate, &pcm, &pcmSize) )
        {
            Com_PrintWarning(0, "runtime sounds: %s is not a 16-bit mono / stereo PCM WAV\n", files[f]);
            FS_FreeFile(file);
            continue;
        }
        // the alias: the stem without its extension and a trailing _<number>
        std::string stem = files[f];
        const size_t dot = stem.rfind('.');
        if ( dot != std::string::npos )
            stem.resize(dot);
        std::string aliasName = stem;
        const size_t under = aliasName.rfind('_');
        if ( under != std::string::npos && under + 1 < aliasName.size()
             && aliasName.find_first_not_of("0123456789", under + 1) == std::string::npos )
            aliasName.resize(under);
        for ( char &c : aliasName )
            c = (char)tolower((unsigned char)c);

        SndRuntimeVariant *variant = new SndRuntimeVariant;
        variant->name = stem;
        variant->pcm.assign((const char *)pcm, (const char *)pcm + pcmSize);
        FS_FreeFile(file);
        memset(&variant->loaded, 0, sizeof(variant->loaded));
        variant->loaded.name = variant->name.c_str();
        snd_asset &asset = variant->loaded.sound;
        asset.format = SND_ASSET_FORMAT_PCMS16;
        asset.channel_count = channels;
        asset.frame_rate = rate;
        asset.frame_count = pcmSize / (channels * 2);
        asset.data_size = pcmSize;
        asset.data = variant->pcm.data();
        memset(&variant->file, 0, sizeof(variant->file));
        variant->file.u.loadSnd = &variant->loaded;
        variant->file.type = 1;
        variant->file.exists = 1;

        SndRuntimeAlias *alias = 0;
        for ( SndRuntimeAlias *a : s_runtimeAliases )
            if ( a->name == aliasName )
                alias = a;
        if ( !alias )
        {
            alias = new SndRuntimeAlias;
            alias->name = aliasName;
            s_runtimeAliases.push_back(alias);
        }
        alias->variants.push_back(variant);
    }
    FS_FreeFileList(files);
    if ( s_runtimeAliases.empty() )
        return;

    // the aliases (one per variant, sharing the name) and the bank's lists
    const unsigned int count = (unsigned int)s_runtimeAliases.size();
    s_runtimeLists.resize(count);
    for ( unsigned int i = 0; i < count; ++i )
    {
        SndRuntimeAlias *alias = s_runtimeAliases[i];
        const unsigned int id = (unsigned int)SND_HashName(alias->name.c_str());
        alias->aliases.resize(alias->variants.size());
        for ( size_t v = 0; v < alias->variants.size(); ++v )
        {
            snd_alias_t &a = alias->aliases[v];
            a = *templ;
            a.name = alias->name.c_str();
            a.id = id;
            a.subtitle = 0;
            a.secondaryname = 0;
            a.soundFile = &alias->variants[v]->file;
            a.flags = (a.flags & ~0xC003u) | (1u << 14); // loaded, not looping, 2D
            a.contextType = 0;
            a.contextValue = 0;
            a.startDelay = 0;
            a.probability = 0;
            a.pitchMin = a.pitchMax = 32767;
            a.volMin = a.volMax = 65535;
            // the kick's grunt (leon_kicka / leon_kickb, and the other characters': ada_kicka...) in the voice group
            // (2, full) at 0.6: in the template's (the ui group's 0.32, as his pain and death) it was too quiet against
            // the fight
            const size_t len = alias->name.size();
            if ( len > 6 && (!_stricmp(alias->name.c_str() + len - 6, "_kicka")
                             || !_stricmp(alias->name.c_str() + len - 6, "_kickb")) )
            {
                a.flags = (a.flags & ~0x3F0000u) | (2u << 16);
                a.volMin = a.volMax = (unsigned __int16)(0.6f * 65535.0f);
            }
        }
        snd_alias_list_t &list = s_runtimeLists[i];
        memset(&list, 0, sizeof(list));
        list.name = alias->name.c_str();
        list.id = id;
        list.head = alias->aliases.data();
        list.count = (int)alias->aliases.size();
    }
    // the hash index (SND_FindInIndex): each list at slot id % count when free, else a free slot chained from there
    s_runtimeIndex.assign(count, SndIndexEntry{ 0xFFFF, 0xFFFF });
    std::vector<unsigned int> pending;
    for ( unsigned int i = 0; i < count; ++i )
    {
        SndIndexEntry &slot = s_runtimeIndex[s_runtimeLists[i].id % count];
        if ( slot.value == 0xFFFF )
            slot.value = (unsigned __int16)i;
        else
            pending.push_back(i);
    }
    for ( unsigned int i : pending )
    {
        unsigned int free = 0;
        while ( s_runtimeIndex[free].value != 0xFFFF )
            ++free;
        s_runtimeIndex[free].value = (unsigned __int16)i;
        unsigned int tail = s_runtimeLists[i].id % count;
        while ( s_runtimeIndex[tail].next != 0xFFFF )
            tail = s_runtimeIndex[tail].next;
        s_runtimeIndex[tail].next = (unsigned __int16)free;
    }
    memset(&s_runtimeBank, 0, sizeof(s_runtimeBank));
    s_runtimeBank.name = "mod_runtime_sounds";
    s_runtimeBank.aliasCount = count;
    s_runtimeBank.alias = s_runtimeLists.data();
    s_runtimeBank.aliasIndex = s_runtimeIndex.data();
    SND_AddBank(&s_runtimeBank);
    s_runtimeBankAdded = true;
    for ( SndRuntimeAlias *alias : s_runtimeAliases )
        Com_Printf(0, "runtime sounds: '%s' (%u variants), settings from '%s'\n", alias->name.c_str(),
                   (unsigned int)alias->variants.size(), templ->name);
}

// Under Leon (his runtime voice loaded): the zombies characters' lines (vox_plr_*) muted, and the player's breathing -
// the characters' (chr_breathing_*: the gasp at a death, the hurt / better breaths; at a third it still blared out on a
// near death and its recovery, full scale in its group), the sprint's gasp and the sniper's held breath (in / out /
// gasp, the heartbeat): RE4's Leon breathes none of them
float __cdecl SND_Runtime_VoiceScale(unsigned int id)
{
    struct Answer
    {
        unsigned int id;
        float scale;
    };
    static std::vector<Answer> s_answers; // per alias id
    if ( !id || !SND_Active() )
        return 1.0f;
    bool leon = false;
    for ( const SndRuntimeAlias *a : s_runtimeAliases )
        leon |= a->name == "leon_pain";
    if ( !leon )
        return 1.0f;
    for ( const Answer &answer : s_answers )
        if ( answer.id == id )
            return answer.scale;
    const snd_alias_list_t *list = SND_FindAliasFromId(id);
    float scale = 1.0f;
    if ( list && list->name && !_strnicmp(list->name, "vox_plr_", 8) )
        scale = 0.0f;
    else if ( list && list->name
              && (!_strnicmp(list->name, "chr_breathing", 13) || !_strnicmp(list->name, "chr_sprint_gasp", 15)
                  || !_strnicmp(list->name, "fly_sprint_gasp", 15) || !_strnicmp(list->name, "wpn_sniper_breath", 17)
                  || !_strnicmp(list->name, "wpn_sniper_heartbeat", 20)) )
        scale = 0.0f;
    // a gun's first raise (wpn_<gun>_1straise_plr, BO1's handling on a new gun): RE4's pickup sounds in its place
    // (maps\leon\_pickup)
    else if ( list && list->name && !_strnicmp(list->name, "wpn_", 4) && I_stristr(list->name, "_1straise") )
        scale = 0.0f;
    // George Romero (Call of the Dead's boss, "the director"): his effects and voice at a quarter (-12 dB; at half he
    // still drowned everything) - every one at volume 1 in the voice / event group, full out to 800 / 1000 units
    else if ( list && list->name
              && (!_strnicmp(list->name, "zmb_director_", 13) || !_strnicmp(list->name, "vox_romero_", 11)
                  || !_strnicmp(list->name, "vox_director_", 13)) )
        scale = 0.25f;
    s_answers.push_back({ id, scale });
    return scale;
}

// Under Leon, a gun bought off the wall sounds RE4's pickup only: BO1's purchase sounds that come in the same moment,
// before or after it, are dropped - the "cha-ching" (zmb_cha_ching) and the wall's own (zmb_weap_wall, ~70 ms after).
// A perk's or a door's cha-ching, with no pickup, still sounds.
float __cdecl SND_Runtime_PickupScale(unsigned int id, SndEntHandle ent)
{
    struct Purchase
    {
        unsigned int id;
        unsigned int time;
        SndEntHandle ent;
    };
    static unsigned int s_pickupTime;
    static Purchase s_purchases[2] = { { (unsigned int)SND_HashName("zmb_cha_ching") },
                                       { (unsigned int)SND_HashName("zmb_weap_wall") } };
    static const unsigned int pickup = (unsigned int)SND_HashName("leon_pickup");
    if ( !id )
        return 1.0f;
    const unsigned int now = Sys_Milliseconds();
    if ( id == pickup )
    {
        s_pickupTime = now;
        for ( const Purchase &purchase : s_purchases )
            if ( purchase.time && now - purchase.time < 300 )
            {
                SND_StopSoundAliasOnEnt(purchase.ent, purchase.id);
                Com_Printf(9, "runtime sounds: RE4's pickup stops BO1's purchase sound %08x\n", purchase.id);
            }
        return 1.0f;
    }
    for ( Purchase &purchase : s_purchases )
    {
        if ( purchase.id != id )
            continue;
        purchase.time = now;
        purchase.ent = ent;
        if ( !s_pickupTime || now - s_pickupTime >= 300 )
            return 1.0f;
        Com_Printf(9, "runtime sounds: RE4's pickup drops BO1's purchase sound %08x\n", id);
        return 0.0f;
    }
    return 1.0f;
}

bool __cdecl SND_Runtime_PlayLocal(int localClientNum, const char *alias)
{
    bool found = false;
    for ( const SndRuntimeAlias *a : s_runtimeAliases )
        if ( !_stricmp(a->name.c_str(), alias) )
            found = true;
    if ( !found )
        return false;
    CG_PlaySound(localClientNum, CG_GetClientNumForLocalClient(localClientNum), 0, 0, false, 1.0f,
                 (unsigned int)SND_HashName(alias));
    return true;
}
