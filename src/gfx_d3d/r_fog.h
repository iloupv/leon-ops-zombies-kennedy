#pragma once

void __cdecl R_ClearFogs();
void __cdecl R_SetFogFromServer(
                unsigned int localClientNum,
                float start,
                float r,
                float g,
                float b,
                float density,
                float heightDensity,
                float baseHeight,
                float fogColorScale,
                float sunFogColorR,
                float sunFogColorG,
                float sunFogColorB,
                float sunFogDirX,
                float sunFogDirY,
                float sunFogDirZ,
                float sunFogStartAng,
                float sunFogEndAng,
                float fogMaxOpacity);
void __cdecl R_GetFogSettings(float *fogSettings);
void __cdecl R_SwitchFog(unsigned int localClientNum, unsigned int fogvar, int startTime, int transitionTime);
