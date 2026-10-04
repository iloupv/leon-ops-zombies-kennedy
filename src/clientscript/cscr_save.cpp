#include "cscr_save.h"
#include "cscr_animtree.h"
#include "cscr_readwrite.h"
#include "cscr_stringlist.h"
#include "cscr_memorytree.h"
#include "cscr_debugger.h"
#include <universal/memfile.h>
#include <qcommon/common.h>
#include <cstddef>
#include <cstring>

// DERIVED from KB Scr_InitVariables / GetVariableValueAddress: both instances have 0x7FFE parent
// slots and 0xFFFC-byte save maps, unlike SP's 0x5FFE server / 0x4000 client slots. Access the KB
// fields by name; SP VariableValueInternal is 0x10 bytes and KB's is 0x1C.
// mod: follows bo1_mod_scriptvars (server instance; saves are server-script only)
#define SAVE_OBJECT_LIMIT VARIABLELIST_PARENT_SIZE(SCRIPTINSTANCE_SERVER)
#define SAVE_CHILD_BEGIN VARIABLELIST_CHILD_BEGIN(SCRIPTINSTANCE_SERVER)
static_assert(sizeof(VariableValueInternal) == 0x1C, "review save traversal for changed VM layout");
static_assert(sizeof(VariableUnion) == 4, "save stacks contain four-byte values");
static_assert(offsetof(VariableStackBuffer, buf) == 13, "KB save stack payload offset (SP 11)");

static void Scr_SaveId(scriptInstance_t inst, unsigned int id)
{
    // zombies: SP 0x00410620 (also inlined in 0x005FA390 / 0x0058D770).
    if (!id)
        return;
    scrVarPub_t *pub = &gScrVarPub[inst];
    iassert(id < SAVE_OBJECT_LIMIT);
    if (pub->saveIdMap[id])
        return;
    iassert(pub->savecount + 1 < SAVE_OBJECT_LIMIT);
    pub->saveIdMap[id] = pub->savecount + 1;
    pub->saveIdMapRev[pub->savecount + 1] = (unsigned short)id;
    ++pub->savecount;
}

static void Scr_AddSaveStack(scriptInstance_t inst, const VariableStackBuffer *stack);

static void Scr_AddSaveValue(scriptInstance_t inst, int type, VariableUnion value)
{
    // zombies: SP 0x00491900. Queue references; Scr_AddSaveObject drains the queue.
    if (type == VAR_POINTER)
        Scr_SaveId(inst, value.pointerValue);
    else if (type == VAR_STACK)
        Scr_AddSaveStack(inst, value.stackValue);
}

static void Scr_AddSaveStack(scriptInstance_t inst, const VariableStackBuffer *stack)
{
    // zombies: SP 0x0058D770. localId is a ushort in SP, uint in KB; payload entries are five bytes.
    Scr_SaveId(inst, stack->localId);
    const char *buf = stack->buf;
    for (unsigned int size = stack->size; size; --size)
    {
        const unsigned char type = (unsigned char)*buf++;
        VariableUnion value;
        memcpy(&value, buf, sizeof(value));
        buf += sizeof(value);
        Scr_AddSaveValue(inst, type, value);
    }
}

static void Scr_AddSaveObjectChildren(scriptInstance_t inst, unsigned int id)
{
    // zombies: SP 0x005FA390. Visit array keys before values; then visit the thread parent/self.
    VariableValueInternal *list = gScrVarGlob[inst].variableList;
    const VariableValueInternal *parent = &list[id + VARIABLELIST_PARENT_BEGIN];
    const unsigned int type = parent->w.type & VAR_MASK;
    for (unsigned int child = FindFirstSibling(inst, id); child; child = FindNextSibling(inst, child))
    {
        const VariableValueInternal *value = &list[SAVE_CHILD_BEGIN + child];
        if (type == VAR_ARRAY)
        {
            const unsigned int name = value->w.name >> VAR_NAME_BITS;
            if (name >= 0x10000 && name < 0x10000 + SAVE_OBJECT_LIMIT)
                Scr_SaveId(inst, name - 0x10000);
        }
        Scr_AddSaveValue(inst, value->w.type & VAR_MASK, value->u.u);
    }
    // SP jump table 0x005FA63C: D/E/F/12 -> self; 10 -> parent then self; 11 and others -> none.
    switch (type)
    {
    case VAR_CHILD_THREAD:
        Scr_SaveId(inst, parent->w.parentLocalId >> VAR_NAME_BITS);
        // fall through
    case VAR_THREAD:
    case VAR_NOTIFY_THREAD:
    case VAR_TIME_THREAD:
    case VAR_DEAD_ENTITY:
        Scr_SaveId(inst, parent->u.o.u.self);
        break;
    default:
        break;
    }
}

