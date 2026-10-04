#pragma once

// mod: mapkit (bo1_mod_mapkit, off by default). A map is a layout file = a retail base map + a list of edits,
// read from mods/mapkit/layouts/<name>.json when the clipmap loads. Box/room edits become extra collision
// brushes that every world trace also tests (CM_Trace / CM_PositionTest / CM_PointContents / sight traces) and
// are drawn through the code mesh (R_Mapkit_GenerateVerts). The other edit kinds are read by the GSC runtime
// (mods/mapkit) through the bo1_mapkit_* builtins.

struct traceWork_t;
struct cbrush_t;
struct trace_t;

struct MapkitBox
{
    float mins[3];
    float maxs[3];
    int contents;
    int editIndex;
    bool hidden; // layout blocker drawing; collision belongs to its retail script_brushmodel
    float texScale;
    float lightOrigin[3]; // where the renderer samples the retail light grid (room boxes: a point inside the room)
    char material[64];
};

void CM_Mapkit_Load(const char *mapName);
// mod: noperks (cm_noperks.cpp): world brushes around the perk machines off at map load when dvar noperks is 1
void CM_NoPerks_Apply(const char *name);
// mod: noperks - true when noperks removed a perk machine within radius of origin on the loaded map
bool CM_NoPerks_NearMachine(const float *origin, float radius);
bool CM_Mapkit_Active();
// layout "empty": true = the retail world (draw, sky, collision, static models, pathnodes) is off; the layout is the map
bool CM_Mapkit_Empty();
// empty base: the level entity string = retail worldspawn + the layout's spawn/ent edits (CM_EntityString)
const char *CM_Mapkit_EntityString(const char *retail);
// empty base: cm.cmodels/brushes/leafbrushNodes grown by one box brush model per zone / wallbuy / "bmodel" ent (CM_LoadMap)
void CM_Mapkit_ExtendSubmodels();
// mapkit-fix-4b: hidden player+monster clip boxes around perk / pap / powerswitch / mysterybox edits, snap:"wall" resolved
// (CM_LoadMap after CM_InitAllThreadData: it traces); the resolved spot is readable as edit fields "_spot" / "_spotyaw"
void CM_Mapkit_AddMachineClips();
bool CM_Mapkit_ResolvedSpot(int edit, float *origin, float *yaw);
bool CM_Mapkit_MachineClip(int edit, float *mins, float *maxs); // fields "_clipmins" / "_clipmaxs"
bool CM_Mapkit_IsSubmodel(unsigned int index); // generated collision only; no retail GfxBrushModel
// a "window" edit's traverse pair in world space (begin outside, end inside) and its yaw; false for other edits
bool CM_Mapkit_WindowNodes(int edit, float *begin, float *end, float *yaw);
bool CM_Mapkit_StairsStep(int edit, int step, float *mins, float *maxs);
int CM_Mapkit_WalkNodes(int edit, float (*out)[3], int max);
// mapkit-fix-3 mood lighting: tint (1 = the flat grey) of the last "light" edit containing p, its light index; -1 = none
int CM_Mapkit_LightAt(const float *p, float *rgb);
void CM_Mapkit_LightUsed(int light);
int CM_Mapkit_LightUses(int edit); // renderer samples that took the edit's tint; -1 = not a light edit
int CM_Mapkit_BoxCount();
const MapkitBox *CM_Mapkit_GetBox(int index);
void CM_Mapkit_SetVisible(int edit, bool visible);
// the box as an axial cbrush_t outside cm.brushes (a world prim for the GJK / prim traces: phys_traverse.cpp intersect_box)
const cbrush_t *CM_Mapkit_GetBrush(int index);
bool CM_Mapkit_IsBrush(const void *brush);

void CM_Mapkit_TraceThrough(const traceWork_t *tw, trace_t *trace);
void CM_Mapkit_TestIn(const traceWork_t *tw, trace_t *trace);
int CM_Mapkit_PointContents(const float *p);
int CM_Mapkit_SightTrace(const traceWork_t *tw, trace_t *trace);

// layout access (edits are the entries of the layout's "edits" array)
enum MapkitFieldType
{
    MKF_NONE = 0,
    MKF_STRING = 1,
    MKF_NUMBER = 2,
    MKF_VECTOR = 3,
};
const char *CM_Mapkit_LayoutName();
int CM_Mapkit_EditCount();
// key may be a dotted path into nested objects/arrays ("gaps.0.side"); "#name" gives an array/object length
int CM_Mapkit_GetField(int edit, const char *key, const char **str, float *vec);
// game side (game/g_mapkit_path.cpp): layout pathnodes appended to PathData and linked, G_InitGame first load only
void G_Mapkit_AppendPathnodes();
void G_Mapkit_LinkPathnodes();
void G_Mapkit_RestorePathnodeStrings(); // restart: the layout nodes' target/targetname again
int G_Mapkit_FirstAddedNode();
int G_Mapkit_AddedNodeCount();
void G_Mapkit_RecordCheck(const char *message); // count/mirror native and script checks for the completion marker
void G_Mapkit_ResetChecks();
