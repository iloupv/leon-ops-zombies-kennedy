#include "ragdoll_controller.h"
#include "ragdoll.h"
#include <bgame/bg_local.h>
#include <xanim/dobj_utils.h>

BoneOrientation *__cdecl Ragdoll_BodyBoneOrientations(RagdollBody *body)
{
    iassert(body);
    bcassert(body->curOrientationBuffer, 2);
    
    return body->boneOrientations[body->curOrientationBuffer];
}

BoneOrientation *__cdecl Ragdoll_BodyPrevBoneOrientations(RagdollBody *body)
{
    iassert(body);
    bcassert(body->curOrientationBuffer, 2);

    return body->boneOrientations[body->curOrientationBuffer ^ 1];
}

void __cdecl Ragdoll_DoControllers(const cpose_t *pose, const DObj *obj, int *partBits)
{
    float v4; // [esp+0h] [ebp-3Ch]
    float v5; // [esp+Ch] [ebp-30h]
    BoneOrientation *boneOrientation; // [esp+1Ch] [ebp-20h]
    RagdollBody *body; // [esp+20h] [ebp-1Ch]
    DObjAnimMat *boneMat; // [esp+24h] [ebp-18h]
    DObjAnimMat *skel; // [esp+28h] [ebp-14h]
    LerpBone *lerpBone; // [esp+2Ch] [ebp-10h]
    int boneIdx; // [esp+30h] [ebp-Ch]
    int boneIdxa; // [esp+30h] [ebp-Ch]
    Bone *bone; // [esp+38h] [ebp-4h]

    iassert(pose);
    iassert(obj);

    if ( pose->isRagdoll && (pose->ragdollHandle || pose->killcamRagdollHandle) )
    {
        body = (pose->killcamRagdollHandle ? Ragdoll_HandleBody(pose->killcamRagdollHandle) : Ragdoll_HandleBody(pose->ragdollHandle));
        if ( Ragdoll_BodyPoseValid(body) )
        {
            skel = DObjGetRotTransArray(obj);
            if ( skel )
            {
                bone = body->bones;
                boneOrientation = Ragdoll_BodyBoneOrientations(body);
                for ( int i = 0; i < body->numBones; ++i )
                {
                    boneIdx = bone->animBones[0];
                    if ( DObjSetSkelRotTransIndex(obj, partBits, boneIdx) )
                    {
                        boneMat = &skel[boneIdx];
                        boneMat->trans[0] = boneOrientation->origin[0];
                        boneMat->trans[1] = boneOrientation->origin[1];
                        boneMat->trans[2] = boneOrientation->origin[2];
                        boneMat->quat[0] = boneOrientation->orientation[0];
                        boneMat->quat[1] = boneOrientation->orientation[1];
                        boneMat->quat[2] = boneOrientation->orientation[2];
                        boneMat->quat[3] = boneOrientation->orientation[3];
                        v5 = Vec4LengthSq(boneMat->quat);
                        if ( v5 == 0.0 )
                        {
                            boneMat->quat[3] = 1.0f;
                            boneMat->transWeight = 2.0f;
                        }
                        else
                        {
                            boneMat->transWeight = 2.0 / v5;
                        }
                    }
                    ++bone;
                    ++boneOrientation;
                }
                lerpBone = body->lerpBones;
                for ( int i = 0; i < body->numLerpBones; ++i )
                {
                    boneIdxa = lerpBone->animBone;
                    if ( DObjSetSkelRotTransIndex(obj, partBits, boneIdxa) )
                    {
                        boneMat = &skel[boneIdxa];
                        boneMat->trans[0] = boneOrientation->origin[0];
                        boneMat->trans[1] = boneOrientation->origin[1];
                        boneMat->trans[2] = boneOrientation->origin[2];
                        boneMat->quat[0] = boneOrientation->orientation[0];
                        boneMat->quat[1] = boneOrientation->orientation[1];
                        boneMat->quat[2] = boneOrientation->orientation[2];
                        boneMat->quat[3] = boneOrientation->orientation[3];
                        v4 = Vec4LengthSq(boneMat->quat);
                        if ( v4 == 0.0 )
                        {
                            boneMat->quat[3] = 1.0f;
                            boneMat->transWeight = 2.0f;
                        }
                        else
                        {
                            boneMat->transWeight = 2.0 / v4;
                        }
                    }
                    ++lerpBone;
                    ++boneOrientation;
                }
            }
        }
    }
}