void Scr_AddSaveObject(scriptInstance_t inst, unsigned int id)
{
    // zombies: SP 0x005F5700. Begin at the previous count, so previously visited objects are skipped.
    scrVarPub_t *pub = &gScrVarPub[inst];
    unsigned int count = pub->savecount;
    Scr_SaveId(inst, id);
    while (count < pub->savecount)
    {
        Scr_AddSaveObjectChildren(inst, pub->saveIdMapRev[count + 1]);
        ++count;
    }
}

static void Scr_SaveRootStack(scriptInstance_t inst, const VariableStackBuffer *stack)
{
    // zombies: SP 0x0045B180. Unlike 0x0058D770, expand each reference immediately (game variable).
    Scr_AddSaveObject(inst, stack->localId);
    const char *buf = stack->buf;
    for (unsigned int size = stack->size; size; --size)
    {
        const unsigned char type = (unsigned char)*buf++;
        VariableUnion value;
        memcpy(&value, buf, sizeof(value));
        buf += sizeof(value);
        if (type == VAR_POINTER)
            Scr_AddSaveObject(inst, value.pointerValue);
        else if (type == VAR_STACK)
            Scr_SaveRootStack(inst, value.stackValue);
    }
}

void Scr_SavePre(scriptInstance_t inst)
{
    // zombies: SP 0x00642F70. Its second argument (1 at G_SaveState) is not read by the exe.
    scrVarPub_t *pub = &gScrVarPub[inst];
    memset(pub->saveIdMap, 0, SAVE_OBJECT_LIMIT * sizeof(*pub->saveIdMap));
    memset(pub->saveIdMapRev, 0, SAVE_OBJECT_LIMIT * sizeof(*pub->saveIdMapRev));
    pub->savecount = 0;
    Scr_AddSaveObject(inst, pub->levelId);
    Scr_AddSaveObject(inst, pub->animId);
    Scr_AddSaveObject(inst, pub->timeArrayId);
    Scr_AddSaveObject(inst, pub->pauseArrayId);
    Scr_AddSaveObject(inst, pub->freeEntList);
    for (unsigned int i = 0; i < CLASS_NUM_COUNT; ++i)
    {
        Scr_AddSaveObject(inst, gScrClassMap[inst][i].id);
        Scr_AddSaveObject(inst, gScrClassMap[inst][i].entArrayId);
    }
    const VariableValueInternal *game = &gScrVarGlob[inst].variableList[SAVE_CHILD_BEGIN + pub->gameId];
    if ((game->w.type & VAR_MASK) == VAR_POINTER)
        Scr_AddSaveObject(inst, game->u.u.pointerValue);
    else if ((game->w.type & VAR_MASK) == VAR_STACK)
        Scr_SaveRootStack(inst, game->u.u.stackValue);
}

void Scr_CheckSaveObjects(scriptInstance_t inst)
{
    // KB diagnostic, not SP: map bijection, live objects, and the principal script roots.
    const scrVarPub_t *pub = &gScrVarPub[inst];
    bool valid = !pub->saveIdMap[0] && !pub->saveIdMapRev[0];
    unsigned int objects = 0;
    unsigned int threads = 0;
    for (unsigned int id = 1; id < SAVE_OBJECT_LIMIT; ++id)
    {
        const unsigned int savedId = pub->saveIdMap[id];
        if (!savedId)
            continue;
        ++objects;
        valid = valid && savedId <= pub->savecount && pub->saveIdMapRev[savedId] == id && !IsObjectFree(inst, id);
        const unsigned int type = gScrVarGlob[inst].variableList[id + VARIABLELIST_PARENT_BEGIN].w.type & VAR_MASK;
        threads += type >= VAR_THREAD && type <= VAR_CHILD_THREAD;
    }
    const unsigned int roots[] = { pub->levelId, pub->animId, pub->timeArrayId, pub->pauseArrayId, pub->freeEntList };
    for (unsigned int id : roots)
        valid = valid && (!id || (id < SAVE_OBJECT_LIMIT && pub->saveIdMap[id]));
    valid = valid && objects == pub->savecount;
    Com_Printf(15, "SaveGame check: script save IDs %s (%u objects, %u threads)\n",
        valid ? "valid" : "DIFFERENT", objects, threads);
    iassert(valid);
}

