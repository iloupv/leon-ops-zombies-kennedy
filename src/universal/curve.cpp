#include "curve.h"
#include "q_shared.h"
#include <cstdlib>

CURVESORTARGS gCurveSortArgs;

cCurve::cCurve()
{
    cCurve::Reset();
}

static const float gHermiteMatrix[4][4] =
{
  { 2.0, -2.0, 1.0, 1.0 },
  { -3.0, 3.0, -2.0, -1.0 },
  { 0.0, 0.0, 1.0, 0.0 },
  { 1.0, 0.0, 0.0, 0.0 }
};

void __cdecl GetPositionOnCubic(
                float *startPos,
                float *startVel,
                float *endPos,
                float *endVel,
                float time,
                float *output)
{
    float resVec[4]; // [esp+0h] [ebp-A0h] BYREF
    float result[4][4]; // [esp+10h] [ebp-90h] BYREF
    float mat[4][4]; // [esp+50h] [ebp-50h] BYREF
    float timeVector[4]; // [esp+90h] [ebp-10h] BYREF

    *(_QWORD *)&mat[0][0] = *(_QWORD *)startPos;
    mat[0][2] = startPos[2];
    mat[0][3] = 0.0f;
    *(_QWORD *)&mat[1][0] = *(_QWORD *)endPos;
    mat[1][2] = endPos[2];
    mat[1][3] = 0.0f;
    *(_QWORD *)&mat[2][0] = *(_QWORD *)startVel;
    mat[2][2] = startVel[2];
    mat[2][3] = 0.0f;
    *(_QWORD *)&mat[3][0] = *(_QWORD *)endVel;
    mat[3][2] = endVel[2];
    mat[3][3] = 0.0f;
    MatrixMultiply44(gHermiteMatrix, mat, result);
    timeVector[0] = (float)(time * time) * time;
    timeVector[1] = time * time;
    timeVector[2] = time;
    timeVector[3] = 1.0f;
    MatrixTransformVector44(timeVector, result, resVec);
    *output = resVec[0];
    output[1] = resVec[1];
    output[2] = resVec[2];
}

void __cdecl GetPositionOnSpine(float *p1, float *p2, float *p3, float *p4, float time, float *output)
{
    float u_3; // [esp+0h] [ebp-10h]
    float u; // [esp+4h] [ebp-Ch]
    float u_2; // [esp+8h] [ebp-8h]
    float u_4; // [esp+Ch] [ebp-4h]

    u = (float)((float)(1.0 - time) * (float)(1.0 - time)) * (float)(1.0 - time);
    u_2 = (float)(3.0 * time) * (float)((float)(1.0 - time) * (float)(1.0 - time));
    u_3 = (float)((float)(3.0 * time) * time) * (float)(1.0 - time);
    u_4 = (float)(time * time) * time;
    *output = (float)((float)((float)(*p1 * u) + (float)(*p2 * u_2)) + (float)(*p3 * u_3)) + (float)(*p4 * u_4);
    output[1] = (float)((float)((float)(p1[1] * u) + (float)(p2[1] * u_2)) + (float)(p3[1] * u_3)) + (float)(p4[1] * u_4);
    output[2] = (float)((float)((float)(p1[2] * u) + (float)(p2[2] * u_2)) + (float)(p3[2] * u_3)) + (float)(p4[2] * u_4);
}

void __thiscall cCurve::Reset()
{
    int i; // [esp+4h] [ebp-4h]

    this->mActive = 0;
    this->mDraw = 0;
    this->mUseColor = 0;
    this->mPaused = 1;
    this->mNotifyEntNum = 1023;
    this->mNotifyLevel = 0;
    this->mCameraEntNum = 1023;
    this->mLastNode = -1;
    this->mNumNodes = 0;
    this->mCurveType = CURVE_RNS;
    this->mDistance = 0.0f;
    this->mCurPos = 0.0f;
    this->mCurServerPos = 0.0f;
    this->mSpeed = 12.0f;
    for ( i = 0; i < 100; ++i )
    {
        this->mNodes[i].pos[2] = 0.0f;
        this->mNodes[i].pos[1] = 0.0f;
        this->mNodes[i].pos[0] = 0.0f;
        this->mNodes[i].vel[2] = 0.0f;
        this->mNodes[i].vel[1] = 0.0f;
        this->mNodes[i].vel[0] = 0.0f;
        this->mNodes[i].dist = 0.0f;
        this->mNodes[i].totalDist = 0.0f;
        this->mNodes[i].bReached = 0;
    }
}

