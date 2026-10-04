#pragma once
#include <bgame/bg_public.h>
#include <clientscript/cscr_variable.h>
#include <qcommon/common.h>

void __cdecl Reached_ScriptMover(gentity_s *pEnt);
int __cdecl ScriptMover_UpdateMove(
                trajectory_t *pTr,
                float *vCurrPos,
                float fSpeed,
                float fMidTime,
                float fDecelTime,
                const float *vPos1,
                const float *vPos2,
                const float *vPos3);
void __cdecl ScriptMover_SetupPhysicsLaunch(
                trajectory_t *pTr,
                trajectory_t *paTr,
                const float *contact_point,
                const float *initial_force);
void __cdecl InitScriptMover(gentity_s *pSelf);
void __cdecl SP_script_brushmodel(gentity_s *self, SpawnVar *v = NULL);
void __cdecl SP_script_model(gentity_s *pSelf, SpawnVar *v = NULL);
void __cdecl SP_script_origin(gentity_s *pSelf, SpawnVar *v = NULL);
void __cdecl ScriptEntCmdGetCommandTimes(float *pfTotalTime, float *pfAccelTime, float *pfDecelTime);
void __cdecl ScriptEntCmd_MoveTo(scr_entref_t entref);
void __cdecl ScriptMover_Move(gentity_s *pEnt, const float *vPos, float fTotalTime, float fAccelTime, float fDecelTime);
void __cdecl ScriptMover_SetupMove(
                trajectory_t *pTr,
                const float *vPos,
                float fTotalTime,
                float fAccelTime,
                float fDecelTime,
                float *vCurrPos,
                float *pfSpeed,
                float *pfMidTime,
                float *pfDecelTime,
                float *vPos1,
                float *vPos2,
                float *vPos3);
void __cdecl ScriptEntCmd_GravityMove(scr_entref_t entref);
void __cdecl ScriptMover_GravityMove(gentity_s *mover, const float *velocity, float totalTime);
void __cdecl ScriptEnt_MoveAxis(scr_entref_t entref, int iAxis);
void __cdecl ScriptEntCmd_MoveX(scr_entref_t entref);
void __cdecl ScriptEntCmd_MoveY(scr_entref_t entref);
void __cdecl ScriptEntCmd_MoveZ(scr_entref_t entref);
void __cdecl ScriptEntCmd_RotateTo(scr_entref_t entref);
void __cdecl ScriptMover_Rotate(
                gentity_s *pEnt,
                const float *vRot,
                float fTotalTime,
                float fAccelTime,
                float fDecelTime);
void __cdecl ScriptEntCmd_DevAddPitch(scr_entref_t entref);
void __cdecl ScriptEnt_DevAddRotate(scr_entref_t entref, unsigned int iAxis);
void __cdecl ScriptEntCmd_DevAddYaw(scr_entref_t entref);
void __cdecl ScriptEntCmd_DevAddRoll(scr_entref_t entref);
void __cdecl ScriptEnt_RotateAxis(scr_entref_t entref, int iAxis);
void __cdecl ScriptEntCmd_RotatePitch(scr_entref_t entref);
void __cdecl ScriptEntCmd_RotateYaw(scr_entref_t entref);
void __cdecl ScriptEntCmd_RotateRoll(scr_entref_t entref);
void __cdecl ScriptEntCmd_Vibrate(scr_entref_t entref);
void __cdecl ScriptEntCmd_RotateVelocity(scr_entref_t entref);
void __cdecl ScriptMover_RotateSpeed(
                gentity_s *pEnt,
                const float *vRotSpeed,
                float fTotalTime,
                float fAccelTime,
                float fDecelTime);
void __cdecl ScriptMover_SetupMoveSpeed(
                trajectory_t *pTr,
                const float *vSpeed,
                float fTotalTime,
                float fAccelTime,
                float fDecelTime,
                float *vCurrPos,
                float *pfSpeed,
                float *pfMidTime,
                float *pfDecelTime,
                float *vPos1,
                float *vPos2,
                float *vPos3);
void __cdecl ScriptEntCmd_SetCanDamage(scr_entref_t entref);
void __cdecl ScriptEntCmd_PhysicsLaunch(scr_entref_t entref);
void __cdecl ScriptEntCmd_Solid(scr_entref_t entref);
void __cdecl ScriptEntCmd_NotSolid(scr_entref_t entref);
void (__cdecl *__cdecl ScriptEnt_GetMethod(const char **pName))(scr_entref_t);
