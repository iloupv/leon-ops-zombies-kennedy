// mod (the Leon mod): RE4's HUD in place of BO1's ammo counter (leon_re4hud). RE4's life meter (game/cockpit.cpp
// LifeMeter): the stone ring whose channel the life fills - half of it at full health, all of it with Juggernog, a gain
// filling in; green, yellow under two thirds, red under one third, as RE4's (pl_class.cpp cPlayer::getLifeLevel) - and
// inside it the rounds in the gun in RE4's digits (BulletInfo), red when few are left, with the reserve smaller under
// them, its EMPTY badge for none; the character's name plate on the ring's arm. In the bottom right corner of the HUD's safe area (scrPlaceView: its margins), drawn
// from RE4's own images, converted from the player's RE4 at setup (tools/leon-converter/re4_hud.py:
// R_RuntimeXModel_HudMaterial); with none of them, BO1's ammo counter stays.
// And what BO1's corner showed beside its ammo counter, moved round the ring: the grenades - lethal and tactical, each
// its icon and its count - in a row under it (the ring lifted over them), and the quick access items of the action slots
// - the d-pad's on a pad; on the keyboard +actionslot 1, 3 and 4, BO1's PC HUD's [X], [5] and [6] with SP's default keys
// - over the name plate, each its icon, its count and its key.
#include "cg_re4hud_mp.h"
#include "cg_local_mp.h"
#include "cg_main_mp.h"
#include "cg_newDraw_mp.h"
#include <cgame/cg_ammocounter.h>
#include <cgame/cg_drawtools.h>
#include <cgame/cg_weapons.h>
#include <cgame/offhandweapons.h>
#include <client/screen_placement.h>
#include <gfx_d3d/r_font.h>
#include <gfx_d3d/r_rendercmds.h>
#include <gfx_d3d/r_runtime_xmodel.h>
#include <bgame/bg_local.h>
#include <bgame/bg_mantle.h>
#include <bgame/bg_perks.h>
#include <bgame/bg_weapons_ammo.h>
#include <ui/ui_main.h>
#include <ui/ui_shared.h>
#include <win32/win_gamepad.h>

#include <math.h>

// The layout is in the frame image's pixels (572 x 360: the ring's centre and channel measured from it), drawn a quarter
// of a virtual unit each, its bottom right corner on the safe area's - over the grenades' row (below)
static const float HUD_FRAME_W = 572.0f, HUD_FRAME_H = 360.0f, HUD_SCALE = 0.25f;
static const float RING_X = 182.0f, RING_Y = 177.0f; // the ring's centre
static const float RING_SCREEN_R = 128.0f; // the dark screen inside it
static const float CHANNEL_IN = 138.0f, CHANNEL_OUT = 168.0f; // the channel the life fills
// its arc: from the arm (degrees, clockwise) down and round to the top - its clear pixels from 1.5 to 268.5, a little
// past them under the frame's ends
static const float CHANNEL_FROM = 1.0f, CHANNEL_SPAN = 268.5f;
static const float DIGIT_W = 64.0f, DIGIT_H = 128.0f, DIGIT_ADVANCE = 46.0f / 0.95f; // a digit's cell, its pitch
// the grenades' row under the ring (the frame's bottom, 360: the ring's own at 357), the HUD's whole height with it: the
// row's icons BO1's 18 virtual units (27 px at 720p, 81 at 4K - the icons' own margins in them)
static const float GRENADE_ROW_H = 80.0f, GRENADE_ICON = 72.0f, HUD_H = HUD_FRAME_H + GRENADE_ROW_H;
// the quick access items over the arm: cells this wide, packed from the safe area's edge leftward, two to a row (they
// reach 380: the stone's edge is at 345 to 372 by their icons, 395 to 400 by their keys), a third in a row over them;
// their keys' baseline above the arm's top (272 from the name plate on), each one's icon in a box this big over its key
// (BO1's d-pad icons: 32 virtual units, much of them margin)
static const float SLOT_W = 96.0f, SLOT_ICON = 88.0f, SLOT_KEY_BASE = 258.0f, SLOT_KEY_H = 30.0f, SLOT_ROW_H = 130.0f;

struct Re4HudPlace
{
    float x, y; // the frame's top left, real pixels
    float k; // real pixels a frame pixel
};