// The SP writer keeps this 16-entry history in TLS (+0x38/+0x3C). A context makes that state
// explicit, preserving one history across all objects, stack values, array keys and root IDs.
struct ScrSaveContext
{
    scriptInstance_t inst;
    MemoryFile *memFile;
    unsigned short history[16];
    unsigned int historyIndex;
};

static void Scr_SaveByte(ScrSaveContext *ctx, unsigned char value)
{
    MemFile_WriteData(ctx->memFile, 1, &value);
}

static void Scr_SaveShort(ScrSaveContext *ctx, unsigned short value)
{
    MemFile_WriteData(ctx->memFile, 2, (unsigned __int8 *)&value);
}

static void Scr_SaveObjectReference(ScrSaveContext *ctx, unsigned int id, unsigned char tag)
{
    // zombies: SP 0x00610BC0, history search 0x006829E0. Odd tokens mean previous ID + 1;
    // even tokens mean previous ID. Token 32 cannot fit in five bits and uses the literal form.
    iassert(id < SAVE_OBJECT_LIMIT);
    const unsigned short savedId = gScrVarPub[ctx->inst].saveIdMap[id];
    iassert(!id || savedId);
    unsigned int token = 0;
    for (unsigned int distance = 1; distance <= 16; ++distance)
    {
        const unsigned short previous = ctx->history[(ctx->historyIndex + distance) & 15];
        if (savedId == previous + 1)
        {
            token = 2 * distance - 1;
            break;
        }
        if (savedId == previous)
        {
            token = 2 * distance;
            break;
        }
    }
    if (token >= 32)
        token = 0;
    Scr_SaveByte(ctx, (unsigned char)((token << 3) + tag));
    if (!token)
        Scr_SaveShort(ctx, savedId);
    ctx->history[ctx->historyIndex] = savedId;
    ctx->historyIndex = (ctx->historyIndex - 1) & 15;
}

static void Scr_SaveString(ScrSaveContext *ctx, unsigned int stringValue)
{
    // zombies: SP 0x0048F7C0 -> SL_ConvertToString 0x00687530 -> CString 0x0066FF20.
    MemFile_WriteCString(ctx->memFile, SL_ConvertToString(stringValue, ctx->inst));
}

static void Scr_SaveCodepos(ScrSaveContext *ctx, const char *pos)
{
    // zombies: SP 0x005019C0; null code positions are -1, all others are program-buffer offsets.
    const int offset = pos ? (int)(pos - gScrVarPub[ctx->inst].programBuffer) : -1;
    MemFile_WriteInt(ctx->memFile, offset);
}

static void Scr_SaveValue(ScrSaveContext *ctx, unsigned int type, VariableUnion value);

static void Scr_SaveStack(ScrSaveContext *ctx, const VariableStackBuffer *stack)
{
    // zombies: SP 0x0045C1B0. DERIVED: KB stack localId is uint and payload starts at 13, not 11.
    Scr_SaveShort(ctx, stack->size);
    Scr_SaveCodepos(ctx, stack->pos);
    Scr_SaveObjectReference(ctx, stack->localId, 0);
    Scr_SaveByte(ctx, stack->time);
    const char *buf = stack->buf;
    for (unsigned int size = stack->size; size; --size)
    {
        const unsigned char type = (unsigned char)*buf++;
        VariableUnion value;
        memcpy(&value, buf, sizeof(value));
        buf += sizeof(value);
        Scr_SaveValue(ctx, type, value);
    }
}

