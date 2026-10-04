#include "actor_lookat.h"
#include <game_mp/actor_mp.h>
#include "actor_animapi.h"
#include <qcommon/dobj_management.h>
#include <game_mp/g_main_mp.h>

void __cdecl Actor_InitLookAt(actor_s *self)
{
    self->lookAtInfo.bDoLookAt = 0;
    self->lookAtInfo.vLookAtPos[0] = 0.0f;
    self->lookAtInfo.vLookAtPos[1] = 0.0f;
    self->lookAtInfo.vLookAtPos[2] = 0.0f;
    self->lookAtInfo.fLookAtTurnAngle = 0.0f;
    self->lookAtInfo.fLookAtTurnSpeed = 0.0f;
    self->lookAtInfo.fLookAtTurnAccel = 120.0f;
    self->lookAtInfo.fLookAtAnimYawLimit = 90.0f;
    self->lookAtInfo.fLookAtYawLimit = 90.0f;
    self->lookAtInfo.bLookAtSetup = 0;
}

void __cdecl Actor_SetLookAtAnimNodes(
                actor_s *self,
                unsigned __int16 animStraight,
                unsigned __int16 animLeft,
                unsigned __int16 animRight)
{
    unsigned int animLookAtStraight; // [esp-4h] [ebp-20h]
    XAnimTree_s *tree; // [esp+18h] [ebp-4h]

    tree = G_GetActorAnimTree(self);

    self->lookAtInfo.animLookAtStraight = animStraight;
    self->lookAtInfo.animLookAtLeft = animLeft;
    self->lookAtInfo.animLookAtRight = animRight;

    Actor_AnimClearGoalWeight(self->lookAtInfo.animLookAtLeft, 0.0);
    Actor_AnimClearGoalWeight(self->lookAtInfo.animLookAtRight, 0.0);

    animLookAtStraight = self->lookAtInfo.animLookAtStraight;
    Com_GetServerDObj(self->ent->s.number);
    Actor_AnimSetCompleteGoalWeight(tree, animLookAtStraight, 1.0, 0.0, 1.0, 0, 0, 0);
    self->lookAtInfo.fLookAtAnimBlendRate = 0.0f;
    self->lookAtInfo.fLookAtLimitBlendRate = 0.0f;
    self->lookAtInfo.iLookAtBlendEndTime = level.time;
    self->lookAtInfo.bLookAtSetup = 1;
}

void __cdecl Actor_SetLookAt(actor_s *self, const float *vPosition, float fTurnAccel)
{
    self->lookAtInfo.vLookAtPos[0] = *vPosition;
    self->lookAtInfo.vLookAtPos[1] = vPosition[1];
    self->lookAtInfo.vLookAtPos[2] = vPosition[2];
    if ( fTurnAccel > 20.0 )
        self->lookAtInfo.fLookAtTurnAccel = fTurnAccel;
    if ( self->lookAtInfo.fLookAtTurnAccel < 20.0 )
        self->lookAtInfo.fLookAtTurnAccel = 20.0f;
    self->lookAtInfo.bDoLookAt = 1;
}

void __cdecl Actor_SetLookAtYawLimits(actor_s *self, float fAnimYawLimit, float fYawLimit, float fBlendTime)
{
    float fLookYawLimitDifference; // [esp+0h] [ebp-8h]
    float fLookAnimYawLimitDifference; // [esp+4h] [ebp-4h]

    if ( fAnimYawLimit != self->lookAtInfo.fLookAtAnimYawLimit || fYawLimit != self->lookAtInfo.fLookAtYawLimit )
    {
        fLookAnimYawLimitDifference = fAnimYawLimit - Actor_CurrentLookAtAnimYawMax(self);
        self->lookAtInfo.fLookAtAnimYawLimit = fAnimYawLimit;
        fLookYawLimitDifference = fYawLimit - Actor_CurrentLookAtYawMax(self);
        self->lookAtInfo.fLookAtYawLimit = fYawLimit;
        self->lookAtInfo.iLookAtBlendEndTime = level.time + (int)(float)(fBlendTime * 1000.0);
        if ( fBlendTime == 0.0 )
        {
            self->lookAtInfo.fLookAtAnimBlendRate = 9999.0f;
            self->lookAtInfo.fLookAtLimitBlendRate = 9999.0f;
        }
        else
        {
            self->lookAtInfo.fLookAtAnimBlendRate = (float)(0.001 / fBlendTime) * fLookAnimYawLimitDifference;
            self->lookAtInfo.fLookAtLimitBlendRate = (float)(0.001 / fBlendTime) * fLookYawLimitDifference;
        }
    }
}

