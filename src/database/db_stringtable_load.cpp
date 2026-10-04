#include "db_stringtable_load.h"
#include <clientscript/cscr_stringlist.h>
#include "db_file_load.h"

void __cdecl Load_ScriptStringCustom(unsigned __int16 *var)
{
    *var = (unsigned __int16)varXAssetList->stringList.strings[*var];
}

void __cdecl Mark_ScriptStringCustom(unsigned __int16 *var)
{
    if ( *var )
        SL_AddUser(*var, 4u, SCRIPTINSTANCE_SERVER);
}