static void Scr_SaveValue(ScrSaveContext *ctx, unsigned int type, VariableUnion value)
{
    // zombies: SP 0x00405000. The jump table at 0x004050EC writes ANIMATION as a raw int;
    // CODEPOS/FUNCTION use offsets, PRECODEPOS/DEVELOPER_CODEPOS have no payload.
    if (type == VAR_POINTER)
    {
        Scr_SaveObjectReference(ctx, value.pointerValue, 1);
        return;
    }
    Scr_SaveByte(ctx, (unsigned char)(type << 3));
    switch (type)
    {
    case VAR_STRING:
    case VAR_ISTRING:
        Scr_SaveString(ctx, value.stringValue);
        break;
    case VAR_VECTOR:
        MemFile_WriteData(ctx->memFile, 12, (unsigned __int8 *)value.vectorValue);
        break;
    case VAR_FLOAT:
        MemFile_WriteFloat(ctx->memFile, value.floatValue);
        break;
    case VAR_INTEGER:
    case VAR_ANIMATION:
        MemFile_WriteInt(ctx->memFile, value.intValue);
        break;
    case VAR_CODEPOS:
    case VAR_FUNCTION:
        Scr_SaveCodepos(ctx, value.codePosValue);
        break;
    case VAR_STACK:
        Scr_SaveStack(ctx, value.stackValue);
        break;
    default:
        break;
    }
}

static void Scr_SaveEntry(ScrSaveContext *ctx, const VariableValueInternal *value, bool isArray)
{
    // zombies: SP 0x008A5CD0, value first, name second. Object field names are canonical IDs;
    // array names are strings, object references, or signed integer indices (bias 0x800000).
    const unsigned int name = value->w.name >> VAR_NAME_BITS;
    Scr_SaveValue(ctx, value->w.type & VAR_MASK, value->u.u);
    if (!isArray)
    {
        const unsigned int extra = name < 0x40 ? 0 : name < 0x4000 ? 1 : name < 0x400000 ? 2 : 3;
        Scr_SaveByte(ctx, (unsigned char)((name << 2) | extra));
        for (unsigned int i = 0; i < extra; ++i)
            Scr_SaveByte(ctx, (unsigned char)(name >> (6 + 8 * i)));
        return;
    }
    if (name < 0x10000)
    {
        Scr_SaveByte(ctx, 4);
        Scr_SaveString(ctx, name);
        return;
    }
    if (name < 0x10000 + SAVE_OBJECT_LIMIT)
    {
        Scr_SaveObjectReference(ctx, name - 0x10000, 5);
        return;
    }
    const int index = (int)name - 0x800000;
    if (index >= -0x20 && index < 0x20)
        Scr_SaveByte(ctx, (unsigned char)(index * 8));
    else if (index >= -0x2000 && index < 0x2000)
    {
        Scr_SaveByte(ctx, (unsigned char)(((index >> 5) & 0xF9) | 1));
        Scr_SaveByte(ctx, (unsigned char)index);
    }
    else if (index >= -0x200000 && index < 0x200000)
    {
        Scr_SaveByte(ctx, (unsigned char)(((index >> 13) & 0xFA) | 2));
        Scr_SaveShort(ctx, (unsigned short)index);
    }
    else
    {
        Scr_SaveByte(ctx, 3);
        MemFile_WriteInt(ctx->memFile, index);
    }
}

