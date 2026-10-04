#include "eval.h"
#include "assertive.h"
#include <cstdlib>
#include <string.h>
#include "q_shared.h"

// probably one of the stinkiest parts of the codebase

#define _CxxThrowException(a, b) iassert(0);

int s_consumedOperandCount[26] =
{
  1,
  -1,
  2,
  0,
  1,
  1,
  0,
  0,
  1,
  1,
  1,
  1,
  1,
  0,
  1,
  1,
  1,
  0,
  1,
  1,
  1,
  1,
  1,
  1,
  1,
  1
};

char s_precedence[26] =
{
  'c',
  '\0',
  '\x03',
  '\x02',
  '\v',
  '\v',
  '\r',
  '\r',
  '\f',
  '\f',
  '\f',
  '\n',
  '\n',
  '\r',
  '\a',
  '\x05',
  '\x06',
  '\r',
  '\x02',
  '\x01',
  '\b',
  '\b',
  '\t',
  '\t',
  '\t',
  '\t'
};

bool s_rightToLeft[26] =
{
  false,
  false,
  true,
  true,
  false,
  false,
  true,
  true,
  false,
  false,
  false,
  false,
  false,
  true,
  false,
  false,
  false,
  true,
  false,
  false,
  false,
  false,
  false,
  false,
  false,
  false
};

bool __cdecl Eval_AnyMissingOperands(const Eval *eval)
{
    int opIndex; // [esp+0h] [ebp-8h]
    int requiredOperandCount; // [esp+4h] [ebp-4h]

    requiredOperandCount = 1;
    for ( opIndex = 0; opIndex < eval->opStackPos; ++opIndex )
        requiredOperandCount += s_consumedOperandCount[eval->opStack[opIndex]];
    return requiredOperandCount != eval->valStackPos;
}

char    Eval_PushOperator(Eval *eval, EvalOperatorType op)
{
    bool v4; // [esp+0h] [ebp-10h]
    const char *v5; // [esp+4h] [ebp-Ch] BYREF
    const char *pExceptionObject; // [esp+8h] [ebp-8h] BYREF
    bool leftToRight; // [esp+Dh] [ebp-3h]
    char precedence; // [esp+Eh] [ebp-2h]
    bool higherPrecedence; // [esp+Fh] [ebp-1h]
    int savedregs; // [esp+10h] [ebp+0h] BYREF

    if ( s_precedence[op] < 0 )
        return 0;
    if ( op == EVAL_OP_RPAREN && !eval->parenCount )
        return 0;
    if ( op == EVAL_OP_LPAREN )
    {
        if ( eval->valStackPos && !eval->pushedOp )
            return 0;
        ++eval->parenCount;
    }
    if ( op == EVAL_OP_PLUS )
    {
        if ( Eval_IsUnaryOp(eval) )
            op = EVAL_OP_UNARY_PLUS;
    }
    else if ( op == EVAL_OP_MINUS && Eval_IsUnaryOp(eval) )
    {
        op = EVAL_OP_UNARY_MINUS;
    }
    if ( (unsigned int)op >= EVAL_OP_COUNT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\eval.cpp",
                    549,
                    0,
                    "%s",
                    "op >= 0 && op < ARRAY_COUNT( s_precedence )") )
    {
        __debugbreak();
    }
    precedence = s_precedence[op];
    while ( eval->opStackPos > 0 )
    {
        higherPrecedence = s_precedence[eval->opStack[eval->opStackPos - 1]] > precedence;
        v4 = s_precedence[eval->opStack[eval->opStackPos - 1]] == precedence
            && !s_rightToLeft[eval->opStack[eval->opStackPos - 1]];
        leftToRight = v4;
        if ( !higherPrecedence && !leftToRight )
            break;
        if ( eval->opStack[eval->opStackPos - 1] == EVAL_OP_LPAREN )
        {
            if ( op == EVAL_OP_RPAREN )
            {
                --eval->parenCount;
                --eval->opStackPos;
                eval->pushedOp = 0;
                return 1;
            }
            break;
        }
        if ( !Eval_EvaluationStep(eval) )
            return 0;
    }
    if ( op != EVAL_OP_COLON || eval->opStackPos && eval->opStack[eval->opStackPos - 1] == EVAL_OP_QUESTION )
    {
        if ( eval->opStackPos == 1024 )
        {
            v5 = "evaluation stack overflow - expression is too complex";
            _CxxThrowException(&v5, &PA.deinit);
        }
        eval->opStack[eval->opStackPos++] = op;
        eval->pushedOp = 1;
        return 1;
    }
    else
    {
        if ( eval->parenCount )
        {
            pExceptionObject = "found ':' without preceding '?' in expression of type 'a ? b : c'";
            _CxxThrowException(&pExceptionObject, &PA.deinit);
        }
        return 0;
    }
}

