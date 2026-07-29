#ifndef NETWORK_SERVER_H
#define NETWORK_SERVER_H

#include <stdbool.h>
#include <stdint.h>
#include "structs.h"

typedef enum {
    PKT_JOIN = 1,
    PKT_LEAVE = 2,
    PKT_PLAYER = 3,
} PacketType;

typedef struct {
    uint8_t type;
    char name[32];
} PacketJoin;

typedef struct {
    uint8_t type;
} PacketLeave;

typedef struct {
    uint8_t type;
    Vector3 pos;
    Vector3 rot;
} PacketPlayer;

bool netInitHost(Uint16 port);
bool netInitClient(const char* ip, Uint16 port);
void netSend(void* data, size_t size);
void netCleanup(void);

void netSendJoin(const char* name);
void netSendLeave(void);
void netSendPlayer(DataObj* player);

void netPoll(DataObj** networkPlayerPtr);

#endif