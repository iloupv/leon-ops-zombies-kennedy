#pragma once

struct msg_t;

// zombies: SP animation command record (SP 0x004E7D80), also carried by snapshots.
struct AnimCommand
{
    int index;
    int type;
    int startTime;
    int sequence;
    int entnum;
    unsigned int anim;
    unsigned int root;
    float weight;
    float rate;
    float time;
    unsigned int flags;
};
static_assert(sizeof(AnimCommand) == 44);

struct AnimCommandMsgState
{
    int entity = -1;
    int time = -1;
};

void MSG_WriteAnimCommand(msg_t *msg, int snapshotTime, AnimCommandMsgState *state, const AnimCommand *command);
// False at the list terminator or on overflow. Invalid tags set overflowed.
bool MSG_ReadAnimCommand(msg_t *msg, int snapshotTime, AnimCommandMsgState *state, AnimCommand *command);