// local variable allocation has failed, the output may be wrong!
bool    Eval_EvaluationStep(Eval *eval)
{
    bool result; // al
    int v4; // esi
    int v5; // esi
    EvalValue *v6; // edx
    EvalOperatorType *v7; // ecx
    long double v9; // [esp+10h] [ebp-B4h]
    const char *v10; // [esp+1Ch] [ebp-A8h] BYREF
    int i; // [esp+20h] [ebp-A4h]
    int v12; // [esp+24h] [ebp-A0h]
    const char *v13; // [esp+28h] [ebp-9Ch] BYREF
    EvalOperatorType v14; // [esp+2Ch] [ebp-98h]
    EvalOperatorType v15; // [esp+30h] [ebp-94h]
    EvalOperatorType v16; // [esp+34h] [ebp-90h]
    EvalOperatorType v17; // [esp+38h] [ebp-8Ch]
    EvalOperatorType v18; // [esp+3Ch] [ebp-88h]
    int v19; // [esp+40h] [ebp-84h]
    EvalOperatorType v20; // [esp+44h] [ebp-80h]
    bool same; // [esp+4Bh] [ebp-79h]
    const char *v22; // [esp+4Ch] [ebp-78h] BYREF
    long double dQuotientFloor; // [esp+50h] [ebp-74h]
    const char *v24; // [esp+5Ch] [ebp-68h] BYREF
    const char *v25; // [esp+60h] [ebp-64h] BYREF
    const char *v26; // [esp+64h] [ebp-60h] BYREF
    char *s; // [esp+68h] [ebp-5Ch]
    int v28; // [esp+6Ch] [ebp-58h]
    const char *v29; // [esp+74h] [ebp-50h]
    const char *v30; // [esp+78h] [ebp-4Ch]
    const char *v31; // [esp+7Ch] [ebp-48h]
    int length[2]; // [esp+80h] [ebp-44h]
    unsigned int v33; // [esp+88h] [ebp-3Ch]
    const char *v34; // [esp+90h] [ebp-34h]
    const char *v35; // [esp+94h] [ebp-30h]
    const char *v36; // [esp+98h] [ebp-2Ch]
    const char *v37; // [esp+9Ch] [ebp-28h] BYREF
    const char *v38; // [esp+A0h] [ebp-24h] BYREF
    EvalOperatorType v39; // [esp+A4h] [ebp-20h]
    const char *v40; // [esp+A8h] [ebp-1Ch] BYREF
    __int32 v41; // [esp+ACh] [ebp-18h]
    const char *v42; // [esp+B0h] [ebp-14h] BYREF
    const char *v43; // [esp+B4h] [ebp-10h] BYREF
    //_UNKNOWN *v44; // [esp+B8h] [ebp-Ch]
    //Eval *evala; // [esp+BCh] [ebp-8h]
    //int vars0; // [esp+C4h] [ebp+0h]
    //
    //v44 = a1;
    //evala = (Eval *)vars0;
    if (!*(_DWORD *)(eval + 20480))
        return 0;
    --*(_DWORD *)(eval + 20480);
    if (!*(_DWORD *)(eval + 4 * *(_DWORD *)(eval + 20480)))
        return 1;
    if (*(_DWORD *)(eval + 4 * *(_DWORD *)(eval + 20480)) == 3)
    {
        v43 = "found '?' with no following ':' in expression of type 'a ? b : c'";
        _CxxThrowException(&v43, &PA.deinit);
    }
    if (!*(_DWORD *)(eval + 20484))
    {
        v42 = "missing operand (for example, 'a + ' or ' / b')";
        _CxxThrowException(&v42, &PA.deinit);
    }
    v41 = *(_DWORD *)(eval + 4 * *(_DWORD *)(eval + 20480)) - 2;
    switch (v41)
    {
    case 0:
        if (*(int *)(eval + 20484) < 3)
        {
            v13 = "missing operand (for example, 'a + ' or ' / b')";
            _CxxThrowException(&v13, &PA.deinit);
        }
        if (*(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 3) + 4096))
        {
            if (*(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 3) + 4096) != 1)
            {
                v10 = "can only switch on numbers";
                _CxxThrowException(&v10, &PA.deinit);
            }
            i = -(*(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 3) + 4104) != 0) - 1;
        }
        else
        {
            if (*(double *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 3) + 4104) == 0.0)
                v12 = -1;
            else
                v12 = -2;
            i = v12;
        }
        if (*(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4096) == 2
            && *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4096) == 2)
        {
            free(*(void **)(eval + 16 * (1 - i + *(_DWORD *)(eval + 20484)) + 4104));
        }
        else
        {
            Eval_PrepareBinaryOpSameTypes((Eval *)eval);
        }
        v6 = (EvalValue *)(eval + 16 * (i + *(_DWORD *)(eval + 20484)) + 4096);
        v7 = (EvalOperatorType *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 3) + 4096);
        *v7 = (EvalOperatorType)v6->type;
        v7[1] = *((EvalOperatorType *)&v6->type + 1);
        v7[2] = (EvalOperatorType)v6->u.i;
        v7[3] = *((EvalOperatorType *)&v6->u.s + 1);
        *(_DWORD *)(eval + 20484) -= 2;
        --*(_DWORD *)(eval + 20480);
        goto LABEL_121;
    case 2:
        if (*(int *)(eval + 20484) >= 2
            && *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4096) == 2
            && *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4096) == 2)
        {
            v36 = *(const char **)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104);
            v34 = v36 + 1;
            v35 = &v36[strlen(v36) + 1];
            v33 = v35 - (v36 + 1);
            length[0] = v33;
            v31 = *(const char **)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4104);
            v29 = v31 + 1;
            v30 = &v31[strlen(v31) + 1];
            v28 = v30 - (v31 + 1);
            length[1] = v28;
            s = (char *)malloc(v33 + v30 - v31);
            memcpy((unsigned __int8 *)s, *(unsigned __int8 **)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104), v33);
            memcpy(
                (unsigned __int8 *)&s[v33],
                *(unsigned __int8 **)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4104),
                v30 - v31);
            free(*(void **)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104));
            free(*(void **)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4104));
            *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) = (_DWORD)s;
        }
        else
        {
            Eval_PrepareBinaryOpSameTypes((Eval *)eval);
            if (*(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4096))
                *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) += *(_DWORD *)(eval
                    + 16
                    * (*(_DWORD *)(eval + 20484) - 1)
                    + 4104);
            else
                *(double *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) = *(double *)(eval
                    + 16
                    * (*(_DWORD *)(eval + 20484) - 2)
                    + 4104)
                + *(double *)(eval
                    + 16
                    * (*(_DWORD *)(eval + 20484) - 1)
                    + 4104);
        }
        -- * (_DWORD *)(eval + 20484);
        goto LABEL_121;
    case 3:
        Eval_PrepareBinaryOpSameTypes((Eval *)eval);
        if (*(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4096))
            *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) -= *(_DWORD *)(eval
                + 16
                * (*(_DWORD *)(eval + 20484) - 1)
                + 4104);
        else
            *(double *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) = *(double *)(eval
                + 16
                * (*(_DWORD *)(eval + 20484) - 2)
                + 4104)
            - *(double *)(eval
                + 16
                * (*(_DWORD *)(eval + 20484) - 1)
                + 4104);
        --*(_DWORD *)(eval + 20484);
        goto LABEL_121;
    case 4:
        goto LABEL_121;
    case 5:
        if (*(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4096) == 1)
        {
            *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4104) = -*(int *)(eval
                + 16
                * (*(_DWORD *)(eval + 20484) - 1)
                + 4104);
        }
        else
        {
            if (*(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4096))
            {
                v40 = "cannot negate strings";
                _CxxThrowException(&v40, &PA.deinit);
            }
            *(double *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4104) = -*(double *)(eval
                + 16
                * (*(_DWORD *)(eval + 20484) - 1)
                + 4104);
        }
        goto LABEL_121;
    case 6:
        Eval_PrepareBinaryOpSameTypes((Eval *)eval);
        if (*(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4096))
            *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) *= *(_DWORD *)(eval
                + 16
                * (*(_DWORD *)(eval + 20484) - 1)
                + 4104);
        else
            *(double *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) = *(double *)(eval
                + 16
                * (*(_DWORD *)(eval + 20484) - 2)
                + 4104)
            * *(double *)(eval
                + 16
                * (*(_DWORD *)(eval + 20484) - 1)
                + 4104);
        --*(_DWORD *)(eval + 20484);
        goto LABEL_121;
    case 7:
        Eval_PrepareBinaryOpSameTypes((Eval *)eval);
        if (*(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4096))
        {
            if (!*(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4104))
            {
                v25 = "divide by zero";
                _CxxThrowException(&v25, &PA.deinit);
            }
            *(int *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) /= *(int *)(eval
                + 16 * (*(_DWORD *)(eval + 20484) - 1)
                + 4104);
        }
        else
        {
            if (*(double *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4104) == 0.0)
            {
                v26 = "divide by zero";
                _CxxThrowException(&v26, &PA.deinit);
            }
            *(double *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) = *(double *)(eval
                + 16
                * (*(_DWORD *)(eval + 20484) - 2)
                + 4104)
                / *(double *)(eval
                    + 16
                    * (*(_DWORD *)(eval + 20484) - 1)
                    + 4104);
        }
        -- * (_DWORD *)(eval + 20484);
        goto LABEL_121;
    case 8:
        Eval_PrepareBinaryOpSameTypes((Eval *)eval);
        if (*(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4096))
        {
            if (!*(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4104))
            {
                v22 = "divide by zero";
                _CxxThrowException(&v22, &PA.deinit);
            }
            *(int *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) %= *(_DWORD *)(eval
                + 16 * (*(_DWORD *)(eval + 20484) - 1)
                + 4104);
        }
        else
        {
            if (*(double *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4104) == 0.0)
            {
                v24 = "divide by zero";
                _CxxThrowException(&v24, &PA.deinit);
            }
            dQuotientFloor = floor(
                *(double *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104)
                / *(double *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4104));
            *(long double *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) = *(double *)(eval
                + 16
                * (*(_DWORD *)(eval + 20484) - 2)
                + 4104)
                - *(double *)(eval
                    + 16
                    * (*(_DWORD *)(eval + 20484) - 1)
                    + 4104)
                * dQuotientFloor;
        }
        -- * (_DWORD *)(eval + 20484);
        goto LABEL_121;
    case 9:
        Eval_PrepareBinaryOpSameTypes((Eval *)eval);
        if (*(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4096))
        {
            *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) <<= *(_DWORD *)(eval
                + 16
                * (*(_DWORD *)(eval + 20484) - 1)
                + 4104);
        }
        else
        {
            v4 = 16 * (*(_DWORD *)(eval + 20484) - 2);
            //__libm_sse2_pow(a2, v9);
            v4 = pow(v4, 2.0);
            *(double *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) = 2.0 * *(double *)(eval + v4 + 4104);
        }
        -- * (_DWORD *)(eval + 20484);
        goto LABEL_121;
    case 10:
        Eval_PrepareBinaryOpSameTypes((Eval *)eval);
        if (*(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4096))
        {
            *(int *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) >>= *(_DWORD *)(eval
                + 16
                * (*(_DWORD *)(eval + 20484) - 1)
                + 4104);
        }
        else
        {
            v5 = 16 * (*(_DWORD *)(eval + 20484) - 2);
            //__libm_sse2_pow(a2, v9);
            v5 = pow(v5, 2.0);
            *(double *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) = 2.0 * *(double *)(eval + v5 + 4104);
        }
        -- * (_DWORD *)(eval + 20484);
        goto LABEL_121;
    case 11:
        if (*(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4096))
        {
            if (*(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4096) == 2)
            {
                v37 = "cannot bitwise invert strings";
                _CxxThrowException(&v37, &PA.deinit);
            }
        }
        else
        {
            *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4104) = (int)*(double *)(eval
                + 16
                * (*(_DWORD *)(eval + 20484) - 1)
                + 4104);
            *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4096) = 1;
        }
        *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4104) = ~*(_DWORD *)(eval
            + 16
            * (*(_DWORD *)(eval + 20484) - 1)
            + 4104);
        goto LABEL_121;
    case 12:
        Eval_PrepareBinaryOpIntegers((Eval *)eval);
        *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) &= *(_DWORD *)(eval
            + 16
            * (*(_DWORD *)(eval + 20484) - 1)
            + 4104);
        --*(_DWORD *)(eval + 20484);
        goto LABEL_121;
    case 13:
        Eval_PrepareBinaryOpIntegers((Eval *)eval);
        *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) |= *(_DWORD *)(eval
            + 16
            * (*(_DWORD *)(eval + 20484) - 1)
            + 4104);
        --*(_DWORD *)(eval + 20484);
        goto LABEL_121;
    case 14:
        Eval_PrepareBinaryOpIntegers((Eval *)eval);
        *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) ^= *(_DWORD *)(eval
            + 16
            * (*(_DWORD *)(eval + 20484) - 1)
            + 4104);
        --*(_DWORD *)(eval + 20484);
        goto LABEL_121;
    case 15:
        if (*(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4096))
        {
            if (*(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4096) != 1)
            {
                v38 = "cannot logical invert strings";
                _CxxThrowException(&v38, &PA.deinit);
            }
            *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4104) = *(_DWORD *)(eval
                + 16
                * (*(_DWORD *)(eval + 20484) - 1)
                + 4104) == 0;
        }
        else
        {
            //v39 = *(double *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4104) == 0.0;
            *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4104) = *(double *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4104) == 0.0;
        }
        *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4096) = 1;
        goto LABEL_121;
    case 16:
        Eval_PrepareBinaryOpBoolean((Eval *)eval);
        *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) &= *(_DWORD *)(eval
            + 16
            * (*(_DWORD *)(eval + 20484) - 1)
            + 4104);
        --*(_DWORD *)(eval + 20484);
        goto LABEL_121;
    case 17:
        Eval_PrepareBinaryOpBoolean((Eval *)eval);
        *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) |= *(_DWORD *)(eval
            + 16
            * (*(_DWORD *)(eval + 20484) - 1)
            + 4104);
        --*(_DWORD *)(eval + 20484);
        goto LABEL_121;
    case 18:
        if (*(int *)(eval + 20484) >= 2
            && *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4096) == 2
            && *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4096) == 2)
        {
            same = _stricmp(
                *(const char **)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104),
                *(const char **)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4104)) == 0;
            free(*(void **)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104));
            free(*(void **)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4104));
            *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4096) = 1;
            *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) = same;
        }
        else
        {
            Eval_PrepareBinaryOpSameTypes((Eval *)eval);
            if (*(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4096))
            {
                *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) = *(_DWORD *)(eval
                    + 16
                    * (*(_DWORD *)(eval + 20484) - 2)
                    + 4104) == *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4104);
            }
            else
            {
                //v20 = *(double *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) == *(double *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4104);
                *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) = *(double *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) == *(double *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4104);
                *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4096) = 1;
            }
        }
        -- * (_DWORD *)(eval + 20484);
        goto LABEL_121;
    case 19:
        if (*(int *)(eval + 20484) >= 2
            && *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4096) == 2
            && *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4096) == 2)
        {
            HIBYTE(v19) = _stricmp(
                *(const char **)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104),
                *(const char **)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4104)) == 0;
            free(*(void **)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104));
            free(*(void **)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4104));
            *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4096) = 1;
            *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) = HIBYTE(v19) == 0;
        }
        else
        {
            Eval_PrepareBinaryOpSameTypes((Eval *)eval);
            if (*(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4096))
            {
                *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) = *(_DWORD *)(eval
                    + 16
                    * (*(_DWORD *)(eval + 20484) - 2)
                    + 4104) != *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4104);
            }
            else
            {
                //v18 = *(double *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) != *(double *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4104);
                *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) = *(double *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) != *(double *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4104);
                *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4096) = 1;
            }
        }
        -- * (_DWORD *)(eval + 20484);
        goto LABEL_121;
    case 20:
        Eval_PrepareBinaryOpSameTypes((Eval *)eval);
        if (*(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4096))
        {
            *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) = *(_DWORD *)(eval
                + 16
                * (*(_DWORD *)(eval + 20484) - 2)
                + 4104) < *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4104);
        }
        else
        {
            //v17 = *(double *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4104) > *(double *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104);
            *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) = *(double *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4104) > *(double *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104);
            *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4096) = 1;
        }
        -- * (_DWORD *)(eval + 20484);
        goto LABEL_121;
    case 21:
        Eval_PrepareBinaryOpSameTypes((Eval *)eval);
        if (*(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4096))
        {
            *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) = *(_DWORD *)(eval
                + 16
                * (*(_DWORD *)(eval + 20484) - 2)
                + 4104) <= *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4104);
        }
        else
        {
            //v16 = *(double *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4104) >= *(double *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104);
            *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) = *(double *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4104) >= *(double *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104);
            *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4096) = 1;
        }
        -- * (_DWORD *)(eval + 20484);
        goto LABEL_121;
    case 22:
        Eval_PrepareBinaryOpSameTypes((Eval *)eval);
        if (*(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4096))
        {
            *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) = *(_DWORD *)(eval
                + 16
                * (*(_DWORD *)(eval + 20484) - 2)
                + 4104) > *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4104);
        }
        else
        {
            //v15 = *(double *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) > *(double *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4104);
            *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) = *(double *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) > *(double *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4104);
            *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4096) = 1;
        }
        -- * (_DWORD *)(eval + 20484);
        goto LABEL_121;
    case 23:
        Eval_PrepareBinaryOpSameTypes((Eval *)eval);
        if (*(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4096))
        {
            *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) = *(_DWORD *)(eval
                + 16
                * (*(_DWORD *)(eval + 20484) - 2)
                + 4104) >= *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4104);
        }
        else
        {
            //v14 = *(double *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) >= *(double *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4104);
            *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) = *(double *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4104) >= *(double *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 1) + 4104);
            *(_DWORD *)(eval + 16 * (*(_DWORD *)(eval + 20484) - 2) + 4096) = 1;
        }
        -- * (_DWORD *)(eval + 20484);
    LABEL_121:
        result = 1;
        break;
    default:
        if (!Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\eval.cpp", 488, 0, "unknown operator type"))
            __debugbreak();
        result = 0;
        break;
    }
    return result;
}