void __thiscall cCurve::Reinit()
{
    this->mActive = 1;
    this->mDraw = 0;
    this->mUseColor = 0;
    this->mPaused = 1;
    this->mNotifyEntNum = 1023;
    this->mNotifyLevel = 0;
    this->mCameraEntNum = 1023;
    this->mLastNode = -1;
    this->mNumNodes = 0;
    this->mCurveType = CURVE_RNS;
    this->mDistance = 0.0f;
    this->mCurPos = 0.0f;
    this->mCurServerPos = 0.0f;
}

void cCurve::AddNode(float *p)
{
    cCurve::nodeData *v3; // [esp+8h] [ebp-10h]
    float diff[3]; // [esp+Ch] [ebp-Ch] BYREF

    if ( !this->mActive
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\curve.cpp", 148, 0, "%s", "mActive == true") )
    {
        __debugbreak();
    }
    if ( this->mNumNodes >= 100
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\curve.cpp",
                    149,
                    0,
                    "%s",
                    "mNumNodes < MAX_NODES") )
    {
        __debugbreak();
    }
    this->mNodes[this->mNumNodes].pos[0] = *p;
    this->mNodes[this->mNumNodes].pos[1] = p[1];
    this->mNodes[this->mNumNodes].pos[2] = p[2];
    if ( this->mNumNodes )
    {
        v3 = &this->mNodes[this->mNumNodes - 1];
        diff[0] = v3->pos[0] - *p;
        diff[1] = v3->pos[1] - p[1];
        diff[2] = v3->pos[2] - p[2];
        this->mNodes[this->mNumNodes - 1].dist = Vec3Length(diff);
        this->mDistance = this->mDistance + this->mNodes[this->mNumNodes - 1].dist;
        this->mNodes[this->mNumNodes].totalDist = this->mDistance;
    }
    else
    {
        this->mDistance = 0.0f;
    }
    ++this->mNumNodes;
}

void __thiscall cCurve::AddNode(float *p, float timePeriod)
{
    if ( !this->mActive
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\curve.cpp", 177, 0, "%s", "mActive == true") )
    {
        __debugbreak();
    }
    if ( this->mNumNodes >= 100
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\curve.cpp",
                    178,
                    0,
                    "%s",
                    "mNumNodes < MAX_NODES") )
    {
        __debugbreak();
    }
    this->mNodes[this->mNumNodes].pos[0] = *p;
    this->mNodes[this->mNumNodes].pos[1] = p[1];
    this->mNodes[this->mNumNodes].pos[2] = p[2];
    if ( this->mNumNodes )
    {
        this->mNodes[this->mNumNodes - 1].dist = timePeriod;
        this->mDistance = this->mDistance + this->mNodes[this->mNumNodes - 1].dist;
        this->mNodes[this->mNumNodes].totalDist = this->mDistance;
    }
    else
    {
        this->mDistance = 0.0f;
    }
    this->mCurveType = CURVE_TNS;
    ++this->mNumNodes;
}