static bool CG_RE4HudOn()
{
    return R_RuntimeXModel_DvarOn("leon_re4hud") && R_RuntimeXModel_HudMaterial("frame");
}

// the corner's own panel (weaponinfo_zombie's and dpad_zombie's menus): the ammo counter (120 to 123), the blood and
// lines behind it and the d-pad's frame (116), the d-pad's arrows (294 to 297) and slots (166 to 169), the grenades'
// icons (103, 104), the weapon's name (81) - and with no pad (gpad_enabled 0) the slots' keys in its place, "[" +
// KeyBinding( "+actionslot 1" ) + "]" and those of slots 3 and 4 (392 to 395: dpad_zombie's texts, CG_OwnerDrawText's
// CG_DrawPlayerActionSlotBind), which stood half under the ring. The grenades and the slots are the RE4 HUD's own now
// (CG_DrawRE4HudGrenades, CG_DrawRE4HudSlots)
bool __cdecl CG_RE4HudHidesOwnerDraw(int ownerDraw)
{
    switch ( ownerDraw )
    {
    case 81:
    case 103:
    case 104:
    case 116:
    case 120:
    case 121:
    case 122:
    case 123:
    case 166:
    case 167:
    case 168:
    case 169:
    case 294:
    case 295:
    case 296:
    case 297:
    case 392:
    case 393:
    case 394:
    case 395:
        return CG_RE4HudOn();
    }
    return false;
}

// (over the ring, as high again as the grenades' row lifts it)
float __cdecl CG_RE4HudPointsLift()
{
    return CG_RE4HudOn() ? 22.0f + GRENADE_ROW_H * HUD_SCALE : 0.0f;
}

static void CG_RE4HudMaterial(const Re4HudPlace &place, float x, float y, float w, float h, const float *color,
                              Material *material)
{
    if ( material )
        R_AddCmdDrawStretchPic(place.x + x * place.k, place.y + y * place.k, w * place.k, h * place.k, 0.0f, 0.0f, 1.0f,
                               1.0f, color, material);
}

static void CG_RE4HudPic(const Re4HudPlace &place, float x, float y, float w, float h, const float *color, const char *name)
{
    CG_RE4HudMaterial(place, x, y, w, h, color, R_RuntimeXModel_HudMaterial(name));
}

// text in the HUD's font, its baseline at y, its left at x - or its middle, centred (frame pixels): UI_DrawText's
// virtual units from the safe area's bottom right (alignments 3, 3), as the frame is placed
static void CG_RE4HudText(const ScreenPlacement *scrPlace, const char *text, float x, float y, bool centred,
                          float fontScale, const float *color)
{
    Font_s *font = UI_GetFontHandle(scrPlace, 0, fontScale);
    float left = (x - HUD_FRAME_W) * HUD_SCALE;
    if ( centred )
        left -= 0.5f * (float)R_TextWidth(text, 0, font) * (float)R_NormalizedTextScale(font, fontScale);
    UI_DrawText(scrPlace, (char *)text, 0x7FFFFFFF, font, left, (y - HUD_H) * HUD_SCALE, 3, 3, fontScale, color, 3);
}

// a quad of the white material centred on (x, y) frame pixels, turned by angle degrees
static void CG_RE4HudQuad(int localClientNum, const Re4HudPlace &place, float x, float y, float w, float h, float angle,
                          const float *color)
{
    const float pw = w * place.k, ph = h * place.k;
    CG_DrawRotatedPicPhysical(&scrPlaceView[localClientNum], place.x + x * place.k - 0.5f * pw,
                              place.y + y * place.k - 0.5f * ph, pw, ph, angle, color, cgMedia.whiteMaterial);
}

// a count in RE4's digits, centred on centreX, its cells' bottom at bottom (frame pixels), size a share of a full cell
static void CG_RE4HudNumber(const Re4HudPlace &place, int value, float centreX, float bottom, float size, const float *color)
{
    char text[12];
    const int len = sprintf_s(text, "%d", value < 0 ? 0 : (value > 999 ? 999 : value));
    const float w = DIGIT_W * size, h = DIGIT_H * size, advance = DIGIT_ADVANCE * size;
    const float left = centreX - 0.5f * (w + (float)(len - 1) * advance);
    for ( int i = 0; i < len; ++i )
        CG_RE4HudPic(place, left + (float)i * advance, bottom - h, w, h, color, va("digit%c", text[i]));
}