void __cdecl Eval_PrepareBinaryOpSameTypes(Eval *eval)
{
    int v1; // eax
    const char *v2; // [esp+4h] [ebp-8h] BYREF
    const char *pExceptionObject; // [esp+8h] [ebp-4h] BYREF

    if ( eval->valStackPos < 2 )
    {
        pExceptionObject = "missing operand (for example, 'a + ' or ' / b')";
        _CxxThrowException(&pExceptionObject, &PA.deinit);
    }
    if ( eval->opStack[4 * eval->valStackPos + 1016] == EVAL_OP_COLON
        || eval->opStack[4 * eval->valStackPos + 1020] == EVAL_OP_COLON )
    {
        v2 = "operation not valid on strings";
        _CxxThrowException(&v2, &PA.deinit);
    }
    if ( eval->opStack[4 * eval->valStackPos + 1016] != eval->opStack[4 * eval->valStackPos + 1020] )
    {
        if ( eval->opStack[4 * eval->valStackPos + 1016] == EVAL_OP_RPAREN )
        {
            *(double *)&eval->opStack[4 * eval->valStackPos + 1018] = (double)(int)eval->opStack[4 * eval->valStackPos + 1018];
            v1 = eval->valStackPos - 2;
        }
        else
        {
            *(double *)&eval->opStack[4 * eval->valStackPos + 1022] = (double)(int)eval->opStack[4 * eval->valStackPos + 1022];
            v1 = eval->valStackPos - 1;
        }
        eval->valStack[v1].type = EVAL_VALUE_DOUBLE;
    }
}

