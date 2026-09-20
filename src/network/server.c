#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <SDL3/SDL.h>

#include "server.h"
#include "../instances.h"
#include "../entities.h"

#define SUCCESS true
#define FAILURE false

extern ClientData client;
extern DataType playerClass;
static struct sockaddr_in serverAddr;

#ifdef _WIN32
static SOCKET sockfd = INVALID_SOCKET;

bool osNetInitServer(Uint16 port) {
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) return FAILURE;
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) return FAILURE;
	
	u_long mode = 1;
	ioctlsocket(sockfd, FIONBIO, &mode);
	
	struct sockaddr_in localAddr; memset(&localAddr, 0, sizeof(localAddr));
	localAddr.sin_family = AF_INET;
	localAddr.sin_port = htons(port);
	localAddr.sin_addr.s_addr = INADDR_ANY;
	
	if(bind(sockfd, (struct sockaddr*)&localAddr, sizeof(localAddr)) == SOCKET_ERROR) return FAILURE;
	return SUCCESS;
}

bool osNetInitClient(const char* ipAddr, Uint16 port) {
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2,2), &wsa) != 0) return FAILURE;
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET) return FAILURE;
	
	u_long mode = 1;
	ioctlsocket(sockfd, FIONBIO, &mode);
	return SUCCESS;
}

void osNetClose(void) {
	if (sockfd != INVALID_SOCKET) closesocket(sockfd);
	sockfd = INVALID_SOCKET;
	WSACleanup();
}

bool osNetSend(void* data, size_t size, struct sockaddr_in* target) {
	if(sockfd == INVALID_SOCKET) return FAILURE;
	sendto(sockfd, (const char*)data, size, 0, (struct sockaddr*)target, sizeof(*target));
	return SUCCESS;
}

ssize_t osNetRecv(void* buffer, size_t size, struct sockaddr_in* fromAddr, socklen_t* addrLen) {
	if(sockfd == INVALID_SOCKET) return -1;
	return recvfrom(sockfd, (char*)buffer, size, 0, (struct sockaddr*)fromAddr, addrLen);
}

#else
#include <fcntl.h>
#include <unistd.h>

static int sockfd = -1;

bool osNetInitServer(Uint16 port) {
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) return FAILURE;
	
	int flags = fcntl(sockfd, F_GETFL, 0);
	fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
	
	struct sockaddr_in localAddr; memset(&localAddr, 0, sizeof(localAddr));
	localAddr.sin_family = AF_INET;
	localAddr.sin_port = htons(port);
	localAddr.sin_addr.s_addr = INADDR_ANY;
	
	if(bind(sockfd, (struct sockaddr*)&localAddr, sizeof(localAddr)) < 0) return FAILURE;
	return SUCCESS;
}

bool osNetInitClient(const char* ipAddr, Uint16 port) {
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) return FAILURE;
	int flags = fcntl(sockfd, F_GETFL, 0);
	fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);
	return SUCCESS;
}

void osNetClose(void) {
	if (sockfd >= 0) close(sockfd);
	sockfd = -1;
}

bool osNetSend(void* data, size_t size, struct sockaddr_in* target) {
	if(sockfd < 0) return FAILURE;
	sendto(sockfd, data, size, 0, (struct sockaddr*)target, sizeof(*target));
	return SUCCESS;
}

ssize_t osNetRecv(void* buffer, size_t size, struct sockaddr_in* fromAddr, socklen_t* addrLen) {
	if(sockfd < 0) return -1;
	return recvfrom(sockfd, buffer, size, 0, (struct sockaddr*)fromAddr, addrLen);
}
#endif

static PlayerEntry netPlayers[MAX_PLAYERS];

PlayerEntry* playerFromID(Uint32 id){
	if (id >= MAX_PLAYERS) return NULL;
	if (netPlayers[id].ip != 0 || netPlayers[id].character != NULL || id == 0) {
		return &netPlayers[id];
	}
	return NULL;
}

bool initServer(Uint16 port) {
	printf("Initialised host server on %d!\n", port);
	return osNetInitServer(port);
}

bool initClient(const char* ipAddr, Uint16 port) {
	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	inet_pton(AF_INET, ipAddr, &serverAddr.sin_addr);
	
	printf("Connecting to server at %s:%d...\n", ipAddr, port);
	return osNetInitClient(ipAddr, port);
}

void netSendJoinRequest() {
	PacketJoinReq req = {PKT_JOIN_REQ, 255, 0};
	osNetSend(&req, sizeof(req), &serverAddr);
}

void netSendLeave() {
	if (!client.online) return;
	PacketDisconnect req = {PKT_DISCONNECT, 255, client.playerID};
	osNetSend(&req, sizeof(req), &serverAddr);
}

void broadcastPacket(void* data, size_t size, Uint32 excludeID) {
	for (int i = 0; i < MAX_PLAYERS; i++) {
		if (netPlayers[i].ip != 0 && netPlayers[i].playerID != excludeID) {
			struct sockaddr_in target = {0};
			target.sin_family = AF_INET;
			target.sin_addr.s_addr = netPlayers[i].ip;
			target.sin_port = netPlayers[i].port;
			osNetSend(data, size, &target);
		}
	}
}