// RE4's digits' glyphs fill their cell from 0.12 of its height to 0.83 (measured on the rounds): a count whose glyphs'
// bottom is at bottom
static void CG_RE4HudCount(const Re4HudPlace &place, int value, float centreX, float bottom, float size, const float *color)
{
    CG_RE4HudNumber(place, value, centreX, bottom + 0.17f * DIGIT_H * size, size, color);
}

// The grenades, in a row under the ring where BO1 had them under its ammo counter (weaponinfo_zombie's offhandSmokeIcon
// and offhandFragIcon, owner-draws 104 and 103: CG_DrawOffHandIcon): the tactical one (the Cymbal Monkey, the Gersh
// Device, Moon's QED - BO1's "smoke" slot) on the left, the lethal one (the frags, Moon's...) on the right, as BO1's. BO1
// stacked an icon a grenade; here each is its icon and the count in RE4's digits. Each shown while he has one (none
// thrown left: nothing, as BO1's), and as BO1's, not while the offhands are out of reach (IsOffHandDisplayVisible: a
// turret, a mantle) nor faded out (hud_fade_offhand); the icon the one he has equipped, else the first he holds.
static void CG_DrawRE4HudGrenades(int localClientNum, cg_s *cgameGlob, const Re4HudPlace &place)
{
    const playerState_s *ps = &cgameGlob->predictedPlayerState;
    if ( !IsOffHandDisplayVisible(cgameGlob) )
        return;
    float color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    color[3] = (float)CG_FadeHudMenu(localClientNum, hud_fade_offhand, cgameGlob->offhandFadeTime,
                                     (int)(hud_fade_offhand->current.value * 1000.0f));
    if ( color[3] <= 0.0f )
        return;
    // (each one's icon's left: the row centred under the ring, both there or not)
    static const struct { OffhandSlot slot; float x; } s_grenades[] = {
        { OFFHAND_SLOT_TACTICAL_GRENADE, 65.0f },
        { OFFHAND_SLOT_LETHAL_GRENADE, 192.0f },
    };
    const float top = HUD_FRAME_H + 0.5f * (GRENADE_ROW_H - GRENADE_ICON);
    for ( const auto &grenade : s_grenades )
    {
        const int count = CalcOffHandAmmo(ps, grenade.slot);
        if ( count <= 0 )
            continue;
        unsigned int weapon = 0;
        if ( cgameGlob->equippedOffHand && BG_GetWeaponDef(cgameGlob->equippedOffHand)->offhandSlot == grenade.slot )
            weapon = cgameGlob->equippedOffHand;
        if ( !weapon )
            weapon = GetBestOffhand(ps, grenade.slot);
        if ( !weapon )
            continue;
        CG_RE4HudMaterial(place, grenade.x, top, GRENADE_ICON, GRENADE_ICON, color, BG_GetWeaponDef(weapon)->hudIcon);
        CG_RE4HudCount(place, count, grenade.x + GRENADE_ICON + 18.0f, top + GRENADE_ICON - 8.0f, 0.45f, color);
    }
}

// What an action slot holds to show (BO1's CG_DrawPlayerActionSlot): its weapon (the claymores, Moon's equipment), or
// for the alternate mode's toggle the weapon it switches to (a gun's launcher); none when it is empty - nor equipment
// with nothing of it left, which BO1's d-pad leaves out too (outside an extra camera's view)
static unsigned int CG_RE4HudSlotWeapon(int localClientNum, cg_s *cgameGlob, unsigned int slotIdx)
{
    const playerState_s *ps = &cgameGlob->predictedPlayerState;
    if ( !CG_ActionSlotIsUsable(localClientNum, slotIdx) )
        return 0;
    if ( ps->actionSlotType[slotIdx] == ACTIONSLOTTYPE_SPECIFYWEAPON )
    {
        const unsigned int weapon = ps->actionSlotParam[slotIdx].specifyWeapon.index;
        const WeaponDef *weapDef = BG_GetWeaponDef(weapon);
        if ( weapDef->offhandSlot == OFFHAND_SLOT_EQUIPMENT && !BG_GetAmmoInClip(ps, weapon)
             && !BG_GetAmmoNotInClip(ps, weapon) && !weapDef->unlimitedAmmo && cgameGlob->extraCamEntity == 1023 )
            return 0;
        return weapon;
    }
    if ( ps->actionSlotType[slotIdx] == ACTIONSLOTTYPE_ALTWEAPONTOGGLE )
    {
        if ( BG_GetWeaponDef(cgameGlob->weaponSelect)->inventoryType == WEAPINVENTORY_ALTMODE )
            return cgameGlob->weaponSelect;
        return CG_AltWeaponToggleIndex(localClientNum, cgameGlob, 0);
    }
    return 0;
}

