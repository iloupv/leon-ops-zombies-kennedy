#pragma once

#include <xanim/xmodel.h>
#include <xanim/xanim.h>

void __cdecl XModelOptimizeCallback(XModel *data);
void __cdecl XModelOptimize(XModel *model);
void __cdecl _XModelOptimize(XModel *model);
void __cdecl XSurfaceOptimize(XModel *model, XSurface *surf);
void __cdecl XSurfaceOptimizeRigid(XModel *model, XSurface *surface);
void __cdecl XModelUnoptimizeCallback(XModel *data);
void __cdecl XModelUnoptimize(XModel *model);
void __cdecl XSurfaceUnoptimize(XSurface *surf);
void __cdecl XSurfaceUnoptimizeRigid(XSurface *surf);