void __cdecl Eval_PrepareBinaryOpIntegers(Eval *eval)
{
    const char *v1; // [esp+0h] [ebp-8h] BYREF
    const char *pExceptionObject; // [esp+4h] [ebp-4h] BYREF

    if ( eval->valStackPos < 2 )
    {
        pExceptionObject = "missing operand (for example, 'a + ' or ' / b')";
        _CxxThrowException(&pExceptionObject, &PA.deinit);
    }
    if ( eval->opStack[4 * eval->valStackPos + 1016] == EVAL_OP_COLON
        || eval->opStack[4 * eval->valStackPos + 1020] == EVAL_OP_COLON )
    {
        v1 = "operation not valid on strings";
        _CxxThrowException(&v1, &PA.deinit);
    }
    if ( eval->opStack[4 * eval->valStackPos + 1016] == EVAL_OP_LPAREN )
    {
        eval->opStack[4 * eval->valStackPos + 1018] = (EvalOperatorType)(int)*(double *)&eval->opStack[4 * eval->valStackPos + 1018];
        eval->opStack[4 * eval->valStackPos + 1016] = EVAL_OP_RPAREN;
    }
    if ( eval->opStack[4 * eval->valStackPos + 1020] == EVAL_OP_LPAREN )
    {
        eval->opStack[4 * eval->valStackPos + 1022] = (EvalOperatorType)(int)*(double *)&eval->opStack[4 * eval->valStackPos + 1022];
        eval->opStack[4 * eval->valStackPos + 1020] = EVAL_OP_RPAREN;
    }
}

