#pragma once

#include "cscr_main.h"
#include "cscr_variable.h"
#include <cstddef>

#define SL_MAX_STRING_INDEX 0x10000

#define MAX_XANIMTREE_NUM       0x80 // 128

struct XAnim_s;

struct scr_anim_s // sizeof=0x4
{                                                                             // XREF: bgsAnim_s::<unnamed_type_generic_human>/r
        //$3FF2EDC1867ACEA597C8BFA34C9C0852 ___u0;
                                                                                // XREF: CScr_GetAnimLength(void)+20/w
                                                                                // CScr_GetAnimLength(void)+25/r ...
    union //$3FF2EDC1867ACEA597C8BFA34C9C0852 // sizeof=0x4
    {                                                                             // XREF: CScr_GetAnimLength(void)+20/w
                                                                                    // CScr_GetAnimLength(void)+25/r ...
            //$6CB7272563F4458FB40A4A5E123C4ABA __s0;
        struct //$6CB7272563F4458FB40A4A5E123C4ABA // sizeof=0x4
        {                                                                             // XREF: $3FF2EDC1867ACEA597C8BFA34C9C0852/r
            unsigned __int16 index;
            unsigned __int16 tree;
        };
        const char *linkPointer;
    };

    scr_anim_s()
    {
        linkPointer = NULL;
    }
    scr_anim_s(int i)
    {
        linkPointer = (const char *)i; // BO1HACK
    }
};

struct scr_animtree_t // sizeof=0x4
{                                                                             // XREF: animScriptData_t/r
    scr_animtree_t(XAnim_s *_anims)
    {
        anims = _anims;
    }
    scr_animtree_t(int val)
    {
        anims = (XAnim_s *)val;
    }
    scr_animtree_t()
    {
        anims = NULL; 
    }
    XAnim_s *anims;                                         // XREF: BG_FindAnimTree+1A/w
};

struct __declspec(align(4)) scr_animscript_t // sizeof=0x8
{                                       // XREF: actor_s/r
                                        // AnimScriptList/r ...
    int func;
    unsigned __int16 name;
    // padding byte
    // padding byte
};

struct scrAnimGlob_t // sizeof=0x40C
{
    const char *start;
    const char *pos;
    unsigned int using_xanim_lookup[2][MAX_XANIMTREE_NUM];
    int bAnimCheck;
};

struct __declspec(align(4)) scrAnimPub_t // sizeof=0x41C
{                                       // XREF: .data:scrAnimPub_t * gScrAnimPub/r
    unsigned int animtrees;             // XREF: Scr_UsingTreeInternal+99/r
                                        // Scr_UsingTreeInternal+C6/r ...
    unsigned int animtree_node;         // XREF: Scr_LoadAnimTreeAtIndex(scriptInstance_t,uint,void * (*)(int),int,bool)+284/r
                                        // Scr_LoadAnimTreeAtIndex(scriptInstance_t,uint,void * (*)(int),int,bool)+2C8/w ...
    unsigned int animTreeNames;         // XREF: Scr_EmitAnimation(scriptInstance_t,char *,uint,uint)+C/r
                                        // Scr_EmitAnimation(scriptInstance_t,char *,uint,uint)+39/r ...
    scr_animtree_t xanim_lookup[2][MAX_XANIMTREE_NUM];
                                        // XREF: GScr_GetAnimTreesLoaded+3D/r
                                        // GScr_FindAnimByName+D0/r ...
    unsigned int xanim_num[2];          // XREF: GScr_GetAnimTreesLoaded+31/r
                                        // GScr_FindAnimByName+BE/r ...
    unsigned int animTreeIndex;         // XREF: Scr_UsingTree(scriptInstance_t,char const *,uint)+6F/o
                                        // Scr_ClientUsingTree(scriptInstance_t,char const *)+23/o ...
    bool animtree_loading;              // XREF: Scr_UsingTreeInternal+F/r
                                        // Scr_LoadAnimTreeAtIndex(scriptInstance_t,uint,void * (*)(int),int,bool)+33/r ...
    // padding byte
    // padding byte
    // padding byte
};

