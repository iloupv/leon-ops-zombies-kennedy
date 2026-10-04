#include "ragdoll_quat.h"
#include <universal/assertive.h>
#include <universal/com_math.h>
#include <universal/q_shared.h>

void __cdecl Ragdoll_QuatMul(const float *qa, const float *qb, float *dest)
{
    float w0; // [esp+0h] [ebp-8h]
    float w1; // [esp+4h] [ebp-4h]

    iassert(qa != dest && qb != dest);
    w0 = qa[3];
    w1 = qb[3];
    dest[0] = w0 * qb[0] + w1 * qa[0] + qa[1] * qb[2] - qa[2] * qb[1];
    dest[1] = w0 * qb[1] + w1 * qa[1] + qa[2] * *qb - *qa * qb[2];
    dest[2] = w0 * qb[2] + w1 * qa[2] + *qa * qb[1] - qa[1] * *qb;
    dest[3] = w0 * w1 - qa[0] * qb[0] - qa[1] * qb[1] - qa[2] * qb[2];
}

void __cdecl Ragdoll_QuatMulInvSecond(const float *qa, const float *qb, float *dest)
{
    float qinv[4]; // [esp+0h] [ebp-10h] BYREF

    Ragdoll_QuatConjugate(qb, qinv);
    Ragdoll_QuatMul(qa, qinv, dest);
}

void __cdecl Ragdoll_QuatConjugate(const float *src, float *dest)
{
    dest[0] = -src[0];
    dest[1] = -src[1];
    dest[2] = -src[2];
    dest[3] = src[3];
}

void __cdecl Ragdoll_QuatInverse(const float *src, float *dest)
{
    iassert(Vec4IsNormalized(src));

    dest[0] = -src[0];
    dest[1] = -src[1];
    dest[2] = -src[2];
    dest[3] = src[3];
}

void __cdecl Ragdoll_QuatPointRotate(const float *p, const float *q, float *dest)
{
    float tmp0[4]; // [esp+0h] [ebp-40h] BYREF
    float qp[4]; // [esp+10h] [ebp-30h] BYREF
    float tmp1[4]; // [esp+20h] [ebp-20h] BYREF
    float qInv[4]; // [esp+30h] [ebp-10h] BYREF

    qp[0] = p[0];
    qp[1] = p[1];
    qp[2] = p[2];
    qp[3] = 0.0;
    Ragdoll_QuatInverse(q, qInv);
    Ragdoll_QuatMul(q, qInv, tmp0);
    Ragdoll_QuatMul(tmp0, qp, tmp1);
    dest[0] = tmp1[0];
    dest[1] = tmp1[1];
    dest[2] = tmp1[2];
}

void __cdecl Ragdoll_Mat33ToQuat(const float (*axis)[3], float *quat)
{
    const float *m = (const float *)axis;
    const float m00 = m[0], m01 = m[1], m02 = m[2];
    const float m10 = m[3], m11 = m[4], m12 = m[5];
    const float m20 = m[6], m21 = m[7], m22 = m[8];
    const float trace = m00 + m11 + m22 + 1.0f;
    float s;

    if ( trace > 0.000001f )
    {
        s = 0.5f / sqrtf(trace);
        quat[0] = (m21 - m12) * s;
        quat[1] = (m02 - m20) * s;
        quat[2] = (m10 - m01) * s;
        quat[3] = 0.25f / s;
    }
    else if ( m00 > m11 && m00 > m22 )
    {
        s = sqrtf(m00 - m11 - m22 + 1.0f) * 2.0f;
        quat[0] = 0.25f * s;
        quat[1] = (m01 + m10) / s;
        quat[2] = (m02 + m20) / s;
        quat[3] = (m21 - m12) / s;   // was (m12 - m21) — IDA-faithful but inconsistent with trace branch's qx ∝ (m21-m12)
    }
    else if ( m11 > m22 )
    {
        s = sqrtf(m11 - m00 - m22 + 1.0f) * 2.0f;
        quat[0] = (m01 + m10) / s;
        quat[1] = 0.25f * s;
        quat[2] = (m12 + m21) / s;
        quat[3] = (m02 - m20) / s;
    }
    else
    {
        s = sqrtf(m22 - m00 - m11 + 1.0f) * 2.0f;
        quat[0] = (m02 + m20) / s;
        quat[1] = (m12 + m21) / s;
        quat[2] = 0.25f * s;
        quat[3] = (m10 - m01) / s;   // was (m01 - m10) — same sign-convention fix as m00 branch
    }
}

void __cdecl Ragdoll_QuatToAxisAngle(const float *quat, float *axisAngle)
{
    float sinHalfTheta; // [esp+8h] [ebp-14h]
    float angleMag;     // [esp+Ch] [ebp-10h]
    float halfTheta;    // [esp+14h] [ebp-8h]

    halfTheta = Q_acos(quat[3]);
    sinHalfTheta = sin(halfTheta);
    if ( I_fabs(sinHalfTheta) <= 0.000001 )
    {
        Vec3Clear(axisAngle);
    }
    else
    {
        angleMag = halfTheta * 2.0 / sinHalfTheta;
        Vec3Scale(quat, angleMag, axisAngle);
    }
}

void __cdecl Ragdoll_QuatLerp(const float *qa, const float *qb, float t, float *dest)
{
    Vec4Lerp(qa, qb, t, dest);
    Vec4Normalize(dest);
}