void __cdecl Eval_PrepareBinaryOpBoolean(Eval *eval)
{
    const char *v1; // [esp+8h] [ebp-8h] BYREF
    const char *pExceptionObject; // [esp+Ch] [ebp-4h] BYREF

    if ( eval->valStackPos < 2 )
    {
        pExceptionObject = "missing operand (for example, 'a + ' or ' / b')";
        _CxxThrowException(&pExceptionObject, &PA.deinit);
    }
    if ( eval->opStack[4 * eval->valStackPos + 1016] == EVAL_OP_COLON
        || eval->opStack[4 * eval->valStackPos + 1020] == EVAL_OP_COLON )
    {
        v1 = "operation not valid on strings";
        _CxxThrowException(&v1, &PA.deinit);
    }
    if ( eval->opStack[4 * eval->valStackPos + 1016] )
    {
        eval->opStack[4 * eval->valStackPos + 1018] = (EvalOperatorType)(eval->opStack[4 * eval->valStackPos + 1018] != EVAL_OP_LPAREN);
    }
    else
    {
        eval->opStack[4 * eval->valStackPos + 1018] = (EvalOperatorType)(*(double *)&eval->opStack[4 * eval->valStackPos + 1018] != 0.0);
        eval->opStack[4 * eval->valStackPos + 1016] = EVAL_OP_RPAREN;
    }
    if ( eval->opStack[4 * eval->valStackPos + 1020] )
    {
        eval->opStack[4 * eval->valStackPos + 1022] = (EvalOperatorType)(eval->opStack[4 * eval->valStackPos + 1022] != EVAL_OP_LPAREN);
    }
    else
    {
        eval->opStack[4 * eval->valStackPos + 1022] = (EvalOperatorType)(*(double *)&eval->opStack[4 * eval->valStackPos + 1022] != 0.0);
        eval->opStack[4 * eval->valStackPos + 1020] = EVAL_OP_RPAREN;
    }
}