struct ScriptTokenizer // sizeof=0x2808
{                                       // XREF: AnimTreeParseInternal/r
    enum TokenType : __int32
    {                                       // XREF: ?ParseIntoTokens@ScriptTokenizer@@AAEXPBD@Z/r
        TOK_IDENTIFIER = 0x0,
        TOK_SYMBOL     = 0x1,
        TOK_NUMBER     = 0x2,
        TOK_UNKNOWN    = 0x3,
    };
    scriptInstance_t inst;              // XREF: AnimTreeParseInternal+10/w
    int m_iNumInStack;                  // XREF: AnimTreeParseInternal+16/w
    char m_stack[40][256];

    char *__thiscall PopToken();
    void __thiscall  PushNextToken();
    char *__thiscall GetAndRemoveToken();
    void __thiscall  ParseIntoTokens(char *strString);
    char *__thiscall PeekToken();
    bool __thiscall  IsAtEnd();
};

struct TreeNameMap // sizeof=0x54
{
    int animIndex;
    char strName[80];
};

struct ClientTreeStorage // sizeof=0x10
{                                       // XREF: .data:gGScrXAnimTreesForClient/r
    char *strName;                      // XREF: Scr_LoadAnimTreeAtIndex(scriptInstance_t,uint,void * (*)(int),int,bool)+69A/w
                                        // Scr_LoadAnimTreeAtIndex(scriptInstance_t,uint,void * (*)(int),int,bool)+6EE/r ...
    scr_animtree_t animTree;            // XREF: Scr_LoadAnimTreeAtIndex(scriptInstance_t,uint,void * (*)(int),int,bool)+634/w
                                        // CScr_RetrieveAnimTree+244/r
    int numIndices;
    TreeNameMap *pTreeNameMap;
};


XExpr::MathTypes __cdecl EqualTypeSameResult(const ParseValue *params, int iNumParams, scriptInstance_t inst);
void __cdecl AnimTreeCompileError(scriptInstance_t inst, const char *msg);
XExpr::MathTypes __cdecl EqualTypeAllowScalar(const ParseValue *params, int iNumParams, scriptInstance_t inst);
XExpr::MathTypes __cdecl FloatsToVec(const ParseValue *params, int iNumParams, scriptInstance_t inst);
XExpr::MathTypes __cdecl VecsToFloat(const ParseValue *params, int iNumParams, scriptInstance_t inst);
XExpr::MathTypes __cdecl Vec3ToVec3(const ParseValue *params, int iNumParams, scriptInstance_t inst);
void __cdecl SetAnimCheck(int bAnimCheck, scriptInstance_t inst);
void __cdecl Scr_EmitAnimation(scriptInstance_t inst, char *pos, unsigned int animName, unsigned int sourcePos);
void __cdecl Scr_EmitAnimationInternal(scriptInstance_t inst, char *pos, unsigned int animName, unsigned int names);
void __cdecl SetVariableValue(scriptInstance_t inst, unsigned int id, VariableValue *value);
void __cdecl RemoveRefToValue(scriptInstance_t inst, int type, VariableUnion u);
void __cdecl RemoveRefToVector(scriptInstance_t inst, const float *vectorValue);
unsigned int __cdecl StoreExprInList(scriptInstance_t inst, unsigned int exprDataArray, ParseValue exprData);
void __cdecl SetIntVariable(scriptInstance_t inst, unsigned int theArray, int integerValue, char *strVarName);
void __cdecl SetPtrVariable(scriptInstance_t inst, unsigned int theArray, unsigned int arrayData, char *strVarName);
unsigned int __cdecl StoreValueInList(scriptInstance_t inst, unsigned int valueDataArray, unsigned int valueData);
unsigned int __cdecl ParseValueFromValueList(ScriptTokenizer *tokenizer, scriptInstance_t inst, unsigned int valueName);
void __cdecl SetFloatVariable(scriptInstance_t inst, unsigned int theArray, float floatValue, char *strVarName);
int __cdecl DetermineValueType(ScriptTokenizer *tokenizer);
int __cdecl Scr_GetAnimsIndex(const XAnim_s *anims, scriptInstance_t inst);
XAnim_s *__cdecl Scr_GetClientAnims(unsigned int index, scriptInstance_t inst);
XAnim_s *__cdecl Scr_GetAnims(unsigned int index, scriptInstance_t inst);
void __cdecl Scr_UsingTree(scriptInstance_t inst, const char *filename, unsigned int sourcePos);
unsigned int __cdecl Scr_UsingTreeInternal(scriptInstance_t inst, const char *filename, unsigned int *index, int user);
void __cdecl Scr_ClientUsingTree(scriptInstance_t inst, const char *filename);
void __cdecl Scr_LoadAnimTreeAtIndex(
                scriptInstance_t inst,
                unsigned int index,
                void *(__cdecl *Alloc)(int),
                int user,
                bool modCheckSum);
