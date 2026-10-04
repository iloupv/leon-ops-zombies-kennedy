#pragma once

// j1: client half of the play-session recorder (bo1_playtrace, TOOL, off by default). Main thread only.
void CG_SP_PlayTraceClientFrame(int localClientNum);                            // after CL_Frame
void CG_SP_PlayTracePredictionMiss(float len);                                  // CG_PredictPlayerState miss
void CG_SP_PlayTraceKey(int key, int down, int repeats, const char *binding);   // CL_KeyEvent (F8 marker)