void __thiscall cCurve::GetPosition(float time, float *pos)
{
    float v3; // [esp+10h] [ebp-54h]
    float dist; // [esp+18h] [ebp-4Ch]
    float *v5; // [esp+20h] [ebp-44h]
    float t; // [esp+34h] [ebp-30h]
    float endVel[3]; // [esp+38h] [ebp-2Ch] BYREF
    float startVel[3]; // [esp+44h] [ebp-20h] BYREF
    int cNum; // [esp+50h] [ebp-14h]
    float curveTime; // [esp+54h] [ebp-10h]
    float currentDistance; // [esp+58h] [ebp-Ch]
    float distance; // [esp+5Ch] [ebp-8h]
    int i; // [esp+60h] [ebp-4h]

    if ( time >= 0.0 )
    {
        if ( time > 1.0 )
            time = 1.0f;
    }
    else
    {
        time = 0.0f;
    }
    distance = time * this->mDistance;
    currentDistance = 0.0f;
    i = 0;
    if ( this->mCurveType == CURVE_BSPLINE )
    {
        while ( distance > this->mNodes[i].totalDist && i < this->mNumNodes && i < 100 )
            ++i;
    }
    else
    {
        while ( distance > (float)(currentDistance + this->mNodes[i].dist) && i < 100 )
            currentDistance = currentDistance + this->mNodes[i++].dist;
    }
    if ( this->mCurveType == CURVE_BSPLINE )
    {
        if ( i >= 4 )
        {
            cNum = (int)((float)((float)(i - 4) / 3.0) - 0.4999999990686774);
            i = 3 * cNum + 3;
        }
        else
        {
            i = 0;
        }
        if ( i <= 0 )
        {
            if ( this->mNodes[i + 3].totalDist <= 0.000001 )
                curveTime = 1.0f;
            else
                curveTime = distance / this->mNodes[i + 3].totalDist;
        }
        else
        {
            curveTime = (float)(distance - this->mNodes[i].totalDist)
                                / (float)(this->mNodes[i + 3].totalDist - this->mNodes[i].totalDist);
        }
        if ( this->mNodes[i + 3].totalDist <= 0.000001 )
        {
            v5 = this->mNodes[i].pos;
            *pos = *v5;
            pos[1] = v5[1];
            pos[2] = v5[2];
        }
        else
        {
            GetPositionOnSpine(
                this->mNodes[i].pos,
                this->mNodes[i + 1].pos,
                this->mNodes[i + 2].pos,
                this->mNodes[i + 3].pos,
                curveTime,
                pos);
        }
    }
    else
    {
        t = distance - currentDistance;
        if ( this->mNodes[i].dist != 0.0 )
            t = t / this->mNodes[i].dist;
        dist = this->mNodes[i].dist;
        startVel[0] = dist * this->mNodes[i].vel[0];
        startVel[1] = dist * this->mNodes[i].vel[1];
        startVel[2] = dist * this->mNodes[i].vel[2];
        v3 = this->mNodes[i].dist;
        endVel[0] = v3 * this->mNodes[i + 1].vel[0];
        endVel[1] = v3 * this->mNodes[i + 1].vel[1];
        endVel[2] = v3 * this->mNodes[i + 1].vel[2];
        GetPositionOnCubic(this->mNodes[i].pos, startVel, this->mNodes[i + 1].pos, endVel, t, pos);
    }
}

void __thiscall cCurve::GetStartVelocity(int index, float *vel)
{
    float v3; // [esp+8h] [ebp-18h]
    float temp_4; // [esp+18h] [ebp-8h]
    float temp_8; // [esp+1Ch] [ebp-4h]

    v3 = 1.0 / this->mNodes[index].dist;
    temp_4 = (float)(3.0 * (float)(v3 * (float)(this->mNodes[index + 1].pos[1] - this->mNodes[index].pos[1])))
                 - this->mNodes[index + 1].vel[1];
    temp_8 = (float)(3.0 * (float)(v3 * (float)(this->mNodes[index + 1].pos[2] - this->mNodes[index].pos[2])))
                 - this->mNodes[index + 1].vel[2];
    *vel = 0.5
             * (float)((float)(3.0 * (float)(v3 * (float)(this->mNodes[index + 1].pos[0] - this->mNodes[index].pos[0])))
                             - this->mNodes[index + 1].vel[0]);
    vel[1] = 0.5 * temp_4;
    vel[2] = 0.5 * temp_8;
}

void __thiscall cCurve::GetEndVelocity(int index, float *vel)
{
    float v3; // [esp+8h] [ebp-18h]
    float temp_4; // [esp+18h] [ebp-8h]
    float temp_8; // [esp+1Ch] [ebp-4h]

    v3 = 1.0 / this->mNodes[index - 1].dist;
    temp_4 = (float)(3.0 * (float)(v3 * (float)(this->mNodes[index].pos[1] - this->mNodes[index - 1].pos[1])))
                 - this->mNodes[index - 1].vel[1];
    temp_8 = (float)(3.0 * (float)(v3 * (float)(this->mNodes[index].pos[2] - this->mNodes[index - 1].pos[2])))
                 - this->mNodes[index - 1].vel[2];
    *vel = 0.5
             * (float)((float)(3.0 * (float)(v3 * (float)(this->mNodes[index].pos[0] - this->mNodes[index - 1].pos[0])))
                             - this->mNodes[index - 1].vel[0]);
    vel[1] = 0.5 * temp_4;
    vel[2] = 0.5 * temp_8;
}

