#pragma once

struct XModel;

int __cdecl R_StaticModelGetMemoryUsage(XModel *model, int *modelCount);
int __cdecl R_StaticModelGetMemoryUsageInst();
