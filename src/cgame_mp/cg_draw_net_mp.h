#pragma once

struct lagometer_t // sizeof=0x1008
{                                       // XREF: .data:lagometer_t lagometer/r
    int frameSamples[128];              // XREF: CG_AddLagometerFrameInfo(cg_s const *)+21/w
    int frameCount;                     // XREF: CG_AddLagometerFrameInfo(cg_s const *)+16/r
    int snapshotFlags[128];             // XREF: CG_AddLagometerSnapshotInfo(snapshot_s *)+51/w
    int snapshotSamples[128];           // XREF: CG_AddLagometerSnapshotInfo(snapshot_s *)+11/w
    int entitySamples[128];             // XREF: CG_DrawEntityCount(int)+F7/r
    int changedEntitySamples[128];      // XREF: CG_DrawEntityCount(int)+118/r
    int archivedEntitySamples[128];     // XREF: CG_DrawEntityCount(int)+108/r
    int serverBandwidth[128];           // XREF: CG_DrawServerBandwidth(int)+D8/r
    int killcamTimes[128];              // XREF: CG_DrawServerKillcamData(int)+B9/r
    int snapshotCount;                  // XREF: CG_AddLagometerSnapshotInfo(snapshot_s *)+9/r
};

struct cg_s;
struct snapshot_s;

void __cdecl CG_AddLagometerFrameInfo(const cg_s *cgameGlob);
void __cdecl CG_AddLagometerSnapshotInfo(snapshot_s *snap);
void __cdecl CG_DrawLagometer(int localClientNum);
void __cdecl CG_DrawDisconnect(int localClientNum);
void __cdecl CG_DrawEntityCount(int localClientNum);
void __cdecl CG_DrawServerBandwidth(int localClientNum);
void __cdecl CG_DrawServerKillcamData(int localClientNum);