static void Scr_SaveObject(ScrSaveContext *ctx, unsigned int id)
{
    // zombies: SP 0x0068A930. Write children from the last hash sibling to the first, unlike
    // the forward traversal in Scr_SavePre. The reader inserts each child at the front.
    VariableValueInternal *list = gScrVarGlob[ctx->inst].variableList;
    const VariableValueInternal *parent = &list[id + VARIABLELIST_PARENT_BEGIN];
    const unsigned int type = parent->w.type & VAR_MASK;
    switch (type)
    {
    case VAR_THREAD:
        Scr_SaveObjectReference(ctx, parent->u.o.u.self, 1);
        break;
    case VAR_NOTIFY_THREAD:
        Scr_SaveObjectReference(ctx, parent->u.o.u.self, 2);
        // SP 0x004CE430 distinguishes no notify name from an empty string.
        Scr_SaveByte(ctx, (parent->w.notifyName >> VAR_NAME_BITS) != 0);
        if (parent->w.notifyName >> VAR_NAME_BITS)
            Scr_SaveString(ctx, parent->w.notifyName >> VAR_NAME_BITS);
        break;
    case VAR_TIME_THREAD:
        Scr_SaveObjectReference(ctx, parent->u.o.u.self, 3);
        MemFile_WriteInt(ctx->memFile, parent->w.waitTime >> VAR_NAME_BITS);
        break;
    case VAR_CHILD_THREAD:
        Scr_SaveObjectReference(ctx, parent->u.o.u.self, 4);
        Scr_SaveObjectReference(ctx, parent->w.parentLocalId >> VAR_NAME_BITS, 0);
        break;
    case VAR_DEAD_ENTITY:
        Scr_SaveObjectReference(ctx, parent->u.o.u.self, 5);
        break;
    case VAR_ENTITY:
        Scr_SaveByte(ctx, (unsigned char)(VAR_ENTITY << 3));
        Scr_SaveShort(ctx, parent->u.o.u.entnum);
        Scr_SaveShort(ctx, (unsigned short)(parent->w.classnum >> VAR_NAME_BITS));
        break;
    default:
        Scr_SaveByte(ctx, (unsigned char)(type << 3));
        break;
    }
    unsigned int count = 0;
    for (unsigned int child = FindFirstSibling(ctx->inst, id); child; child = FindNextSibling(ctx->inst, child))
        ++count;
    iassert(count <= 0xFFFF);
    Scr_SaveShort(ctx, (unsigned short)count);
    unsigned int written = 0;
    for (unsigned int index = FindLastSibling(ctx->inst, id); index;
        index = list[SAVE_CHILD_BEGIN + index].hash.u.prevSibling)
    {
        const unsigned int child = list[SAVE_CHILD_BEGIN + index].hash.id;
        Scr_SaveEntry(ctx, &list[SAVE_CHILD_BEGIN + child], type == VAR_ARRAY);
        ++written;
    }
    iassert(written == count);
}

void Scr_Save(scriptInstance_t inst, MemoryFile *memFile)
{
    // zombies: SP 0x0050DF00. Scr_SavePre has already assigned the object graph's save IDs.
    ScrSaveContext ctx = {};
    ctx.inst = inst;
    ctx.memFile = memFile;
    const scrVarPub_t *pub = &gScrVarPub[inst];
    MemFile_WriteInt(memFile, pub->time);
    Scr_SaveShort(&ctx, pub->savecount);
    for (unsigned int i = 1; i <= pub->savecount; ++i)
        Scr_SaveObject(&ctx, pub->saveIdMapRev[i]);
    const VariableValueInternal *game = &gScrVarGlob[inst].variableList[SAVE_CHILD_BEGIN + pub->gameId];
    Scr_SaveValue(&ctx, game->w.type & VAR_MASK, game->u.u);
    Scr_SaveObjectReference(&ctx, pub->levelId, 0);
    Scr_SaveObjectReference(&ctx, pub->animId, 0);
    Scr_SaveObjectReference(&ctx, pub->timeArrayId, 0);
    Scr_SaveObjectReference(&ctx, pub->pauseArrayId, 0);
    Scr_SaveObjectReference(&ctx, pub->freeEntList, 0);
    for (unsigned int i = 0; i < CLASS_NUM_COUNT; ++i)
    {
        Scr_SaveObjectReference(&ctx, gScrClassMap[inst][i].id, 0);
        Scr_SaveObjectReference(&ctx, gScrClassMap[inst][i].entArrayId, 0);
    }
}

static unsigned char Scr_LoadByte(ScrSaveContext *ctx)
{
    unsigned char value;
    MemFile_ReadData(ctx->memFile, 1, &value);
    return value;
}

static unsigned short Scr_LoadShort(ScrSaveContext *ctx)
{
    unsigned short value;
    MemFile_ReadData(ctx->memFile, 2, (unsigned char *)&value);
    return value;
}

