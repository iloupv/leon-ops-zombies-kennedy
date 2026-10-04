#pragma once

struct cgCloudLayer_t // sizeof=0x1C
{                                       // XREF: .data:cgCloudLayer_t * cg_cloudLayers/r
    struct XModel *model;
    float cloudCover;
    float minHeight;
    float maxHeight;
    float minSpeed;
    float maxSpeed;
    float radius;
};

struct cgCloud_t // sizeof=0x1C
{                                       // XREF: .data:cgCloud_t * cg_clouds/r
    float pos[3];
    float speed;
    float qsin;
    float qcos;
    cgCloudLayer_t *layer;
};

void __cdecl CG_UpdateClouds(int msec);
