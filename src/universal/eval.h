#pragma once

enum EvalOperatorType : __int32
{                                       // XREF: Eval/r
                                        // ?Eval_PushOperator@@YA_NPAUEval@@W4EvalOperatorType@@@Z/r ...
    EVAL_OP_LPAREN        = 0x0,
    EVAL_OP_RPAREN        = 0x1,
    EVAL_OP_COLON         = 0x2,
    EVAL_OP_QUESTION      = 0x3,
    EVAL_OP_PLUS          = 0x4,
    EVAL_OP_MINUS         = 0x5,
    EVAL_OP_UNARY_PLUS    = 0x6,
    EVAL_OP_UNARY_MINUS   = 0x7,
    EVAL_OP_MULTIPLY      = 0x8,
    EVAL_OP_DIVIDE        = 0x9,
    EVAL_OP_MODULUS       = 0xA,
    EVAL_OP_LSHIFT        = 0xB,
    EVAL_OP_RSHIFT        = 0xC,
    EVAL_OP_BITWISE_NOT   = 0xD,
    EVAL_OP_BITWISE_AND   = 0xE,
    EVAL_OP_BITWISE_OR    = 0xF,
    EVAL_OP_BITWISE_XOR   = 0x10,
    EVAL_OP_LOGICAL_NOT   = 0x11,
    EVAL_OP_LOGICAL_AND   = 0x12,
    EVAL_OP_LOGICAL_OR    = 0x13,
    EVAL_OP_EQUALS        = 0x14,
    EVAL_OP_NOT_EQUAL     = 0x15,
    EVAL_OP_LESS          = 0x16,
    EVAL_OP_LESS_EQUAL    = 0x17,
    EVAL_OP_GREATER       = 0x18,
    EVAL_OP_GREATER_EQUAL = 0x19,
    EVAL_OP_COUNT         = 0x1A,
};

enum EvalValueType : __int32
{                                       // XREF: EvalValue/r
    EVAL_VALUE_DOUBLE = 0x0,
    EVAL_VALUE_INT    = 0x1,
    EVAL_VALUE_STRING = 0x2,
};

struct EvalValue // sizeof=0x10
{                                       // XREF: Eval/r
                                        // PC_Int_Expression_Parse/r ...
    EvalValueType type;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    //EvalValue::<unnamed_type_u> u;
    union //EvalValue::<unnamed_type_u> // sizeof=0x8
    {                                       // XREF: EvalValue/r
        long double d;
        int i;
        char *s;
    } u;
};

struct __declspec(align(4)) Eval // sizeof=0x5010
{                                       // XREF: PC_Int_Expression_Parse/r
                                        // PC_Float_Expression_Parse/r
    EvalOperatorType opStack[1024];
    EvalValue valStack[1024];
    int opStackPos;                     // XREF: PC_Int_Expression_Parse+17/w
                                        // PC_Float_Expression_Parse+17/w
    int valStackPos;                    // XREF: PC_Int_Expression_Parse+21/w
                                        // PC_Float_Expression_Parse+21/w
    int parenCount;                     // XREF: PC_Int_Expression_Parse+2B/w
                                        // PC_Float_Expression_Parse+2B/w
    bool pushedOp;                      // XREF: PC_Int_Expression_Parse+35/w
                                        // PC_Float_Expression_Parse+35/w
    // padding byte
    // padding byte
    // padding byte
};

bool __cdecl Eval_AnyMissingOperands(const Eval *eval);
char    Eval_PushOperator(Eval *eval, EvalOperatorType op);
bool    Eval_EvaluationStep(Eval *eval);
void __cdecl Eval_PrepareBinaryOpSameTypes(Eval *eval);
void __cdecl Eval_PrepareBinaryOpIntegers(Eval *eval);
void __cdecl Eval_PrepareBinaryOpBoolean(Eval *eval);
bool __cdecl Eval_IsUnaryOp(const Eval *eval);
char __cdecl Eval_PushInteger(Eval *eval, int value);
bool __cdecl Eval_CanPushValue(const Eval *eval);
char __cdecl Eval_PushNumber(Eval *eval, long double value);
EvalValue * Eval_Solve(EvalValue *result, Eval *eval);
bool __cdecl Eval_OperatorForToken(const char *text, EvalOperatorType *op);