int __cdecl cCurve::CurveSortCompare(const float *e1, const float *e2)
{
    float d1; // [esp+18h] [ebp-10h]
    float d2; // [esp+1Ch] [ebp-Ch]

    d1 = Vec3DistanceSq(gCurveSortArgs.p, e1);
    d2 = Vec3DistanceSq(gCurveSortArgs.p, e2);
    if ( gCurveSortArgs.increasing_sort_order )
        return (int)(float)(d1 - d2);
    else
        return (int)(float)(d2 - d1);
}

void __thiscall cCurve::Sort(float *p, bool is_increasing_sort_order)
{
    int v3; // ecx
    double v4; // st7
    float diff[3]; // [esp+10h] [ebp-10h] BYREF
    int i; // [esp+1Ch] [ebp-4h]

    gCurveSortArgs.increasing_sort_order = is_increasing_sort_order;
    gCurveSortArgs.p[0] = *p;
    gCurveSortArgs.p[1] = p[1];
    gCurveSortArgs.p[2] = p[2];
    qsort(this, this->mNumNodes, 0x24u, (int (__cdecl *)(const void *, const void *))cCurve::CurveSortCompare);
    for ( i = 0; i < this->mNumNodes; ++i )
    {
        if ( i )
        {
            v3 = i - 1;
            diff[0] = this->mNodes[v3].pos[0] - this->mNodes[i].pos[0];
            diff[1] = this->mNodes[v3].pos[1] - this->mNodes[i].pos[1];
            diff[2] = this->mNodes[v3].pos[2] - this->mNodes[i].pos[2];
            v4 = Vec3Length(diff);
            this->mNodes[i - 1].dist = v4;
            this->mDistance = this->mDistance + this->mNodes[i - 1].dist;
            this->mNodes[i].totalDist = this->mDistance;
        }
        else
        {
            this->mDistance = 0.0f;
            this->mNodes[0].totalDist = 0.0f;
        }
    }
}

void __thiscall cCurve::BuildNonUniform()
{
    float *vel; // [esp+20h] [ebp-6Ch]
    int j; // [esp+6Ch] [ebp-20h]
    float diff2[3]; // [esp+70h] [ebp-1Ch] BYREF
    float diff1[3]; // [esp+7Ch] [ebp-10h] BYREF
    int i; // [esp+88h] [ebp-4h]

    for ( i = 1; i < this->mNumNodes - 1; ++i )
    {
        diff1[0] = this->mNodes[i + 1].pos[0] - this->mNodes[i].pos[0];
        diff1[1] = this->mNodes[i + 1].pos[1] - this->mNodes[i].pos[1];
        diff1[2] = this->mNodes[i + 1].pos[2] - this->mNodes[i].pos[2];
        if ( diff1[0] != 0.0 && diff1[1] != 0.0 && diff1[2] != 0.0 )
            Vec3NormalizeFast(diff1);
        diff2[0] = this->mNodes[i - 1].pos[0] - this->mNodes[i].pos[0];
        diff2[1] = this->mNodes[i - 1].pos[1] - this->mNodes[i].pos[1];
        diff2[2] = this->mNodes[i - 1].pos[2] - this->mNodes[i].pos[2];
        if ( diff2[0] != 0.0 && diff2[1] != 0.0 && diff2[2] != 0.0 )
            Vec3NormalizeFast(diff2);
        vel = this->mNodes[i].vel;
        *vel = diff1[0] - diff2[0];
        vel[1] = diff1[1] - diff2[1];
        vel[2] = diff1[2] - diff2[2];
        if ( this->mNodes[i].vel[0] != 0.0 && this->mNodes[i].vel[1] != 0.0 && this->mNodes[i].vel[2] != 0.0 )
            Vec3NormalizeFast(this->mNodes[i].vel);
    }
    cCurve::GetStartVelocity(0, this->mNodes[0].vel);
    cCurve::GetEndVelocity(this->mNumNodes - 1, this->mNodes[this->mNumNodes - 1].vel);
    if ( this->mCurveType )
    {
        for ( j = 0; j < 3; ++j )
        {
            cCurve::Smooth();
            if ( this->mCurveType == CURVE_TNS )
                cCurve::Constrain();
        }
    }
}

