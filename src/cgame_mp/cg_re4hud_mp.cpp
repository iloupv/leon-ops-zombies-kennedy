// mod (the Leon mod): RE4's HUD in place of BO1's ammo counter (leon_re4hud). RE4's life meter (game/cockpit.cpp
// LifeMeter): the stone ring whose channel the life fills - green, yellow under two thirds, red under one third, as
// RE4's (pl_class.cpp cPlayer::getLifeLevel) - and inside it the heart monitor's pulse, quicker as the life falls; the
// rounds in the gun in RE4's digits (BulletInfo) with the reserve smaller beside them, its EMPTY badge for none; the
// character's name plate on the ring's arm. In the bottom right corner of the HUD's safe area (scrPlaceView: its
// margins), drawn from RE4's own images, converted from the player's RE4 at setup (tools/leon-converter/re4_hud.py:
// R_RuntimeXModel_HudMaterial); with none of them, BO1's ammo counter stays.
#include "cg_re4hud_mp.h"
#include "cg_local_mp.h"
#include "cg_main_mp.h"
#include "cg_newDraw_mp.h"
#include <cgame/cg_ammocounter.h>
#include <cgame/cg_drawtools.h>
#include <client/screen_placement.h>
#include <gfx_d3d/r_rendercmds.h>
#include <gfx_d3d/r_runtime_xmodel.h>
#include <bgame/bg_mantle.h>
#include <bgame/bg_weapons_ammo.h>
#include <ui/ui_main.h>

#include <math.h>

// The layout is in the frame image's pixels (572 x 360: the ring's centre and channel measured from it), drawn a quarter
// of a virtual unit each, its bottom right corner on the safe area's
static const float HUD_FRAME_W = 572.0f, HUD_FRAME_H = 360.0f, HUD_SCALE = 0.25f;
static const float RING_X = 182.0f, RING_Y = 177.0f; // the ring's centre
static const float RING_SCREEN_R = 128.0f; // the monitor's screen inside it
static const float CHANNEL_IN = 138.0f, CHANNEL_OUT = 168.0f; // the channel the life fills
static const float CHANNEL_FROM = 20.0f, CHANNEL_SPAN = 245.0f; // its arc: from the arm (degrees, clockwise) up and round
static const float DIGIT_W = 64.0f, DIGIT_H = 128.0f, DIGIT_ADVANCE = 46.0f / 0.95f; // a digit's cell, its pitch
static const float AMMO_BOTTOM = 268.0f; // the digits' cells sit on the arm

struct Re4HudPlace
{
    float x, y; // the frame's top left, real pixels
    float k; // real pixels a frame pixel
};

static bool CG_RE4HudOn()
{
    return R_RuntimeXModel_DvarOn("leon_re4hud") && R_RuntimeXModel_HudMaterial("frame");
}

bool __cdecl CG_RE4HudHidesAmmoCounter()
{
    return CG_RE4HudOn();
}

static void CG_RE4HudPic(const Re4HudPlace &place, float x, float y, float w, float h, const float *color, const char *name)
{
    Material *material = R_RuntimeXModel_HudMaterial(name);
    if ( material )
        R_AddCmdDrawStretchPic(place.x + x * place.k, place.y + y * place.k, w * place.k, h * place.k, 0.0f, 0.0f, 1.0f,
                               1.0f, color, material);
}

// a quad of the white material centred on (x, y) frame pixels, turned by angle degrees
static void CG_RE4HudQuad(int localClientNum, const Re4HudPlace &place, float x, float y, float w, float h, float angle,
                          const float *color)
{
    const float pw = w * place.k, ph = h * place.k;
    CG_DrawRotatedPicPhysical(&scrPlaceView[localClientNum], place.x + x * place.k - 0.5f * pw,
                              place.y + y * place.k - 0.5f * ph, pw, ph, angle, color, cgMedia.whiteMaterial);
}

// the heart monitor's trace at a beat's phase (0..1): the P wave, the QRS spike, the T wave; up is positive
static float CG_RE4HudBeat(float phase)
{
    static const float keys[][2] = { { 0.00f, 0.0f }, { 0.04f, 0.10f }, { 0.08f, 0.0f }, { 0.15f, 0.0f }, { 0.17f, -0.15f },
                                      { 0.20f, 1.0f }, { 0.23f, -0.35f }, { 0.26f, 0.0f }, { 0.42f, 0.0f }, { 0.48f, 0.22f },
                                      { 0.56f, 0.0f }, { 1.00f, 0.0f } };
    for ( int i = 1; i < (int)(sizeof(keys) / sizeof(keys[0])); ++i )
    {
        if ( phase <= keys[i][0] )
        {
            const float t = (phase - keys[i - 1][0]) / (keys[i][0] - keys[i - 1][0]);
            return keys[i - 1][1] + (keys[i][1] - keys[i - 1][1]) * t;
        }
    }
    return 0.0f;
}