double __cdecl Actor_CurrentLookAtAnimYawMax(actor_s *self)
{
    int iTimeToTargetAnim; // [esp+0h] [ebp-8h]
    float fCurrentLimit; // [esp+4h] [ebp-4h]

    iTimeToTargetAnim = self->lookAtInfo.iLookAtBlendEndTime - level.time;
    if ( iTimeToTargetAnim < 0 )
        iTimeToTargetAnim = 0;
    fCurrentLimit = self->lookAtInfo.fLookAtAnimYawLimit
                                - (float)((float)iTimeToTargetAnim * self->lookAtInfo.fLookAtAnimBlendRate);
    if ( fCurrentLimit >= 0.0 )
    {
        if ( fCurrentLimit > 180.0 )
            return 180.0f;
    }
    else
    {
        return 0.0f;
    }
    return fCurrentLimit;
}

double __cdecl Actor_CurrentLookAtYawMax(actor_s *self)
{
    int iTimeToTargetLimit; // [esp+0h] [ebp-8h]
    float fCurrentLimit; // [esp+4h] [ebp-4h]

    iTimeToTargetLimit = self->lookAtInfo.iLookAtBlendEndTime - level.time;
    if ( iTimeToTargetLimit < 0 )
        iTimeToTargetLimit = 0;
    fCurrentLimit = self->lookAtInfo.fLookAtYawLimit
                                - (float)((float)iTimeToTargetLimit * self->lookAtInfo.fLookAtLimitBlendRate);
    if ( fCurrentLimit >= 0.0 )
    {
        if ( fCurrentLimit > 180.0 )
            return 180.0f;
    }
    else
    {
        return 0.0f;
    }
    return fCurrentLimit;
}

void __cdecl Actor_StopLookAt(actor_s *self, float fTurnAccel)
{
    if ( fTurnAccel > 20.0 )
        self->lookAtInfo.fLookAtTurnAccel = fTurnAccel;
    self->lookAtInfo.bDoLookAt = 0;
}

