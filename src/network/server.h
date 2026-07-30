#ifndef NETWORK_SERVER_H
#define NETWORK_SERVER_H

#include <stdbool.h>
#include <stdint.h>
#include "structs.h"
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#else
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#endif

#define MAX_NET_PLAYERS 16

typedef enum {
    PKT_JOIN = 1,
    PKT_LEAVE = 2,
    PKT_PLAYER = 3,
} PacketType;

typedef struct {
    struct sockaddr_in addr;
    bool active;
    uint8_t id;
    DataObj* obj;
} NetClient;

#pragma pack(push, 1)
typedef struct {
    uint8_t type;
    uint8_t id;
    char name[31];
} NetPacketJoin;

typedef struct {
    uint8_t type;
    uint8_t id;
} NetPacketLeave;

typedef struct {
    uint8_t type;
    uint8_t id;
    Vector3 pos;
    Vector3 rot;
} NetPacketPlayer;
#pragma pack(pop)

bool netInitHost(Uint16 port);
bool netInitClient(const char* ip, Uint16 port);
void netSend(void* data, size_t size);
void netCleanup(void);

void netSendJoin(const char* name);
void netSendLeave(void);
void netSendPlayer(DataObj* player);

void netPoll(void);

#endif