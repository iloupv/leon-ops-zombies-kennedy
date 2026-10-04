// Included by g_sp_savegame.cpp after g_sp_save_badplace.inl (uses the WriteStruct/ReadField port).
// zombies: clients (SP 0x007EA2A0 / 0x007EC530). SP gclient is 0x1D28; KB keeps its 0x29E0 MP
// gclient_s, so the core image is KB's with a DERIVED table, and the SP-only client fields KB keeps
// in side tables follow it as one DERIVED extension image. Extras follow in SP order.
#include "g_scr_sp_players_x1.h"
#include "g_sp_player_state.h"
#include <client_mp/g_client_mp.h>

// DERIVED side image: SP gclient fields that KB stores outside gclient_s.
struct SPClientSaveExt
{
    gclientSpExt ext;              // SP +0x1B64 perks, +0x1BA4 downs, +0x1BA8 revives, +0x1C80 lookatent
    SPPlayerX1State x1;            // SP clientState +0x44, +0x1CCC, +0x1D10
    SPPlayerBuiltinState builtin;  // SP ps +0x4FC perks, +0x1D0E, permission bits
    int scriptHintString;          // SP +0x584, hint index + 1 (0 = none)
};

#define CF(member, type) { offsetof(gclient_s, member), sizeof(((gclient_s *)0)->member), type }
static const SPEntitySaveField s_clientFields[] = {
    // SP 0x00A53B1C order: +0x1C84 entity pointer (type 2, no KB field: read only by SP
    // FireBullets 0x004625C0), +0x1C80 lookatent (type 3, in s_clientExtFields), +0x1CC0
    // useHoldEntity (type 3), ps +0x17C viewmodelIndex (type 18).
    CF(useHoldEntity, 3), CF(ps.viewmodelIndex, 18),
    // KB-only MP copy of the same model index.
    CF(sess.viewmodelIndex, 18)
};
#undef CF
static const SPEntitySaveField s_clientExtFields[] = {
    { offsetof(SPClientSaveExt, ext.lookatent), sizeof(EntHandle), 3 }
};
static_assert(sizeof(gclient_s) == 0x29E0, "DERIVED client image");
static_assert(sizeof(gclientSpExt) == 0x10 && sizeof(SPPlayerX1State) == 0xC
    && sizeof(SPPlayerBuiltinState) == 0x10 && sizeof(SPClientSaveExt) == 0x30, "DERIVED client extension image");
static_assert(sizeof(((usercmd_s *)0)->button_bits) == 8, "SP writes 8 button bytes");
static_assert(sizeof(ActionSlotParam) == 4 && ARRAY_COUNT(((playerState_s *)0)->actionSlotType) == 4,
    "SP action slot pairs");

static int G_LoadWeaponIndex(MemoryFile *memFile)
{
    // zombies: SP 0x007E9B40, byte-length name; empty means weapon 0.
    char name[256];
    G_LoadWeaponName(memFile, name);
    return name[0] ? G_GetWeaponIndexForName(name) : 0;
}

static void G_SaveClientWeapon(MemoryFile *memFile, unsigned int weapon)
{
    // zombies: SP 0x007EA3D8 / 0x007EA4AD: a zero index writes a zero length (empty string).
    G_SaveWeaponName(memFile, weapon ? BG_GetWeaponVariantDef(weapon)->szInternalName : "");
}

static void G_WriteClient(const gclient_s *client, const SPClientSaveExt *ext, MemoryFile *memFile)
{
    // zombies: SP 0x007EA2A0. Copy to the static image (SP 0x01C77080), clear the event rings and
    // entity event sequence (SP ps +0xE8..+0x12F, +0x520), then WriteStruct.
    static gclient_s image;
    memcpy(&image, client, sizeof(image));
    playerState_s *ps = &image.ps;
    ps->predictableEventSequence = 0;
    ps->predictableEventSequenceOld = 0;
    memset(ps->predictableEvents, 0, sizeof(ps->predictableEvents));
    memset(ps->predictableEventParms, 0, sizeof(ps->predictableEventParms));
    ps->unpredictableEventSequence = 0;
    ps->unpredictableEventSequenceOld = 0;
    memset(ps->unpredictableEvents, 0, sizeof(ps->unpredictableEvents));
    memset(ps->unpredictableEventParms, 0, sizeof(ps->unpredictableEventParms));
    ps->entityEventSequence = 0;
    SP_WriteStruct(s_clientFields, ARRAY_COUNT(s_clientFields), client, (unsigned char *)&image, sizeof(image), memFile);
    SP_WriteEntityStruct(s_clientExtFields, ARRAY_COUNT(s_clientExtFields), ext, sizeof(*ext), memFile);
    for (int i = 0; i < 4; ++i)
    {
        MemFile_WriteData(memFile, 4, (unsigned char *)&client->ps.actionSlotType[i]);
        MemFile_WriteData(memFile, 4, (unsigned char *)&client->ps.actionSlotParam[i]);
    }
    MemFile_WriteData(memFile, 8, (unsigned char *)&client->sess.cmd.button_bits);
    G_SaveClientWeapon(memFile, client->sess.cmd.weapon);
    G_SaveClientWeapon(memFile, client->sess.cmd.offHandIndex);
    // SP +0x1AD4, setmovespeedscale's value; KB keeps it in the session (g_scr_main_mp.cpp).
    MemFile_WriteData(memFile, 4, (unsigned char *)&client->sess.moveSpeedScaleMultiplier);
}

static void G_ReadClient(gclient_s *client, SPClientSaveExt *ext, MemoryFile *memFile, gentity_s *liveEnt)
{
    // zombies: SP 0x007EC530. liveEnt is the restored player entity when the client is installed
    // (SP passes its r.inuse); diagnostic reads pass nullptr and skip the live tail.
    SP_ReadEntityStruct(s_clientFields, ARRAY_COUNT(s_clientFields), client, sizeof(*client), memFile);
    SP_ReadEntityStruct(s_clientExtFields, ARRAY_COUNT(s_clientExtFields), ext, sizeof(*ext), memFile);
    for (int i = 0; i < 4; ++i)
    {
        MemFile_ReadData(memFile, 4, (unsigned char *)&client->ps.actionSlotType[i]);
        MemFile_ReadData(memFile, 4, (unsigned char *)&client->ps.actionSlotParam[i]);
    }
    MemFile_ReadData(memFile, 8, (unsigned char *)&client->sess.cmd.button_bits);
    client->sess.cmd.weapon = (unsigned short)G_LoadWeaponIndex(memFile);
    client->sess.cmd.offHandIndex = (unsigned short)G_LoadWeaponIndex(memFile);
    // Not ported: SP 0x007EC5DF..0x007EC657 copies weapon/offhand into the local cg usercmd and,
    // for an active local client 0, queues buttons/weapons for the next CL usercmd (0x00453880).
    // KB's loopback client input needs its own equivalent when the live install lands.
    MemFile_ReadData(memFile, 4, (unsigned char *)&client->sess.moveSpeedScaleMultiplier);
    if (!liveEnt)
        return;
    liveEnt->client->ps.eFlags |= 2; // SP 0x007EC67A: only for an in-use entity.
    // Not ported: SP 0x0059A340 then overwrites client +0x1A30 (usercmd) with the cg copy.
    SetClientViewAngle(liveEnt, client->ps.viewangles); // SP 0x00606320, ps +0x180
}
