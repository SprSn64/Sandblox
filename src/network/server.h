#ifndef NETWORK_SERVER_H
#define NETWORK_SERVER_H

#include <stdbool.h>
#include <stdint.h>
#include "structs.h"

//winsock for windows
#ifdef _WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>
#else
	#include <netinet/in.h>
	#include <arpa/inet.h>
	#include <sys/socket.h>
#endif

#define MAX_PLAYERS 32

typedef enum PacketType {
	PKT_NONE = 0x00,
	PKT_JOIN_REQ = 0x01,
	PKT_JOIN = 0x02,
	PKT_DISCONNECT = 0x03,
	PKT_PLAYER = 0x04
} PacketType;

#pragma pack(push, 1)
typedef struct {
	Uint8 type;
	Uint8 destGlobalID;
	Uint32 srcGlobalID;
} Packet;

typedef struct {
	Uint8 type;
	Uint8 destGlobalID;
	Uint32 srcGlobalID;
} PacketJoinReq;

typedef struct {
	Uint8 type;
	Uint8 destGlobalID;
	Uint32 srcGlobalID;
} PacketJoin;

typedef struct {
	Uint8 type;
	Uint8 destGlobalID;
	Uint32 srcGlobalID;
} PacketDisconnect;

typedef struct {
	Uint8 type;
	Uint8 destGlobalID;
	Uint32 srcGlobalID;
	Vector3 pos;
	Vector3 rot;
} PacketPlayer;
#pragma pack(pop)

bool osNetInitServer(Uint16 port);
bool osNetInitClient(const char* ipAddr, Uint16 port);
void osNetClose(void);
bool osNetSend(void* data, size_t size, struct sockaddr_in* target);
ssize_t osNetRecv(void* buffer, size_t size, struct sockaddr_in* fromAddr, socklen_t* addrLen);

bool initServer(Uint16 port);
bool initClient(const char* ipAddr, Uint16 port);
void closeConnection(void);
void pollPackets(void);
void netSendJoinRequest(void);
void netSendLeave(void); 
void netSendPlayer(DataObj* player);

void removePlayer(PlayerEntry* player);
PlayerEntry* playerFromID(Uint32 id);
void addSelfPlayer(void);

#endif