static int Scr_LoadInt(ScrSaveContext *ctx)
{
    int value;
    MemFile_ReadData(ctx->memFile, 4, (unsigned char *)&value);
    return value;
}

static unsigned int Scr_LoadObjectReference(ScrSaveContext *ctx, unsigned char tag)
{
    // zombies: SP 0x004925C0. The allocation pass makes forward references valid too.
    const unsigned int token = tag >> 3;
    const unsigned int savedId = token
        ? ctx->history[(ctx->historyIndex + ((token + 1) >> 1)) & 15] + (token & 1)
        : Scr_LoadShort(ctx);
    iassert(savedId <= gScrVarPub[ctx->inst].savecount);
    const unsigned int id = gScrVarPub[ctx->inst].saveIdMapRev[savedId];
    if (id)
        AddRefToObject(ctx->inst, id);
    ctx->history[ctx->historyIndex] = (unsigned short)savedId;
    ctx->historyIndex = (ctx->historyIndex - 1) & 15;
    return id;
}

static unsigned int Scr_LoadString(ScrSaveContext *ctx)
{
    // zombies: SP 0x00505F00 (0x0062BB00 adds a presence byte for notify names).
    return SL_GetString_(ctx->inst, MemFile_ReadCString(ctx->memFile), 0, 15);
}

static const char *Scr_LoadCodepos(ScrSaveContext *ctx)
{
    // zombies: SP 0x0043BB20.
    const int offset = Scr_LoadInt(ctx);
    return offset < 0 ? nullptr : gScrVarPub[ctx->inst].programBuffer + offset;
}

static VariableValue Scr_LoadValue(ScrSaveContext *ctx);

static VariableStackBuffer *Scr_LoadStack(ScrSaveContext *ctx)
{
    // zombies: SP 0x005FD3F0. DERIVED: KB uses a uint localId and a 13-byte stack header.
    const unsigned short size = Scr_LoadShort(ctx);
    const unsigned int length = offsetof(VariableStackBuffer, buf) + 5 * size;
    iassert(length <= 0xFFFF);
    VariableStackBuffer *stack = (VariableStackBuffer *)MT_Alloc(length, 1, ctx->inst);
    stack->size = size;
    stack->bufLen = (unsigned short)length;
    stack->pos = Scr_LoadCodepos(ctx);
    stack->localId = Scr_LoadObjectReference(ctx, Scr_LoadByte(ctx));
    stack->time = Scr_LoadByte(ctx);
    char *buf = stack->buf;
    for (unsigned int i = 0; i < size; ++i)
    {
        const VariableValue value = Scr_LoadValue(ctx);
        *buf++ = (char)value.type;
        memcpy(buf, &value.u, sizeof(value.u));
        buf += sizeof(value.u);
    }
    // KB counts archived VM stacks for its diagnostics; SP has no numScriptThreads field.
    ++gScrVarPub[ctx->inst].numScriptThreads;
    return stack;
}

static VariableValue Scr_LoadValue(ScrSaveContext *ctx)
{
    // zombies: SP 0x005491B0, dispatch table 0x00549278.
    VariableValue value = {};
    const unsigned char tag = Scr_LoadByte(ctx);
    if (tag & 7)
    {
        value.type = VAR_POINTER;
        value.u.pointerValue = Scr_LoadObjectReference(ctx, tag);
        return value;
    }
    value.type = tag >> 3;
    switch (value.type)
    {
    case VAR_STRING:
    case VAR_ISTRING:
        value.u.stringValue = Scr_LoadString(ctx);
        break;
    case VAR_VECTOR:
    {
        // SP 0x0060D710.
        float vector[3];
        MemFile_ReadData(ctx->memFile, sizeof(vector), (unsigned char *)vector);
        value.u.vectorValue = Scr_AllocVector(ctx->inst, vector);
        break;
    }
    case VAR_FLOAT:
        MemFile_ReadData(ctx->memFile, 4, (unsigned char *)&value.u.floatValue);
        break;
    case VAR_INTEGER:
    case VAR_ANIMATION:
        value.u.intValue = Scr_LoadInt(ctx);
        break;
    case VAR_CODEPOS:
    case VAR_FUNCTION:
        value.u.codePosValue = Scr_LoadCodepos(ctx);
        break;
    case VAR_STACK:
        value.u.stackValue = Scr_LoadStack(ctx);
        break;
    default:
        break;
    }
    return value;
}

