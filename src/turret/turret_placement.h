#pragma once

#include <universal/dvar.h>

enum turret_handler : __int32
{                                       // XREF: ?Turret_PlaceTurret_Internal@@YAHW4turret_handler@@HHQBM11QAM2@Z/r
                                        // Turret_PlaceTurret_Trace/r ...
    TURRET_HANDLER_CLIENT = 0x0,
    TURRET_HANDLER_SERVER = 0x1,
    TURRET_HANDLER_COUNT  = 0x2,
};

struct trace_t;
struct col_context_t;
struct entityState_s;
struct playerState_s;

void __cdecl Turret_PlaceTurret_RegisterDvars();
int __cdecl Turret_PlaceTurret_Internal(
                turret_handler handler,
                int clientNum,
                unsigned int moverEntNum,
                const float *moverOrigin,
                const float *viewOrigin,
                const float *viewAngles,
                float *outOrigin,
                float *outAngles);
void __cdecl Turret_PlaceTurret_Trace(
                turret_handler handler,
                trace_t *results,
                const float *start,
                const float *mins,
                const float *maxs,
                const float *end,
                int passEntityNum,
                int contentmask,
                col_context_t *context);
bool __cdecl Turret_PlaceTurret_UpdateFooting(
                turret_handler handler,
                int moverEntNum,
                trace_t *trace,
                float *outOrigin,
                float (*outAxis)[3]);
int __cdecl Turret_PlaceTurret_Client(int localClientNum, const entityState_s *es, float *outOrigin, float *outAngles);
int __cdecl Turret_PlaceTurret_Server(playerState_s *ps, float *outOrigin, float *outAngles);