bool __cdecl Eval_IsUnaryOp(const Eval *eval)
{
    return !eval->valStackPos || eval->pushedOp;
}

char __cdecl Eval_PushInteger(Eval *eval, int value)
{
    if ( !Eval_CanPushValue(eval) )
        return 0;
    eval->valStack[eval->valStackPos].type = EVAL_VALUE_INT;
    eval->valStack[eval->valStackPos++].u.i = value;
    eval->pushedOp = 0;
    return 1;
}

bool __cdecl Eval_CanPushValue(const Eval *eval)
{
    const char *pExceptionObject; // [esp+0h] [ebp-4h] BYREF

    if ( !eval->valStackPos )
        return 1;
    if ( eval->valStackPos == 1024 )
    {
        pExceptionObject = "evaluation stack overflow - expression is too complex";
        _CxxThrowException(&pExceptionObject, &PA.deinit);
    }
    return eval->pushedOp;
}

char __cdecl Eval_PushNumber(Eval *eval, long double value)
{
    if ( !Eval_CanPushValue(eval) )
        return 0;
    eval->valStack[eval->valStackPos].type = EVAL_VALUE_DOUBLE;
    eval->valStack[eval->valStackPos++].u.d = value;
    eval->pushedOp = 0;
    return 1;
}

EvalValue * Eval_Solve(EvalValue *result, Eval *eval)
{
    int v4; // [esp+0h] [ebp-2Ch] BYREF
    const char *v6; // [esp+14h] [ebp-18h] BYREF
    unsigned int pExceptionObject[5]; // [esp+18h] [ebp-14h] BYREF
    int savedregs; // [esp+2Ch] [ebp+0h] BYREF

    pExceptionObject[1] = (unsigned int)&v4;
    pExceptionObject[4] = 0;
    if ( eval->parenCount )
    {
        pExceptionObject[0] = (unsigned int)"missing ')'";
        _CxxThrowException(pExceptionObject, &PA.deinit);
    }
    while ( Eval_EvaluationStep(eval) )
        ;
    if ( eval->opStackPos
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\eval.cpp",
                    649,
                    0,
                    "%s",
                    "eval->opStackPos == 0") )
    {
        __debugbreak();
    }
    if ( eval->valStackPos > 1 )
    {
        v6 = "extra operand (for example, 'a b +')";
        _CxxThrowException(&v6, &PA.deinit);
    }
    *result = eval->valStack[0];
    return result;
}