// a count in RE4's digits, its cells' bottom right at (right, bottom) frame pixels, size a share of a full cell
static void CG_RE4HudNumber(const Re4HudPlace &place, int value, float right, float bottom, float size, const float *color)
{
    char text[12];
    const int len = sprintf_s(text, "%d", value < 0 ? 0 : (value > 999 ? 999 : value));
    const float w = DIGIT_W * size, h = DIGIT_H * size, advance = DIGIT_ADVANCE * size;
    for ( int i = 0; i < len; ++i )
    {
        const float x = right - w - (float)(len - 1 - i) * advance;
        CG_RE4HudPic(place, x, bottom - h, w, h, color, va("digit%c", text[i]));
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
    place.y = scrPlace->realViewableMax[1] - HUD_FRAME_H * place.k;
    const float time = (float)cgameGlob->time * 0.001f;

    // the life: its share of the most it can be, its state as RE4's - fine over two thirds, caution over one, danger
    // under (and down: danger, empty). The most it can be is the most he has had since he was last down: the max
    // health stat (stats[2]) stays the spawn's 100 under SP's setmaxhealth - Juggernog's 250, a character's own more -
    // while his health regenerates to the real one
    const bool down = ps->pm_type == 6 || ps->pm_type == 7;
    static int s_lifeMax[1];
    int &lifeMax = s_lifeMax[localClientNum & 0];
    if ( down || ps->stats[0] <= 0 )
        lifeMax = 0;
    lifeMax = ps->stats[0] > lifeMax ? ps->stats[0] : lifeMax;
    lifeMax = ps->stats[2] > lifeMax ? ps->stats[2] : lifeMax;
    float life = lifeMax > 0 ? (float)ps->stats[0] / (float)lifeMax : 0.0f;
    life = down ? 0.0f : (life < 0.0f ? 0.0f : (life > 1.0f ? 1.0f : life));
    const dvar_s *lifeTest = Dvar_FindVar("leon_re4hudtest"); // tests: the meter at that share (percent)
    if ( lifeTest && lifeTest->type == DVAR_TYPE_STRING && lifeTest->current.string && lifeTest->current.string[0] )
        life = (float)atof(lifeTest->current.string) * 0.01f;
    static const float s_fine[4] = { 0.25f, 0.95f, 0.30f, 0.95f };
    static const float s_caution[4] = { 1.0f, 0.82f, 0.10f, 0.95f };
    static const float s_danger[4] = { 1.0f, 0.12f, 0.08f, 0.95f };
    const float *state = life > 2.0f / 3.0f ? s_fine : (life > 1.0f / 3.0f ? s_caution : s_danger);
    const float beatsPerSecond = state == s_fine ? 1.1f : (state == s_caution ? 1.6f : 2.4f);
    static int s_loggedLife = -1;
    if ( R_RuntimeXModel_DvarOn("leon_re4hudlog") && (int)(life * 100.0f) != s_loggedLife ) // tests: each change of it
    {
        s_loggedLife = (int)(life * 100.0f);
        Com_Printf(0, "leon_re4hud: t %d life %d/%d (%d %%), pm_type %d\n", cgameGlob->time, ps->stats[0], lifeMax,
                   s_loggedLife, ps->pm_type);
    }

    // the monitor's screen, then its trace: two beats across it, scrolling, fading toward its oldest end
    static const float s_screen[4] = { 0.0f, 0.05f, 0.03f, 0.85f };
    CG_RE4HudPic(place, RING_X - RING_SCREEN_R, RING_Y - RING_SCREEN_R, 2.0f * RING_SCREEN_R, 2.0f * RING_SCREEN_R,
                 s_screen, "disc");
    const int samples = 56;
    const float traceW = 210.0f, traceAmp = down ? 0.0f : 55.0f;
    float prevX = 0.0f, prevY = 0.0f;
    for ( int i = 0; i <= samples; ++i )
    {
        const float u = (float)i / (float)samples; // 0: the oldest, left; 1: the newest
        float phase = time * beatsPerSecond - (1.0f - u) * 2.0f;
        phase -= floorf(phase);
        const float x = RING_X - 0.5f * traceW + u * traceW;
        const float y = RING_Y + 12.0f - CG_RE4HudBeat(phase) * traceAmp;
        if ( i )
        {
            const float dx = x - prevX, dy = y - prevY;
            float color[4] = { state[0], state[1], state[2], state[3] * (0.15f + 0.85f * u) };
            CG_RE4HudQuad(localClientNum, place, 0.5f * (x + prevX), 0.5f * (y + prevY), sqrtf(dx * dx + dy * dy) + 1.5f,
                          4.0f, atan2f(dy, dx) * 57.29578f, color);
        }
        prevX = x;
        prevY = y;
    }

    // the life in the channel, from its arm end round as far as the life goes; under the frame, whose spokes cut it into
    // RE4's segments
    const float filled = CHANNEL_SPAN * life;
    const float radius = 0.5f * (CHANNEL_IN + CHANNEL_OUT), thick = CHANNEL_OUT - CHANNEL_IN + 2.0f;
    for ( float a = 0.0f; a < filled; a += 3.0f )
    {
        const float step = filled - a < 3.0f ? filled - a : 3.0f;
        const float mid = (CHANNEL_FROM + a + 0.5f * step) * 0.017453292f;
        CG_RE4HudQuad(localClientNum, place, RING_X + cosf(mid) * radius, RING_Y + sinf(mid) * radius, thick,
                      radius * step * 0.017453292f + 1.0f, mid * 57.29578f, state);
    }
    CG_RE4HudPic(place, 0.0f, 0.0f, HUD_FRAME_W, HUD_FRAME_H, colorWhite, "frame");

    // the character's name plate on the arm (Luis has none in RE4: his name in the HUD's font)
    const dvar_s *character = Dvar_FindVar("leon_character");
    const char *name = character && character->type == DVAR_TYPE_STRING && character->current.string
                       && character->current.string[0] ? character->current.string : "leon";
    // each plate's lettering (its opaque box in the image, pixels): scaled to one height on the arm
    static const struct { const char *character; float w, h, left, top, textW, textH; } s_plates[] = {
        { "leon", 256.0f, 128.0f, 18.0f, 30.0f, 222.0f, 67.0f },
        { "ada", 280.0f, 136.0f, 19.0f, 20.0f, 245.0f, 99.0f },
        { "hunk", 376.0f, 136.0f, 15.0f, 23.0f, 344.0f, 96.0f },
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
        const float fontScale = 0.38f;
        Font_s *font = UI_GetFontHandle(scrPlace, 0, fontScale);
        char text[16];
        I_strncpyz(text, name, sizeof(text));
        text[0] = (char)toupper((unsigned char)text[0]);
        UI_DrawText(scrPlace, text, 16, font, (plateLeft - HUD_FRAME_W) * HUD_SCALE,
                    (plateMid + 0.5f * plateText - HUD_FRAME_H) * HUD_SCALE, 3, 3, fontScale, s_stone, 3);
    }

    // the rounds: in the gun (its clip's) and in reserve, smaller, at the right; EMPTY in place of none in the gun
    const unsigned int weapon = ClipCounterWeapIdx(cgameGlob, GetWeaponIndex(cgameGlob));
    if ( !weapon || down )
        return;
    const WeaponDef *weapDef = BG_GetWeaponDef(weapon);
    const int clip = BG_GetAmmoInClip(ps, weapon);
    const bool low = CG_CheckPlayerForLowAmmoSpecific(cgameGlob, weapon);
    static const float s_low[4] = { 1.0f, 0.35f, 0.25f, 1.0f };
    float right = HUD_FRAME_W;
    if ( !weapDef->suppressAmmoReserveDisplay )
    {
        static const float s_reserve[4] = { 0.82f, 0.82f, 0.82f, 1.0f };
        const int reserve = BG_GetTotalAmmoReserve(ps, weapon);
        const float size = 0.5f;
        CG_RE4HudNumber(place, reserve, right, AMMO_BOTTOM, size, s_reserve);
        int digits = reserve >= 100 ? 3 : (reserve >= 10 ? 2 : 1);
        right -= DIGIT_W * size + (float)(digits - 1) * DIGIT_ADVANCE * size + 10.0f;
    }
    if ( clip > 0 )
        CG_RE4HudNumber(place, clip, right, AMMO_BOTTOM, 0.95f, low ? s_low : colorWhite);
    else
        CG_RE4HudPic(place, right - 112.0f, AMMO_BOTTOM - 116.0f, 112.0f, 112.0f, colorWhite, "empty");
}