static unsigned int Scr_LoadEntry(ScrSaveContext *ctx, VariableValue *value, bool isArray)
{
    // zombies: SP 0x006631D0. Value first, then the canonical field ID or typed array key.
    *value = Scr_LoadValue(ctx);
    const unsigned char tag = Scr_LoadByte(ctx);
    if (!isArray)
    {
        unsigned int name = tag >> 2;
        for (unsigned int i = 0; i < (tag & 3); ++i)
            name |= (unsigned int)Scr_LoadByte(ctx) << (6 + 8 * i);
        return name;
    }
    // The writer accepts signed indices, but the exe reader uses logical SHR on the tag
    // (0x006632CD/DB/F8), and sign-extends only the short payload. Preserve those operations.
    switch (tag & 7)
    {
    case 0:
        return (tag >> 3) + 0x800000;
    case 1:
        return (((tag >> 3) << 8) | Scr_LoadByte(ctx)) + 0x800000;
    case 2:
        return (((tag >> 3) << 16) | (int)(short)Scr_LoadShort(ctx)) + 0x800000;
    case 3:
        return Scr_LoadInt(ctx) + 0x800000;
    case 4:
        return Scr_LoadString(ctx);
    case 5:
        return Scr_LoadObjectReference(ctx, tag) + 0x10000;
    default:
        return 0;
    }
}

static void Scr_LoadObject(ScrSaveContext *ctx, unsigned int id)
{
    // zombies: SP 0x006168C0. All records start as AllocObject, including threads/entities.
    VariableValueInternal *list = gScrVarGlob[ctx->inst].variableList;
    VariableValueInternal *parent = &list[id + VARIABLELIST_PARENT_BEGIN];
    const unsigned char tag = Scr_LoadByte(ctx);
    unsigned int type;
    switch (tag & 7)
    {
    case 1:
        type = VAR_THREAD;
        parent->u.o.u.self = Scr_LoadObjectReference(ctx, tag);
        break;
    case 2:
        type = VAR_NOTIFY_THREAD;
        parent->u.o.u.self = Scr_LoadObjectReference(ctx, tag);
        if (Scr_LoadByte(ctx))
            parent->w.notifyName |= Scr_LoadString(ctx) << VAR_NAME_BITS;
        break;
    case 3:
        type = VAR_TIME_THREAD;
        parent->u.o.u.self = Scr_LoadObjectReference(ctx, tag);
        parent->w.waitTime |= (unsigned int)Scr_LoadInt(ctx) << VAR_NAME_BITS;
        break;
    case 4:
        type = VAR_CHILD_THREAD;
        parent->u.o.u.self = Scr_LoadObjectReference(ctx, tag);
        parent->w.parentLocalId |= Scr_LoadObjectReference(ctx, Scr_LoadByte(ctx)) << VAR_NAME_BITS;
        break;
    case 5:
        type = VAR_DEAD_ENTITY;
        parent->u.o.u.self = Scr_LoadObjectReference(ctx, tag);
        break;
    default:
        type = tag >> 3;
        if (type == VAR_ENTITY)
        {
            parent->u.o.u.entnum = Scr_LoadShort(ctx);
            parent->w.classnum |= (unsigned int)(short)Scr_LoadShort(ctx) << VAR_NAME_BITS;
        }
        else if (type == VAR_ARRAY)
            parent->u.o.u.size = 0;
        break;
    }
    parent->w.type = (parent->w.type & ~VAR_MASK) | type;
    const unsigned int count = Scr_LoadShort(ctx);
    for (unsigned int i = 0; i < count; ++i)
    {
        VariableValue value;
        const unsigned int name = Scr_LoadEntry(ctx, &value, type == VAR_ARRAY);
        const unsigned int child = GetNewVariable(ctx->inst, id, name);
        // GetNewVariable takes its own array-key reference; release the reader's reference.
        if (type == VAR_ARRAY)
        {
            const VariableValue key = Scr_GetArrayIndexValue(ctx->inst, name);
            RemoveRefToValue(ctx->inst, key.type, key.u);
        }
        list[SAVE_CHILD_BEGIN + child].w.type |= value.type;
        list[SAVE_CHILD_BEGIN + child].u.u = value.u;
    }
}