// The quick access items, over the name plate where BO1's d-pad was beside its ammo counter: the slots its menu shows
// (dpad_zombie's slot1, slot3 and slot4 - the d-pad's up, left and right; actionslot 2, down, it leaves out), in the
// d-pad's order left to right, those holding something packed from the safe area's edge (so a lone claymore sits at the
// right), two to a row (a third over them). Each its d-pad icon (a wide one as wide as the cell), its count when it has
// more than one (BO1's: on a dark disc at the icon's corner, red at none) and its key under it - BO1's PC text, "[" +
// KeyBinding( "+actionslot N" ) + "]", in its lit colour (CG_DrawPlayerActionSlotBind); with a pad on, the pad's
// button. An empty slot has nothing, as BO1's d-pad has no icon there (its PC menu kept the empty slots' keys, unlit:
// here they go).
static void CG_DrawRE4HudSlots(int localClientNum, cg_s *cgameGlob, const Re4HudPlace &place)
{
    const playerState_s *ps = &cgameGlob->predictedPlayerState;
    const ScreenPlacement *scrPlace = &scrPlaceView[localClientNum];
    const float alpha = (float)AmmoCounterFadeAlpha(localClientNum, cgameGlob);
    if ( alpha <= 0.0f )
        return;
    static const unsigned int s_slots[] = { 2, 0, 3 }; // (slotIdx: actionslot 3, 1, 4)
    int shown = 0;
    for ( int i = ARRAY_COUNT(s_slots) - 1; i >= 0; --i )
    {
        const unsigned int slotIdx = s_slots[i];
        const unsigned int weapon = CG_RE4HudSlotWeapon(localClientNum, cgameGlob, slotIdx);
        if ( !weapon )
            continue;
        const float centre = HUD_FRAME_W - ((float)(shown % 2) + 0.5f) * SLOT_W;
        const float keyBase = SLOT_KEY_BASE - (float)(shown / 2) * SLOT_ROW_H;
        ++shown;
        const WeaponVariantDef *weapVariantDef = BG_GetWeaponVariantDef(weapon);
        const WeaponDef *weapDef = BG_GetWeaponDef(weapon);
        float w = SLOT_ICON, h = SLOT_ICON;
        if ( weapVariantDef->dpadIconRatio == WEAPON_ICON_RATIO_2TO1 )
            h = 0.5f * SLOT_ICON;
        else if ( weapVariantDef->dpadIconRatio == WEAPON_ICON_RATIO_4TO1 )
            h = 0.25f * SLOT_ICON;
        const float iconBottom = keyBase - SLOT_KEY_H - 4.0f;
        const float color[4] = { 1.0f, 1.0f, 1.0f, alpha };
        CG_RE4HudMaterial(place, centre - 0.5f * w, iconBottom - h, w, h, color, weapVariantDef->dpadIcon);
        if ( (weapDef->iStartAmmo > 1 && !weapDef->bIsCarriedKillstreakWeapon)
             || ps->actionSlotType[slotIdx] == ACTIONSLOTTYPE_ALTWEAPONTOGGLE )
        {
            static const float s_low[4] = { 1.0f, 0.35f, 0.25f, 1.0f };
            const int ammo = BG_WeaponAmmo(ps, weapon);
            const float disc = 40.0f, discX = centre + 0.5f * w - 0.5f * disc, discY = iconBottom - 0.5f * disc;
            const float black[4] = { 0.0f, 0.0f, 0.0f, 0.8f * alpha };
            const float count[4] = { ammo ? 1.0f : s_low[0], ammo ? 1.0f : s_low[1], ammo ? 1.0f : s_low[2], alpha };
            CG_RE4HudMaterial(place, discX - 0.5f * disc, discY - 0.5f * disc, disc, disc, black,
                              cgMedia.ammoCounterRifleBullet); // (BO1's disc: hud_dpad_eqip_count_backing)
            CG_RE4HudCount(place, ammo, discX, discY + 13.0f, 0.3f, count);
        }
        char binding[256], key[260];
        UI_GetKeyBindingLocalizedString(localClientNum, va("+actionslot %d", slotIdx + 1), binding, 0);
        sprintf_s(key, gpad_enabled && gpad_enabled->current.enabled ? "%s" : "[%s]", binding);
        const float lit[4] = { 1.0f, 0.8f, 0.4f, 0.9f * alpha };
        CG_RE4HudText(scrPlace, key, centre, keyBase, true, 0.26f, lit);
    }
}