void __thiscall cCurve::BuildBSpline()
{
    int i; // [esp+4h] [ebp-14h]
    float lastNode[3]; // [esp+8h] [ebp-10h] BYREF
    int nNeeded; // [esp+14h] [ebp-4h]

    nNeeded = 0;
    if ( this->mNumNodes >= 4 )
    {
        if ( (this->mNumNodes - 1) % 3 )
            nNeeded = 3 - (this->mNumNodes - 1) % 3;
    }
    else
    {
        nNeeded = 4 - this->mNumNodes;
    }
    if ( nNeeded )
    {
        lastNode[0] = this->mNodes[this->mNumNodes - 1].pos[0];
        lastNode[1] = this->mNodes[this->mNumNodes - 1].pos[1];
        lastNode[2] = this->mNodes[this->mNumNodes - 1].pos[2];
        for ( i = 0; i < nNeeded; ++i )
        {
            if ( this->mNumNodes < 100 )
                cCurve::AddNode(lastNode);
        }
    }
}

void __thiscall cCurve::Build()
{
    if ( this->mCurveType == CURVE_BSPLINE )
        cCurve::BuildBSpline();
    else
        cCurve::BuildNonUniform();
}

void __thiscall cCurve::Smooth()
{
    float *v1; // ecx
    float *vel; // [esp+Ch] [ebp-44h]
    float v4; // [esp+10h] [ebp-40h]
    float v5; // [esp+14h] [ebp-3Ch]
    float dist; // [esp+18h] [ebp-38h]
    float endVel[3]; // [esp+1Ch] [ebp-34h] BYREF
    float startVel[3]; // [esp+28h] [ebp-28h] BYREF
    int i; // [esp+34h] [ebp-1Ch]
    float newVel[3]; // [esp+38h] [ebp-18h]
    float oldVel[3]; // [esp+44h] [ebp-Ch] BYREF

    cCurve::GetStartVelocity(0, oldVel);
    for ( i = 1; i < this->mNumNodes - 1; ++i )
    {
        cCurve::GetEndVelocity(i, endVel);
        dist = this->mNodes[i].dist;
        endVel[0] = dist * endVel[0];
        endVel[1] = dist * endVel[1];
        endVel[2] = dist * endVel[2];
        cCurve::GetStartVelocity(i, startVel);
        v5 = this->mNodes[i - 1].dist;
        startVel[0] = v5 * startVel[0];
        startVel[1] = v5 * startVel[1];
        startVel[2] = v5 * startVel[2];
        newVel[0] = startVel[0] + endVel[0];
        newVel[1] = startVel[1] + endVel[1];
        newVel[2] = startVel[2] + endVel[2];
        v4 = 1.0 / (float)(this->mNodes[i - 1].dist + this->mNodes[i].dist);
        newVel[0] = v4 * (float)(startVel[0] + endVel[0]);
        newVel[1] = v4 * (float)(startVel[1] + endVel[1]);
        newVel[2] = v4 * (float)(startVel[2] + endVel[2]);
        vel = this->mNodes[i - 1].vel;
        *vel = oldVel[0];
        vel[1] = oldVel[1];
        vel[2] = oldVel[2];
        oldVel[0] = newVel[0];
        oldVel[1] = newVel[1];
        oldVel[2] = newVel[2];
    }
    cCurve::GetEndVelocity(this->mNumNodes - 1, this->mNodes[this->mNumNodes - 1].vel);
    v1 = this->mNodes[this->mNumNodes - 2].vel;
    *v1 = oldVel[0];
    v1[1] = oldVel[1];
    v1[2] = oldVel[2];
}

