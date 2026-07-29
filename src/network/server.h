#ifndef NETWORK_SERVER_H
#define NETWORK_SERVER_H

#include <stdbool.h>
#include "structs.h"

typedef enum {
    PKT_PLAYER = 1,
} PacketType;

typedef struct {
    uint8_t type;
    Vector3 pos;
    Vector3 rot;
} PacketPlayer;

bool netInitHost(Uint16 port);
bool netInitClient(const char* ip, Uint16 port);
void netSend(void*data, size_t size);
bool netReceive(void *out, size_t size);
void netCleanup(void);

void netSendPlayer(DataObj* player);
void netReceivePlayer(DataObj* player);

#endif