void __cdecl CG_DrawRE4Hud(int localClientNum)
{
    if ( !CG_RE4HudOn() )
        return;
    cg_s *cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    const playerState_s *ps = &cgameGlob->predictedPlayerState;
    const ScreenPlacement *scrPlace = &scrPlaceView[localClientNum];
    Re4HudPlace place;
    place.k = HUD_SCALE * scrPlace->scaleVirtualToReal[1];
    place.x = scrPlace->realViewableMax[0] - HUD_FRAME_W * place.k;
    place.y = scrPlace->realViewableMax[1] - HUD_H * place.k;

    // the life: its share of his max health, its state as RE4's - fine over two thirds, caution over one, danger under
    // (and down: danger, empty). His max is the player state's max health stat (stats[2]), kept in step with SP's
    // setmaxhealth (game_sp's PlayerCmdSP_setmaxhealth): the spawn's 100, Juggernog's 250, a character's own more
    // (Leon's 50) - and back to 100 when Leon is switched for Ada at the start, her full health a full meter (the
    // meter took the most he had had since he was last down for his max: Leon's 150 read Ada's 100 as two thirds)
    const bool down = ps->pm_type == 6 || ps->pm_type == 7;
    const int lifeMax = ps->stats[2] > 0 ? ps->stats[2] : ps->stats[0];
    float life = lifeMax > 0 ? (float)ps->stats[0] / (float)lifeMax : 0.0f;
    life = down ? 0.0f : (life < 0.0f ? 0.0f : (life > 1.0f ? 1.0f : life));
    // and as the meter shows it: a loss at once, a gain (the regen's, a revive's) filling in as RE4's own meter eases
    // to the life (LifeMeter::move: nine tenths of the shown life and a tenth of the real one each of its 30 frames
    // a second). _gameskill's regen gives the life back in steps - all of it at once after its delay, or a tenth
    // every 50 ms when he was very hurt - which the meter showed as jumps
    static float s_shownLife[1] = { -1.0f };
    static int s_shownTime[1];
    float &shown = s_shownLife[localClientNum & 0];
    int &shownTime = s_shownTime[localClientNum & 0];
    const int elapsed = cgameGlob->time - shownTime;
    shownTime = cgameGlob->time;
    if ( shown < 0.0f || life <= shown || elapsed < 0 || elapsed > 1000 ) // (a new level, a pause: as it is)
        shown = life;
    else
    {
        shown = life - (life - shown) * powf(0.9f, (float)elapsed * 0.03f);
        if ( life - shown < 0.002f )
            shown = life;
    }
    float meter = shown;
    const dvar_s *lifeTest = Dvar_FindVar("leon_re4hudtest"); // tests: the meter at that share (percent)
    if ( lifeTest && lifeTest->type == DVAR_TYPE_STRING && lifeTest->current.string && lifeTest->current.string[0] )
        meter = (float)atof(lifeTest->current.string) * 0.01f;
    static const float s_fine[4] = { 0.15f, 0.60f, 0.19f, 0.95f };
    static const float s_caution[4] = { 0.70f, 0.56f, 0.06f, 0.95f };
    static const float s_danger[4] = { 0.66f, 0.08f, 0.05f, 0.95f };
    const float *state = meter > 2.0f / 3.0f ? s_fine : (meter > 1.0f / 3.0f ? s_caution : s_danger);
    // the channel: half of it the life's whole without Juggernog, all of it with (his health then raised to the most)
    const bool jugg = BG_HasPerk(ps->perks, BG_GetPerkIndexForName("specialty_armorvest"));
    const float fill = jugg ? meter : 0.5f * meter;
    static int s_loggedLife = -1, s_loggedShown = -1, s_loggedMax = -1;
    if ( R_RuntimeXModel_DvarOn("leon_re4hudlog") && ((int)(life * 100.0f) != s_loggedLife
                                                      || (int)(meter * 100.0f) != s_loggedShown || lifeMax != s_loggedMax) )
    {
        // (tests: each change of it, of the meter, of his max)
        s_loggedLife = (int)(life * 100.0f);
        s_loggedShown = (int)(meter * 100.0f);
        s_loggedMax = lifeMax;
        Com_Printf(0, "leon_re4hud: t %d life %d/%d (%d %%) shown %d %%, pm_type %d\n", cgameGlob->time, ps->stats[0],
                   lifeMax, s_loggedLife, s_loggedShown, ps->pm_type);
    }

    // the ring's dark screen, the rounds on it (below)
    static const float s_screen[4] = { 0.0f, 0.05f, 0.03f, 0.85f };
    CG_RE4HudPic(place, RING_X - RING_SCREEN_R, RING_Y - RING_SCREEN_R, 2.0f * RING_SCREEN_R, 2.0f * RING_SCREEN_R,
                 s_screen, "disc");

    // the life in the channel, from its end at the arm round as far as the life goes; under the frame, whose spokes cut
    // it into RE4's segments
    const float filled = CHANNEL_SPAN * fill;
    const float radius = 0.5f * (CHANNEL_IN + CHANNEL_OUT), thick = CHANNEL_OUT - CHANNEL_IN + 2.0f;
    for ( float a = 0.0f; a < filled; a += 3.0f )
    {
        const float step = filled - a < 3.0f ? filled - a : 3.0f;
        const float mid = (CHANNEL_FROM + a + 0.5f * step) * 0.017453292f;
        CG_RE4HudQuad(localClientNum, place, RING_X + cosf(mid) * radius, RING_Y + sinf(mid) * radius, thick,
                      radius * step * 0.017453292f + 1.0f, mid * 57.29578f, state);
    }
    CG_RE4HudPic(place, 0.0f, 0.0f, HUD_FRAME_W, HUD_FRAME_H, colorWhite, "frame");

    // the character's name plate on the arm (Luis has none in RE4: re4_hud.py makes his of its plates' letters, and
    // without it his name is in the HUD's font)
    const dvar_s *character = Dvar_FindVar("leon_character");
    const char *name = character && character->type == DVAR_TYPE_STRING && character->current.string
                       && character->current.string[0] ? character->current.string : "leon";
    // each plate's lettering (its opaque box in the image, pixels): scaled to one height on the arm. Luis's is set at
    // Leon's plate's scale on its baseline: Leon's box's height, his i's dot over it
    static const struct { const char *character; float w, h, left, top, textW, textH; } s_plates[] = {
        { "leon", 256.0f, 128.0f, 18.0f, 30.0f, 222.0f, 67.0f },
        { "ada", 280.0f, 136.0f, 19.0f, 20.0f, 245.0f, 99.0f },
        { "hunk", 376.0f, 136.0f, 15.0f, 23.0f, 344.0f, 96.0f },
        { "luis", 255.0f, 128.0f, 18.0f, 30.0f, 220.0f, 67.0f },
    };
    const float plateText = 52.0f, plateLeft = 398.0f, plateMid = 313.0f, plateRoom = 165.0f;
    bool plated = false;
    for ( const auto &plate : s_plates )
    {
        if ( I_stricmp(plate.character, name) || !R_RuntimeXModel_HudMaterial(va("name_%s", plate.character)) )
            continue;
        float k = plateText / plate.textH;
        if ( plate.textW * k > plateRoom )
            k = plateRoom / plate.textW; // (HUNK's: as wide as the arm allows)
        CG_RE4HudPic(place, plateLeft - plate.left * k, plateMid - (plate.top + 0.5f * plate.textH) * k, plate.w * k,
                     plate.h * k, colorWhite, va("name_%s", plate.character));
        plated = true;
    }
    if ( !plated )
    {
        static const float s_stone[4] = { 0.86f, 0.83f, 0.76f, 1.0f };
        char text[16];
        I_strncpyz(text, name, sizeof(text));
        text[0] = (char)toupper((unsigned char)text[0]);
        CG_RE4HudText(scrPlace, text, plateLeft, plateMid + 0.5f * plateText, false, 0.38f, s_stone);
    }

    // the grenades under the ring, the quick access items over the name plate (not while he is down: his hands are
    // empty then - maps\leon\_leon.gsc leon_down_no_weapons)
    if ( !down )
    {
        CG_DrawRE4HudGrenades(localClientNum, cgameGlob, place);
        CG_DrawRE4HudSlots(localClientNum, cgameGlob, place);
    }

    // the rounds inside the ring: in the gun (its clip's), and in reserve, smaller, under them; EMPTY in place of none in
    // the gun
    const unsigned int weapon = ClipCounterWeapIdx(cgameGlob, GetWeaponIndex(cgameGlob));
    if ( !weapon || down )
        return;
    const WeaponDef *weapDef = BG_GetWeaponDef(weapon);
    const int clip = BG_GetAmmoInClip(ps, weapon);
    // the rounds in the gun red when few are left, as RE4's: BO1's own low clip (CG_CheckPlayerForLowClipSpecific, its
    // ammo counter's flashing clip: the weapon's lowAmmoWarningThreshold of a full clip, or under); a weapon without
    // one (no threshold, or a clip that is all its ammo) at a quarter of its clip. The reserve red as BO1's own when
    // it runs low (CG_CheckPlayerForLowAmmoSpecific: a fifth of the most it holds) - the clip took that colour before,
    // whatever the clip held
    const int clipSize = BG_GetClipSize(weapon);
    const bool ownLow = weapDef->lowAmmoWarningThreshold > 0.0f && !BG_WeaponIsClipOnly(weapon);
    const bool lowClip = ownLow ? CG_CheckPlayerForLowClipSpecific(cgameGlob, weapon)
                                : clipSize > 0 && (float)clip <= (float)clipSize * 0.25f;
    const bool reserveShown = !weapDef->suppressAmmoReserveDisplay;
    const int reserve = BG_GetTotalAmmoReserve(ps, weapon);
    const bool lowReserve = reserveShown && CG_CheckPlayerForLowAmmoSpecific(cgameGlob, weapon);
    static const float s_low[4] = { 1.0f, 0.35f, 0.25f, 1.0f };
    static unsigned int s_loggedWeapon;
    static int s_loggedLow = -1;
    if ( R_RuntimeXModel_DvarOn("leon_re4hudlog")
         && (weapon != s_loggedWeapon || (int)lowClip + 2 * (int)lowReserve != s_loggedLow) ) // tests: each change
    {
        s_loggedWeapon = weapon;
        s_loggedLow = (int)lowClip + 2 * (int)lowReserve;
        Com_Printf(0, "leon_re4hud: t %d weapon %s clip %d/%d (low at %.2f of it%s): %s, reserve %d: %s\n",
                   cgameGlob->time, BG_WeaponName(weapon), clip, clipSize,
                   ownLow ? weapDef->lowAmmoWarningThreshold : 0.25f, ownLow ? "" : ", the HUD's",
                   lowClip ? "red" : "white", reserve, lowReserve ? "red" : "grey");
    }
    const float clipBottom = RING_Y + (reserveShown ? 30.0f : 55.0f);
    if ( clip > 0 )
        CG_RE4HudNumber(place, clip, RING_X, clipBottom, 0.8f, lowClip ? s_low : colorWhite);
    else
        CG_RE4HudPic(place, RING_X - 56.0f, clipBottom - 100.0f, 112.0f, 112.0f, colorWhite, "empty");
    if ( reserveShown )
    {
        static const float s_reserve[4] = { 0.82f, 0.82f, 0.82f, 1.0f };
        CG_RE4HudNumber(place, reserve, RING_X, RING_Y + 98.0f, 0.45f, lowReserve ? s_low : s_reserve);
    }
}