void __thiscall cCurve::Constrain()
{
    double v1; // st7
    int v2; // eax
    double v3; // st7
    float *vel; // [esp+4h] [ebp-3Ch]
    float v6; // [esp+8h] [ebp-38h]
    float delta[3]; // [esp+28h] [ebp-18h] BYREF
    float r0; // [esp+34h] [ebp-Ch]
    float r1; // [esp+38h] [ebp-8h]
    int i; // [esp+3Ch] [ebp-4h]

    for ( i = 1; i < this->mNumNodes - 1; ++i )
    {
        delta[0] = this->mNodes[i].pos[0] - this->mNodes[i - 1].pos[0];
        delta[1] = this->mNodes[i].pos[1] - this->mNodes[i - 1].pos[1];
        delta[2] = this->mNodes[i].pos[2] - this->mNodes[i - 1].pos[2];
        v1 = Vec3Length(delta);
        r0 = v1 / this->mNodes[i - 1].dist;
        v2 = i + 1;
        delta[0] = this->mNodes[v2].pos[0] - this->mNodes[i].pos[0];
        delta[1] = this->mNodes[v2].pos[1] - this->mNodes[i].pos[1];
        delta[2] = this->mNodes[v2].pos[2] - this->mNodes[i].pos[2];
        v3 = Vec3Length(delta);
        r1 = v3 / this->mNodes[i].dist;
        vel = this->mNodes[i].vel;
        v6 = (float)((float)(4.0 * r0) * r1) / (float)((float)(r0 + r1) * (float)(r0 + r1));
        *vel = v6 * *vel;
        vel[1] = v6 * vel[1];
        vel[2] = v6 * vel[2];
    }
}

void __thiscall cCurve::GetPos(float t, float *p)
{
    if ( (t < 0.0 || t > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\curve.cpp",
                    633,
                    0,
                    "%s",
                    "(t >= 0.0f) && (t <= 1.0f)") )
    {
        __debugbreak();
    }
    cCurve::GetPosition(t, p);
}

double __thiscall cCurve::GetLength()
{
    double v1; // st7
    float diff[3]; // [esp+10h] [ebp-44h] BYREF
    int j; // [esp+1Ch] [ebp-38h]
    float pos; // [esp+20h] [ebp-34h]
    float end; // [esp+24h] [ebp-30h]
    float step; // [esp+28h] [ebp-2Ch]
    int i; // [esp+2Ch] [ebp-28h]
    float curveLength; // [esp+30h] [ebp-24h]
    int kNumSteps; // [esp+34h] [ebp-20h]
    float endPos[3]; // [esp+38h] [ebp-1Ch] BYREF
    float start; // [esp+44h] [ebp-10h]
    float startPos[3]; // [esp+48h] [ebp-Ch] BYREF

    if ( !this->mActive
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\curve.cpp", 640, 0, "%s", "mActive == true") )
    {
        __debugbreak();
    }
    if ( this->mCurveType != CURVE_TNS )
        return this->mDistance;
    curveLength = 0.0f;
    start = 0.0f;
    kNumSteps = 32;
    for ( i = 0; i < this->mNumNodes; ++i )
    {
        end = start + this->mNodes[i].dist;
        step = (float)(end - start) / 32.0;
        pos = start;
        start = start + this->mNodes[i].dist;
        cCurve::GetPosition(pos / this->mDistance, startPos);
        for ( j = 0; j < 32; ++j )
        {
            pos = pos + step;
            cCurve::GetPosition(pos / this->mDistance, endPos);
            diff[0] = endPos[0] - startPos[0];
            diff[1] = endPos[1] - startPos[1];
            diff[2] = endPos[2] - startPos[2];
            v1 = Vec3Length(diff);
            curveLength = v1 + curveLength;
            startPos[0] = endPos[0];
            startPos[1] = endPos[1];
            startPos[2] = endPos[2];
        }
    }
    return curveLength;
}

void __thiscall cCurve::SetDraw(const float *userColor)
{
    this->mDraw = 1;
    if ( userColor )
    {
        this->mDrawColor[0] = *userColor;
        this->mDrawColor[1] = userColor[1];
        this->mDrawColor[2] = userColor[2];
        this->mDrawColor[3] = userColor[3];
        this->mUseColor = 1;
    }
    else
    {
        this->mUseColor = 0;
    }
}

