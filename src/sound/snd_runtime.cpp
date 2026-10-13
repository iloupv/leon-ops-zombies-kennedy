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

void __cdecl SND_Runtime_LoadForLevel()
{
    SND_Runtime_Free();
    if ( !SND_Active() )
        return;
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

bool __cdecl SND_Runtime_MutesVoice(unsigned int id)
{
    static std::vector<unsigned int> s_muted, s_kept; // the answers so far, per alias id
    if ( !id || !SND_Active() )
        return false;
    bool leon = false;
    for ( const SndRuntimeAlias *a : s_runtimeAliases )
        leon |= a->name == "leon_pain";
    if ( !leon )
        return false;
    for ( unsigned int m : s_muted )
        if ( m == id )
            return true;
    for ( unsigned int k : s_kept )
        if ( k == id )
            return false;
    const snd_alias_list_t *list = SND_FindAliasFromId(id);
    const bool mute = list && list->name && !_strnicmp(list->name, "vox_plr_", 8);
    (mute ? s_muted : s_kept).push_back(id);
    return mute;
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
