#pragma once
// a1 chunk 6: TEST SWITCH bo1_nettrace (headless -Client only, measurement, no behaviour change). See the .cpp.

struct client_t;
struct usercmd_s;

bool CG_SP_NetTraceEnabled();
void CG_SP_NetTraceClientFrame(int localClientNum);
void CG_SP_NetTraceServerCmd(const client_t *cl, const usercmd_s *cmd);
void CG_SP_NetTraceServerSnap(const client_t *cl);
void CG_SP_NetTraceServerFrame(int stage);

// L13: TEST SWITCHES bo1_hudtrace / bo1_shocktrace (measurement only). See the .cpp.
void CG_SP_HudTraceClientFrame(int localClientNum);
bool CG_SP_ShockTraceEnabled();
