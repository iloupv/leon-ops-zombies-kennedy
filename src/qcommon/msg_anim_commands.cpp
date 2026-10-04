#include "msg_anim_commands.h"
#include "msg.h"
#include <cmath>

// zombies: weight codec (SP 0x004DAD20).
static void MSG_WriteAnimWeight(msg_t *msg, float value)
{
    if (fabsf(value - 1.0f) < 0.0079f)
    {
        MSG_WriteBit0(msg);
        MSG_WriteBit1(msg);
        return;
    }
    if (fabsf(value) < 0.0079f)
    {
        MSG_WriteBit0(msg);
        MSG_WriteBit0(msg);
        return;
    }
    MSG_WriteBit1(msg);
    MSG_WriteBits(msg, (int)(value * 127.0f), 7);
}

// zombies: blend time codec (SP 0x0061B1D0).
static void MSG_WriteAnimBlendTime(msg_t *msg, float value)
{
    if (fabsf(value - 0.2f) < 0.005f)
    {
        MSG_WriteBit0(msg);
        return;
    }
    MSG_WriteBit1(msg);
    MSG_WriteBits(msg, (int)(value * 1000.0f + 0.5f) / 50, 5);
}

// zombies: rate codec (SP 0x00465D80).
static void MSG_WriteAnimRate(msg_t *msg, float value)
{
    if (fabsf(value - 1.0f) < 0.01f)
    {
        MSG_WriteBit0(msg);
        return;
    }
    MSG_WriteBit1(msg);
    MSG_WriteBits(msg, (int)(value * 42.333332f), 7);
}

// zombies: root codec (SP 0x004CE600).
static void MSG_WriteAnimRoot(msg_t *msg, unsigned int root)
{
    if (root == 0 || root == 3)
    {
        MSG_WriteBit0(msg);
        MSG_WriteBits(msg, root == 3, 1);
        return;
    }
    MSG_WriteBit1(msg);
    MSG_WriteBits(msg, root, 11);
}

// zombies: serialized command writer (SP 0x00656F90).
void MSG_WriteAnimCommand(msg_t *msg, int snapshotTime, AnimCommandMsgState *state, const AnimCommand *command)
{
    if (state->entity != command->entnum)
    {
        state->entity = command->entnum;
        MSG_WriteBits(msg, 9, 4);
        MSG_WriteBits(msg, command->entnum, g_entNumBits); // mod (L25): networked entity numbers 1537+
    }
    if (state->time != command->startTime)
    {
        state->time = command->startTime;
        MSG_WriteBits(msg, 10, 4);
        MSG_WriteShort(msg, snapshotTime - command->startTime);
    }
    if (command->type < 1 || command->type > 8)
        return;
    MSG_WriteBits(msg, command->type, 4);
    switch (command->type)
    {
    case 1:
        MSG_WriteBits(msg, command->flags, 2);
        MSG_WriteBits(msg, command->anim, 11);
        MSG_WriteAnimBlendTime(msg, command->time);
        break;
    case 2:
    case 8:
        MSG_WriteBits(msg, command->anim, 11);
        MSG_WriteAnimBlendTime(msg, command->time);
        break;
    case 3:
    case 4:
    case 5:
        MSG_WriteBits(msg, command->flags, 2);
        MSG_WriteBits(msg, command->anim, 11);
        if (command->type == 5)
            MSG_WriteAnimRoot(msg, command->root);
        MSG_WriteAnimWeight(msg, command->weight);
        MSG_WriteAnimBlendTime(msg, command->time);
        MSG_WriteAnimRate(msg, command->rate);
        break;
    case 6:
        MSG_WriteBits(msg, command->anim, 11);
        // zombies: normalized animation time (SP 0x004E5E80).
        MSG_WriteBits(msg, (int)(command->time * 1000.0f + 0.5f), 11);
        break;
    case 7:
        MSG_WriteAnimWeight(msg, command->weight);
        MSG_WriteAnimRate(msg, command->rate);
        MSG_WriteBits(msg, command->anim, 11);
        MSG_WriteAnimBlendTime(msg, command->time);
        break;
    }
}

static float MSG_ReadAnimWeight(msg_t *msg)
{
    if (MSG_ReadBit(msg))
        return MSG_ReadBits(msg, 7) * 0.0078740157f;
    return MSG_ReadBit(msg) ? 1.0f : 0.0f;
}

static float MSG_ReadAnimBlendTime(msg_t *msg)
{
    return MSG_ReadBit(msg) ? (MSG_ReadBits(msg, 5) * 50) * 0.001f : 0.2f;
}

static float MSG_ReadAnimRate(msg_t *msg)
{
    return MSG_ReadBit(msg) ? MSG_ReadBits(msg, 7) * 0.023622047f : 1.0f;
}

// zombies: serialized command reader (SP 0x005CB6A0). SP's timestamp delta is SIGNED,
// despite the brief's ushort description: 0x005CB73F uses MOVSX, allowing future commands.
bool MSG_ReadAnimCommand(msg_t *msg, int snapshotTime, AnimCommandMsgState *state, AnimCommand *command)
{
    *command = {};
    for (;;)
    {
        command->type = MSG_ReadBits(msg, 4);
        if (msg->overflowed)
            return false;
        if (command->type == 11)
            return false;
        if (command->type == 9)
        {
            state->entity = MSG_ReadBits(msg, g_entNumBits); // mod (L25): networked entity numbers 1537+
            continue;
        }
        if (command->type == 10)
        {
            state->time = snapshotTime - (short)MSG_ReadShort(msg);
            continue;
        }
        if (command->type < 1 || command->type > 8 || state->entity < 0 || state->time == -1)
        {
            msg->overflowed = 1;
            return false;
        }
        command->entnum = state->entity;
        command->startTime = state->time;
        switch (command->type)
        {
        case 1:
            command->flags = MSG_ReadBits(msg, 2);
            command->anim = MSG_ReadBits(msg, 11);
            command->time = MSG_ReadAnimBlendTime(msg);
            break;
        case 2:
        case 8:
            command->anim = MSG_ReadBits(msg, 11);
            command->time = MSG_ReadAnimBlendTime(msg);
            break;
        case 3:
        case 4:
        case 5:
            command->flags = MSG_ReadBits(msg, 2);
            command->anim = MSG_ReadBits(msg, 11);
            if (command->type == 5)
                command->root = MSG_ReadBit(msg) ? MSG_ReadBits(msg, 11) : (MSG_ReadBit(msg) ? 3 : 0);
            command->weight = MSG_ReadAnimWeight(msg);
            command->time = MSG_ReadAnimBlendTime(msg);
            command->rate = MSG_ReadAnimRate(msg);
            break;
        case 6:
            command->anim = MSG_ReadBits(msg, 11);
            command->time = MSG_ReadBits(msg, 11) * 0.001f;
            break;
        case 7:
            command->weight = MSG_ReadAnimWeight(msg);
            command->rate = MSG_ReadAnimRate(msg);
            command->anim = MSG_ReadBits(msg, 11);
            command->time = MSG_ReadAnimBlendTime(msg);
            break;
        }
        return !msg->overflowed;
    }
}