bool __cdecl Eval_OperatorForToken(const char *text, EvalOperatorType *op)
{
    bool result; // al

    if ( !text && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\eval.cpp", 674, 0, "%s", "text") )
        __debugbreak();
    if ( !op && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\eval.cpp", 675, 0, "%s", "op") )
        __debugbreak();
    switch ( *text )
    {
        case '!':
            if ( text[1] == '=' )
                *op = EVAL_OP_NOT_EQUAL;
            else
                *op = EVAL_OP_LOGICAL_NOT;
            result = 1;
            break;
        case '%':
            *op = EVAL_OP_MODULUS;
            result = 1;
            break;
        case '&':
            if ( text[1] == '&' )
                *op = EVAL_OP_LOGICAL_AND;
            else
                *op = EVAL_OP_BITWISE_AND;
            result = 1;
            break;
        case '(':
            *op = EVAL_OP_LPAREN;
            result = 1;
            break;
        case ')':
            *op = EVAL_OP_RPAREN;
            result = 1;
            break;
        case '*':
            *op = EVAL_OP_MULTIPLY;
            result = 1;
            break;
        case '+':
            *op = EVAL_OP_PLUS;
            result = 1;
            break;
        case '-':
            *op = EVAL_OP_MINUS;
            result = 1;
            break;
        case '/':
            *op = EVAL_OP_DIVIDE;
            result = 1;
            break;
        case ':':
            *op = EVAL_OP_COLON;
            result = 1;
            break;
        case '<':
            if ( text[1] == '<' )
            {
                *op = EVAL_OP_LSHIFT;
                result = 1;
            }
            else
            {
                if ( text[1] == '=' )
                    *op = EVAL_OP_LESS_EQUAL;
                else
                    *op = EVAL_OP_LESS;
                result = 1;
            }
            break;
        case '=':
            if ( text[1] != '=' )
                goto LABEL_46;
            *op = EVAL_OP_EQUALS;
            result = 1;
            break;
        case '>':
            if ( text[1] == '>' )
            {
                *op = EVAL_OP_RSHIFT;
                result = 1;
            }
            else
            {
                if ( text[1] == 61 )
                    *op = EVAL_OP_GREATER_EQUAL;
                else
                    *op = EVAL_OP_GREATER;
                result = 1;
            }
            break;
        case '?':
            *op = EVAL_OP_QUESTION;
            result = 1;
            break;
        case '^':
            *op = EVAL_OP_BITWISE_XOR;
            result = 1;
            break;
        case '|':
            if ( text[1] == '|' )
                *op = EVAL_OP_LOGICAL_OR;
            else
                *op = EVAL_OP_BITWISE_OR;
            result = 1;
            break;
        case '~':
            *op = EVAL_OP_BITWISE_NOT;
            result = 1;
            break;
        default:
LABEL_46:
            result = 0;
            break;
    }
    return result;
}

