#pragma once

struct msg_t;
struct snapshot_s;
struct centity_s;

void CL_SP_ResetAnimCommands(int localClientNum);
void CL_SP_ParseAnimCommands(int localClientNum, msg_t *msg, int messageNum, int serverTime);
bool CL_SP_GetAnimCommands(int localClientNum, int messageNum, snapshot_s *snapshot);
void CG_SP_ReplayAnimCommands(int localClientNum);
void CG_SP_ReplayEntityAnimCommands(int localClientNum, int entnum);
// L15 (TEST, bo1_measure_animscripted_cl): client scripted anim weights / placement trace.
void CG_SP_MeasureAnimScripted(int localClientNum, const centity_s *cent);