unsigned int __cdecl GetVariableName(scriptInstance_t inst, unsigned int id);
int __cdecl Scr_GetAnimTreeSize(scriptInstance_t inst, unsigned int parentNode);
unsigned int __cdecl FindFirstSibling(scriptInstance_t inst, unsigned int id);
unsigned int __cdecl FindNextSibling(scriptInstance_t inst, unsigned int id);
unsigned int __cdecl Scr_GetAnimTreeValueCount(scriptInstance_t inst, unsigned int rootData);
unsigned int __cdecl GetNamedArrayVarArray(scriptInstance_t inst, unsigned int theArray, char *strVarName);
void __cdecl ConnectScriptToAnim(
                scriptInstance_t inst,
                unsigned int names,
                int index,
                unsigned int filename,
                unsigned int name,
                unsigned __int16 treeIndex);
void __cdecl Scr_CheckAnimsDefined(scriptInstance_t inst, unsigned int names, unsigned int filename);
void __cdecl Scr_PrecacheAnimationTree(scriptInstance_t inst, unsigned int parentNode);
bool __cdecl Scr_LoadAnimTreeInternal(
                scriptInstance_t inst,
                const char *filename,
                unsigned int parentNode,
                unsigned int names);
void __cdecl Scr_AnimTreeParse(scriptInstance_t inst, const char *pos, unsigned int parentNode, unsigned int names);
char __cdecl AnimTreeParseInternal(
                scriptInstance_t inst,
                unsigned int parentNode,
                unsigned int names,
                bool bIncludeParent,
                bool bLoop,
                bool bComplete);
char __cdecl ParseNode(
                ScriptTokenizer *tokenizer,
                scriptInstance_t inst,
                unsigned int parentNode,
                unsigned int rootData,
                unsigned int names,
                bool bLoop,
                bool bComplete);
ParseValue __cdecl ParseTopRankExpr(ScriptTokenizer *tokenizer, scriptInstance_t inst, unsigned int rootData);
ExpressionParseTypes __cdecl DetermineParseType(ScriptTokenizer *tokenizer);
bool __cdecl IsStandardParam(const char *strParamName);
int __cdecl GetStdParamIx(const char *strParamName);
ParseValue __cdecl BuildExpr(ExpressionParseTypes eCurrType, ParseValue *params, int iNumParams, scriptInstance_t inst);
void __cdecl CreateParameterVarName(char *strOutString, unsigned int iMaxNumChars, int iOperandIx);
ParseValue __cdecl ParseRank2Expr(ScriptTokenizer *tokenizer, scriptInstance_t inst, unsigned int rootData);
ParseValue __cdecl ParseBottomRankExpr(ScriptTokenizer *tokenizer, scriptInstance_t inst, unsigned int rootData);
const char *__cdecl ConvertParseTypeToStr(ExpressionParseTypes eType);
void __cdecl RequireParseTypePopOrError(ScriptTokenizer *tokenizer, scriptInstance_t inst, ExpressionParseTypes eTest);
ParseValue __cdecl ParseFunctionCall(
                ExpressionParseTypes eFuncType,
                ScriptTokenizer *tokenizer,
                scriptInstance_t inst,
                unsigned int rootData);