void pollPackets() {
	Uint8 buffer[512];
	struct sockaddr_in currAddr;
	socklen_t addrLen = sizeof(currAddr);
	ssize_t bytes;

	while((bytes = osNetRecv(buffer, sizeof(buffer), &currAddr, &addrLen)) > 0) {
		Packet* packet = (Packet*)buffer;
		Uint32 incomingIP = currAddr.sin_addr.s_addr;
		Uint16 incomingPort = currAddr.sin_port;

		switch(packet->type) {
			case PKT_JOIN_REQ:
				if (client.hosting) {
					Uint32 newID = 0;
					for (Uint32 i = 1; i < MAX_PLAYERS; i++) {
						if (netPlayers[i].ip == 0 && netPlayers[i].character == NULL) {
							newID = i;
							break;
						}
					}
					
					if (newID != 0) {
						PlayerEntry* newPlayer = &netPlayers[newID];
						newPlayer->playerID = newID;
						newPlayer->ip = incomingIP;
						newPlayer->port = incomingPort;
						
						newPlayer->character = newObject(&playerClass);
						if (newPlayer->character && client.gameWorld) {
							parentObject(newPlayer->character, client.gameWorld->headObj);
							char nameBuf[32];
							snprintf(nameBuf, sizeof(nameBuf), "Player %d", newID);
							newPlayer->character->name = strdup(nameBuf);
						}

						PacketJoin res = {PKT_JOIN, 255, newID};
						osNetSend(&res, sizeof(res), &currAddr);
						printf("Server: Assigned globalIndex %d to new client\n", newID);
					}
				}
				break;

			case PKT_JOIN:
				if (client.online) {
					if (client.playerID != 0xFFFFFFFF && playerFromID(packet->srcGlobalID)) {
						break; 
					}

					client.playerID = packet->srcGlobalID;
					printf("Client: Successfully joined! Assigned ID: %d\n", client.playerID);
					
					if (client.playerID < MAX_PLAYERS) {
						PlayerEntry* self = &netPlayers[client.playerID];
						self->playerID = client.playerID;
						if (client.gameWorld) {
							self->character = client.gameWorld->currPlayer;
						}
					}
				}
				break;

			case PKT_DISCONNECT: {
				PlayerEntry* p = playerFromID(packet->srcGlobalID);
				if (p) {
					removePlayer(p);
					printf("Player %d disconnected\n", packet->srcGlobalID);
				}
				if (client.hosting) {
					broadcastPacket(buffer, bytes, packet->srcGlobalID);
				}
				break;
			}

			case PKT_PLAYER:
				if (bytes >= sizeof(PacketPlayer)) {
					PacketPlayer* posPack = (PacketPlayer*)buffer;
					
					if (packet->srcGlobalID == client.playerID || packet->srcGlobalID == 0xFFFFFFFF) break;
					
					PlayerEntry* p = playerFromID(packet->srcGlobalID);
					if (!p) {
						if (packet->srcGlobalID < MAX_PLAYERS) {
							p = &netPlayers[packet->srcGlobalID];
							p->playerID = packet->srcGlobalID;
							p->ip = incomingIP;
							p->port = incomingPort;
						}
					}

					if (p) {
						if (!p->character) {
							p->character = newObject(&playerClass);
							if (p->character) {
								parentObject(p->character, client.gameWorld->headObj);
								char nameBuf[32];
								snprintf(nameBuf, sizeof(nameBuf), "Player %d", p->playerID);
								p->character->name = strdup(nameBuf);
							}
						}

						if (p->character) {
							p->character->pos = posPack->pos;
							p->character->rot = posPack->rot;
						}
					}

					if (client.hosting) {
						broadcastPacket(posPack, bytes, packet->srcGlobalID);
					}
				}
				break;
		}
	}
}

void netSendPlayer(DataObj* player) {
	if (!client.online && !client.hosting) return;
	if (!player) return;
	
	if (client.online && client.playerID == 0xFFFFFFFF) return;
	
	PacketPlayer pack;
	pack.type = PKT_PLAYER;
	pack.destGlobalID = 255;
	pack.srcGlobalID = client.playerID;
	pack.pos = player->pos;
	pack.rot = player->rot; 

	if (client.online) {
		osNetSend(&pack, sizeof(pack), &serverAddr);
	} else if (client.hosting) {
		broadcastPacket(&pack, sizeof(pack), client.playerID); 
	}
}

void closeConnection() {
	osNetClose();
}

void addSelfPlayer(void) {
	if (client.hosting) {
		client.playerID = 0; 
		
		PlayerEntry* self = &netPlayers[0];
		self->playerID = 0;
		if (client.gameWorld) {
			self->character = client.gameWorld->currPlayer; 
		}
	} else if (client.online) {
		client.playerID = 0xFFFFFFFF;
	}
}

void removePlayer(PlayerEntry* player) {
	if (!player) return;

	if (player->character && player->character != client.gameWorld->currPlayer) {
		removeObject(player->character);
	}

	memset(player, 0, sizeof(PlayerEntry));
}