void __cdecl Actor_UpdateLookAt(actor_s *self)
{
    float vAngles[3]; // [esp+10h] [ebp-40h] BYREF
    float fLookYawTarget; // [esp+1Ch] [ebp-34h]
    float fFrametime; // [esp+20h] [ebp-30h]
    float fYawDelta; // [esp+24h] [ebp-2Ch]
    float fBaseYaw; // [esp+28h] [ebp-28h]
    float fLookYawLimit; // [esp+2Ch] [ebp-24h]
    float fYawSpeed; // [esp+30h] [ebp-20h]
    float vEyePosition[3]; // [esp+34h] [ebp-1Ch] BYREF
    float vDelta[3]; // [esp+40h] [ebp-10h] BYREF
    int bNegativeTarget; // [esp+4Ch] [ebp-4h]

    bNegativeTarget = 0;
    if ( self->lookAtInfo.bLookAtSetup
        && (self->lookAtInfo.bDoLookAt
         && self->lookAtInfo.fLookAtAnimYawLimit != 0.0
         && self->lookAtInfo.fLookAtYawLimit != 0.0
         || self->lookAtInfo.fLookAtTurnAngle != 0.0) )
    {
        fFrametime = 0.05f;
        fLookYawLimit = Actor_CurrentLookAtYawMax(self);
        Sentient_GetEyePosition(self->sentient, vEyePosition);
        vDelta[0] = self->lookAtInfo.vLookAtPos[0] - vEyePosition[0];
        vDelta[1] = self->lookAtInfo.vLookAtPos[1] - vEyePosition[1];
        vDelta[2] = self->lookAtInfo.vLookAtPos[2] - vEyePosition[2];
        vectoangles(vDelta, vAngles);
        fBaseYaw = self->ent->r.currentAngles[1];
        if ( !self->lookAtInfo.bDoLookAt
            || self->lookAtInfo.fLookAtAnimYawLimit == 0.0
            || self->lookAtInfo.fLookAtYawLimit == 0.0 )
        {
            fLookYawTarget = 0.0f;
        }
        else
        {
            fLookYawTarget = AngleNormalize180(fBaseYaw - vAngles[1]);
            if ( (-(fLookYawLimit)) <= fLookYawTarget )
            {
                if ( fLookYawTarget > fLookYawLimit )
                    fLookYawTarget = fLookYawLimit;
            }
            else
            {
                fLookYawTarget = -fLookYawLimit;
            }
        }
        fYawDelta = fLookYawTarget - self->lookAtInfo.fLookAtTurnAngle;
        if ( fabs(fYawDelta) >= 1.0 )
        {
            fYawSpeed = self->lookAtInfo.fLookAtTurnSpeed;
            if ( fLookYawTarget < 0.0 || fLookYawTarget == 0.0 && fYawDelta < 0.0 )
            {
                fYawSpeed = fYawSpeed * -1.0;
                fLookYawTarget = fLookYawTarget * -1.0;
                fYawDelta = fYawDelta * -1.0;
                bNegativeTarget = 1;
            }
            if ( fYawDelta >= 0.0 )
            {
                if ( (float)((float)(fFrametime * 2.0) * fYawSpeed) < fYawDelta )
                {
                    fYawSpeed = (float)(self->lookAtInfo.fLookAtTurnAccel * fFrametime) + fYawSpeed;
                }
                else
                {
                    fYawSpeed = fYawSpeed - (float)((float)(fFrametime * 10.0) * fYawSpeed);
                    if ( fYawSpeed >= 10.0 )
                    {
                        if ( fYawDelta > (float)((float)(fFrametime * 2.0) * fYawSpeed) )
                            fYawSpeed = (float)(fYawDelta / (float)(fFrametime * 2.0)) + 0.1;
                    }
                    else
                    {
                        fYawSpeed = 10.0f;
                    }
                }
            }
            else if ( fYawDelta < (float)((float)((float)(fFrametime * 2.0) * fYawSpeed) * -1.0) )
            {
                fYawSpeed = fYawSpeed - (float)(self->lookAtInfo.fLookAtTurnAccel * fFrametime);
            }
            else
            {
                fYawSpeed = (float)((float)(fFrametime * 10.0) * fYawSpeed) + fYawSpeed;
                if ( fYawSpeed <= -10.0 )
                {
                    if ( (float)((float)((float)(fFrametime * 2.0) * fYawSpeed) * -1.0) > fYawDelta )
                        fYawSpeed = (float)((float)(fYawDelta / (float)(fFrametime * 2.0)) * -1.0) - 0.1;
                }
                else
                {
                    fYawSpeed = -10.0f;
                }
            }
            if ( fYawSpeed > (float)(self->lookAtInfo.fLookAtTurnAccel * 0.22) )
                fYawSpeed = self->lookAtInfo.fLookAtTurnAccel * 0.22;
            if ( bNegativeTarget )
            {
                fYawSpeed = fYawSpeed * -1.0;
                fLookYawTarget = fLookYawTarget * -1.0;
            }
            self->lookAtInfo.fLookAtTurnSpeed = fYawSpeed;
            if ( fLookYawTarget <= self->lookAtInfo.fLookAtTurnAngle )
            {
                self->lookAtInfo.fLookAtTurnAngle = (float)(self->lookAtInfo.fLookAtTurnSpeed * fFrametime)
                                                                                    + self->lookAtInfo.fLookAtTurnAngle;
                if ( fLookYawTarget >= self->lookAtInfo.fLookAtTurnAngle )
                {
                    self->lookAtInfo.fLookAtTurnAngle = fLookYawTarget;
                    self->lookAtInfo.fLookAtTurnSpeed = 0.0f;
                }
            }
            else
            {
                self->lookAtInfo.fLookAtTurnAngle = (float)(self->lookAtInfo.fLookAtTurnSpeed * fFrametime)
                                                                                    + self->lookAtInfo.fLookAtTurnAngle;
                if ( self->lookAtInfo.fLookAtTurnAngle >= fLookYawTarget )
                {
                    self->lookAtInfo.fLookAtTurnAngle = fLookYawTarget;
                    self->lookAtInfo.fLookAtTurnSpeed = 0.0f;
                }
            }
        }
        else
        {
            self->lookAtInfo.fLookAtTurnAngle = fLookYawTarget;
            self->lookAtInfo.fLookAtTurnSpeed = 0.0f;
        }
    }
}

