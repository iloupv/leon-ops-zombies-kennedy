#pragma once

struct CURVESORTARGS // sizeof=0x10
{                                       // XREF: .data:CURVESORTARGS gCurveSortArgs/r
    bool increasing_sort_order;         // XREF: cCurve::CurveSortCompare(void const *,void const *)+3A/r
                                        // cCurve::Sort(float * const,bool)+C/w
    // padding byte
    // padding byte
    // padding byte
    float p[3];                         // XREF: cCurve::CurveSortCompare(void const *,void const *)+16/o
                                        // cCurve::CurveSortCompare(void const *,void const *)+2A/o ...
};

struct cCurve // sizeof=0xE58
{                                       // XREF: .data:cCurve * cCurveManager::mCurves/r
                                        // .data:cCurve g_ropeCurve/r
    enum eCurveType : __int32
    {                                       // XREF: cCurve/r
        CURVE_RNS     = 0x0,
        CURVE_SNS     = 0x1,
        CURVE_TNS     = 0x2,
        CURVE_BSPLINE = 0x3,
    };

    struct __declspec(align(4)) nodeData // sizeof=0x24
    {                                       // XREF: cCurve/r
        float pos[3];
        float vel[3];
        float dist;
        float totalDist;
        bool bReached;
        // padding byte
        // padding byte
        // padding byte
    };
    cCurve::nodeData mNodes[100];
    int mNotifyEntNum;
    int mCameraEntNum;
    bool mNotifyLevel;
    // padding byte
    // padding byte
    // padding byte
    float mSpeed;
    int mNumNodes;
    bool mActive;                       // XREF: cCurveManager::GetFreeCurve(void)+31/r
                                        // cCurveManager::GetFreeCurve(void)+4B/w ...
    // padding byte
    // padding byte
    // padding byte
    cCurve::eCurveType mCurveType;      // XREF: Rope_BuildCurve:loc_85F7ED/r
                                        // Rope_BuildCurve+276/w
    float mDistance;
    bool mDraw;
    // padding byte
    // padding byte
    // padding byte
    float mDrawColor[4];
    bool mUseColor;
    bool mPaused;
    // padding byte
    // padding byte
    float mCurPos;
    float mCurServerPos;
    int mLastNode;
    int mIndex;

    cCurve();

    static int CurveSortCompare(const float *e1, const float *e2);

    void Reset();
    void Reinit();
    void AddNode(float *p);
    void AddNode(float *p, float timePeriod);
    void GetPosition(float time, float *pos);
    void GetStartVelocity(int index, float *vel);
    void GetEndVelocity(int index, float *vel);
    void Sort(float *p, bool is_increasing_sort_order);
    void BuildNonUniform();
    void BuildBSpline();
    void Build();
    void Smooth();
    void Constrain();
    void GetPos(float t, float *p);
    double GetLength();
    void SetDraw(const float *userColor);
};

void __cdecl GetPositionOnCubic(
                float *startPos,
                float *startVel,
                float *endPos,
                float *endVel,
                float time,
                float *output);
void __cdecl GetPositionOnSpine(float *p1, float *p2, float *p3, float *p4, float time, float *output);

