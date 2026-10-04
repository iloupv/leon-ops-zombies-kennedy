#include "graph.h"
#include <universal/assertive.h>
#include <universal/q_shared.h>
#include <universal/com_loadutils.h>
#include <universal/q_parse.h>
#include "common.h"
#include <devgui/devgui.h>
#include <universal/com_files.h>

#include <cstring>

int gGraphsCount;
GraphFloat gGraphs[8];

void __cdecl GraphFloat_Load(GraphFloat *graph, char *fileName, float scale)
{
    const char *buffer; // [esp+0h] [ebp-400Ch]
    char loadBuffer[16388]; // [esp+4h] [ebp-4008h] BYREF

    if ( !graph && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\graph.cpp", 82, 0, "%s", "graph") )
        __debugbreak();
    if ( !fileName && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\graph.cpp", 83, 0, "%s", "fileName") )
        __debugbreak();
    Com_Memset((unsigned int *)graph, 0, 360);
    buffer = Com_LoadInfoString(fileName, "graph", "GRAPH_FLOAT_FILE", loadBuffer);
    GraphFloat_ParseBuffer(graph, buffer, fileName);
    graph->scale = scale;
}

void __cdecl GraphFloat_ParseBuffer(GraphFloat *graph, const char *buffer, const char *fileName)
{
    parseInfo_t *tokenb; // [esp+0h] [ebp-10h]
    parseInfo_t *token; // [esp+0h] [ebp-10h]
    parseInfo_t *tokena; // [esp+0h] [ebp-10h]
    int knotIndex; // [esp+4h] [ebp-Ch]
    float knots; // [esp+8h] [ebp-8h]
    float knots_4; // [esp+Ch] [ebp-4h]

    if ( !graph && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\graph.cpp", 33, 0, "%s", "graph") )
        __debugbreak();
    if ( !buffer && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\graph.cpp", 34, 0, "%s", "buffer") )
        __debugbreak();
    if ( !fileName && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\graph.cpp", 35, 0, "%s", "fileName") )
        __debugbreak();
    I_strncpyz(graph->name, fileName, 64);
    Com_BeginParseSession(fileName);
    tokenb = Com_Parse(&buffer);
    graph->knotCount = atoi(tokenb->token);
    for ( knotIndex = 0; ; ++knotIndex )
    {
        token = Com_Parse(&buffer);
        if ( !token->token[0] )
            break;
        if ( token->token[0] == 125 )
            break;
        knots = atof(token->token);
        tokena = Com_Parse(&buffer);
        if ( !tokena->token[0] || tokena->token[0] == 125 )
            break;
        knots_4 = atof(tokena->token);
        if ( knotIndex >= 32 )
            Com_Error(ERR_DROP, "GraphFloat_ParseBuffer: File [%s] has too many knots.  Max is [%d]", fileName, 32);
        graph->knots[knotIndex][0] = knots;
        graph->knots[knotIndex][1] = knots_4;
    }
    Com_EndParseSession();
    if ( knotIndex != graph->knotCount )
        Com_Error(ERR_DROP, "GraphFloat_ParseBuffer: Error parsing graph file [%s]", fileName);
}

GraphFloat *__cdecl GraphFloat_Load(char *fileName)
{
    char graphName[132]; // [esp+4h] [ebp-90h] BYREF
    GraphFloat *graph; // [esp+8Ch] [ebp-8h]
    int i; // [esp+90h] [ebp-4h]

    if ( !fileName || !*fileName )
        return 0;
    for ( i = 0; i < gGraphsCount; ++i )
    {
        graph = &gGraphs[i];
        if ( !I_stricmp(graph->name, fileName) )
            return graph;
    }
    if ( i == 8 )
        return 0;
    graph = &gGraphs[gGraphsCount++];
    GraphFloat_Load(graph, fileName, 1.0);
    Com_sprintf(graphName, 0x80u, "Graphs/%s", fileName);
    GraphFloat_CreateDevGui(graph, graphName);
    return graph;
}

double __cdecl GraphFloat_GetValue(const GraphFloat *graph, float fraction)
{
    float result; // [esp+18h] [ebp-4h]

    if ( !graph && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\graph.cpp", 145, 0, "%s", "graph") )
        __debugbreak();
    if ( (fraction < 0.0 || fraction > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\graph.cpp",
                    146,
                    0,
                    "fraction not in [0.0f, 1.0f]\n\t%g not in [%g, %g]",
                    fraction,
                    0.0,
                    1.0) )
    {
        __debugbreak();
    }
    result = GraphGetValueFromFraction(graph->knotCount, graph->knots, fraction);
    return (float)(result * graph->scale);
}

void __cdecl GraphFloat_CreateDevGui(GraphFloat *graph, const char *devguiPath)
{
    if ( !graph && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\graph.cpp", 248, 0, "%s", "graph") )
        __debugbreak();
    if ( !devguiPath
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\graph.cpp", 249, 0, "%s", "devguiPath") )
    {
        __debugbreak();
    }
    graph->devguiGraph.knotCountMax = 32;
    graph->devguiGraph.knots = graph->knots;
    graph->devguiGraph.knotCount = &graph->knotCount;
    graph->devguiGraph.eventCallback = (void (__cdecl *)(const DevGraph *, DevEventType, int))GraphFloat_DevGuiCB_Event;
    graph->devguiGraph.textCallback = (void (__cdecl *)(const DevGraph *, const float, const float, char *, const int))GraphFloat_DevGuiCB_Text;
    graph->devguiGraph.data = graph;
    DevGui_AddGraph(devguiPath, &graph->devguiGraph);
}

void __cdecl GraphFloat_DevGuiCB_Event(const DevGraph *graph, DevEventType event)
{
    if ( !graph && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\graph.cpp", 212, 0, "%s", "graph") )
        __debugbreak();
    if ( !graph->data
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\graph.cpp", 213, 0, "%s", "graph->data") )
    {
        __debugbreak();
    }
    if ( event == EVENT_SAVE )
        GraphFloat_SaveToFile((const GraphFloat *)graph->data);
}

void __cdecl GraphFloat_SaveToFile(const GraphFloat *graph)
{
    char buffer[1028]; // [esp+30h] [ebp-418h] BYREF
    int knotCount; // [esp+438h] [ebp-10h]
    int fileHandle; // [esp+43Ch] [ebp-Ch]
    const char *basePath; // [esp+440h] [ebp-8h]
    int knotIndex; // [esp+444h] [ebp-4h]

    if ( !graph && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\graph.cpp", 167, 0, "%s", "graph") )
        __debugbreak();
    fileHandle = FS_FOpenTextFileWrite((char*)graph->name);
    if ( fileHandle )
    {
        if ( !graph->knotCount
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\graph.cpp", 181, 0, "%s", "graph->knotCount") )
        {
            __debugbreak();
        }
        knotCount = graph->knotCount;
        Com_sprintf(buffer, 0x400u, "%s\n\n%d\n", "GRAPH_FLOAT_FILE", knotCount);
        FS_Write(buffer, &buffer[strlen(buffer) + 1] - &buffer[1], fileHandle);
        for ( knotIndex = 0; knotIndex < knotCount; ++knotIndex )
        {
            Com_sprintf(buffer, 0x400u, "%.4f %.4f\n", graph->knots[knotIndex][0], graph->knots[knotIndex][1]);
            FS_Write(buffer, &buffer[strlen(buffer) + 1] - &buffer[1], fileHandle);
        }
        FS_FCloseFile(fileHandle);
        basePath = Sys_DefaultInstallPath();
        Com_Printf(18, "^7GraphFloat_SaveToFile: Successfully saved file [%s\\%s].\n", basePath, graph->name);
    }
    else
    {
        Com_PrintError(0, "GraphFloat_SaveToFile: Could not save file [%s].\n", graph->name);
    }
}

void __cdecl GraphFloat_DevGuiCB_Text(const DevGraph *devGuiGraph, float inputX, float inputY, char *text)
{
    sprintf(text, "Fraction: %.3f, Value: %.3f", inputX, (float)(inputY * *((float *)devGuiGraph->data + 81)));
}