ParseValue __cdecl ParseStdParameter(ScriptTokenizer *tokenizer, scriptInstance_t inst);
ParseValue __cdecl ParseParameter(ScriptTokenizer *tokenizer, scriptInstance_t inst, unsigned int rootData);
unsigned int __cdecl GetNamedArrayVarInt(scriptInstance_t inst, unsigned int theArray, char *strVarName);
int __cdecl GetAnimTreeParseProperties(ScriptTokenizer *tokenizer);
void __cdecl StoreOptionsInNode(
                scriptInstance_t inst,
                unsigned int currentAnimArray,
                unsigned int iCurrOptionIx,
                unsigned int optionName,
                unsigned int iEntryIndex);
void __cdecl StoreValuesInNode(
                scriptInstance_t inst,
                unsigned int currentAnimArray,
                unsigned int iCurrIx,
                unsigned int iEntryIndex);
void __cdecl ParseParameterDef(ScriptTokenizer *tokenizer, scriptInstance_t inst, unsigned int rootData);
int __cdecl Scr_CreateAllAnimTreeData(
                scriptInstance_t inst,
                unsigned int rootData,
                unsigned int names,
                scr_animtree_t *animtree,
                unsigned int filenameId,
                unsigned int index,
                void *(__cdecl *Alloc)(int));
int __cdecl Scr_CreateAnimationTree(
                scriptInstance_t inst,
                unsigned int parentNode,
                unsigned int rootData,
                unsigned int names,
                XAnim_s *anims,
                unsigned int childIndex,
                const char *parentName,
                unsigned int parentIndex,
                unsigned int filename,
                int treeIndex,
                unsigned __int16 flags,
                void *(__cdecl *Alloc)(int),
                unsigned int paramMap);
void __cdecl Scr_ProcessNodeValues(
                scriptInstance_t inst,
                XAnim_s *anims,
                unsigned int name,
                unsigned int rootNode,
                unsigned int animIndex,
                unsigned int arrayNode);
void __cdecl CreateValues(scriptInstance_t inst, XAnim_s *anims, unsigned int valueArray);
double __cdecl GetNamedArrayVarFloat(scriptInstance_t inst, unsigned int theArray, char *strVarName);
scr_animtree_t __cdecl CScr_RetrieveAnimTree(
                const char *strAnimTreeName,
                unsigned int names,
                unsigned int filename,
                unsigned __int16 iOurTreeIx);
scr_animtree_t __cdecl Scr_FindAnimTree(scriptInstance_t inst, const char *filename);
VariableValue __cdecl Scr_EvalVariable(scriptInstance_t inst, unsigned int id);
void __cdecl AddRefToValue(scriptInstance_t inst, int type, VariableUnion u);
void __cdecl AddRefToObject(scriptInstance_t inst, unsigned int id);
void __cdecl AddRefToVector(scriptInstance_t inst, const float *vectorValue);
void __cdecl Scr_FindAnim(
                scriptInstance_t inst,
                const char *filename,
                const char *animName,
                scr_anim_s *anim,
                int user);
void __cdecl Scr_ShutdownAnimTrees(int user);
void __cdecl RemoveRefToValue(scriptInstance_t inst, VariableValue *value);

extern scrAnimGlob_t gScrAnimGlob[2];
extern scrAnimPub_t gScrAnimPub[2];