void Scr_Load(scriptInstance_t inst, MemoryFile *memFile)
{
    // zombies: SP 0x0042AEB0. Retained bytecode; allocate graph, decode it, then install roots.
    ScrSaveContext ctx = {};
    ctx.inst = inst;
    ctx.memFile = memFile;
    scrVarPub_t *pub = &gScrVarPub[inst];
    iassert(!pub->levelId && !pub->animId && !pub->timeArrayId && !pub->pauseArrayId && !pub->gameId);
    pub->varUsagePos = "<script load variable>"; // KB allocation/debug bookkeeping.
    pub->time = Scr_LoadInt(&ctx);
    pub->savecount = Scr_LoadShort(&ctx);
    iassert(pub->savecount < SAVE_OBJECT_LIMIT);
    memset(pub->saveIdMap, 0, SAVE_OBJECT_LIMIT * sizeof(unsigned short));
    memset(pub->saveIdMapRev, 0, SAVE_OBJECT_LIMIT * sizeof(unsigned short));
    for (unsigned int i = 1; i <= pub->savecount; ++i)
    {
        const unsigned int id = AllocObject(inst);
        pub->saveIdMapRev[i] = (unsigned short)id;
        pub->saveIdMap[id] = (unsigned short)i;
    }
    for (unsigned int i = 1; i <= pub->savecount; ++i)
        Scr_LoadObject(&ctx, pub->saveIdMapRev[i]);
    // SP 0x00533830 uses AllocValue, not Scr_AllocGameVariable's new empty array.
    pub->gameId = AllocValue(inst);
    const VariableValue game = Scr_LoadValue(&ctx);
    VariableValueInternal *entry = &gScrVarGlob[inst].variableList[SAVE_CHILD_BEGIN + pub->gameId];
    entry->w.type |= game.type;
    entry->u.u = game.u;
    pub->levelId = Scr_LoadObjectReference(&ctx, Scr_LoadByte(&ctx));
    pub->animId = Scr_LoadObjectReference(&ctx, Scr_LoadByte(&ctx));
    pub->timeArrayId = Scr_LoadObjectReference(&ctx, Scr_LoadByte(&ctx));
    pub->pauseArrayId = Scr_LoadObjectReference(&ctx, Scr_LoadByte(&ctx));
    pub->freeEntList = Scr_LoadObjectReference(&ctx, Scr_LoadByte(&ctx));
    for (unsigned int i = 0; i < CLASS_NUM_COUNT; ++i)
    {
        iassert(!gScrClassMap[inst][i].id && !gScrClassMap[inst][i].entArrayId);
        gScrClassMap[inst][i].id = Scr_LoadObjectReference(&ctx, Scr_LoadByte(&ctx));
        gScrClassMap[inst][i].entArrayId = Scr_LoadObjectReference(&ctx, Scr_LoadByte(&ctx));
    }
    if (gScrVarDebugPub[inst])
    {
        const unsigned int roots[] = { pub->levelId, pub->animId, pub->timeArrayId,
            pub->pauseArrayId, pub->freeEntList };
        for (unsigned int id : roots)
            ++gScrVarDebugPub[inst]->extRefCount[id];
        for (unsigned int i = 0; i < CLASS_NUM_COUNT; ++i)
        {
            ++gScrVarDebugPub[inst]->extRefCount[gScrClassMap[inst][i].id];
            ++gScrVarDebugPub[inst]->extRefCount[gScrClassMap[inst][i].entArrayId];
        }
    }
    pub->varUsagePos = nullptr;
}

void Scr_LoadPost(scriptInstance_t inst)
{
    // zombies: SP 0x006837B0, after game-side fields have acquired their script references.
    for (unsigned int i = 1; i <= gScrVarPub[inst].savecount; ++i)
        RemoveRefToObject(inst, gScrVarPub[inst].saveIdMapRev[i]);
}
