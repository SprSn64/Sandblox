#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdint.h>
#include <SDL3/SDL.h>

#include "server.h"
#include "../instances.h"
#include "../entities.h"

extern ClientData client;
extern DataType playerClass;

static struct sockaddr_in serverAddr;
static int sockfd = -1;

#ifdef __linux__
#include <fcntl.h>
#include <unistd.h>

#define SUCCESS true
#define FAILURE false

PlayerEntry* headPlayer = NULL;
Uint32 nextNetID = 0;

extern ClientData client;

Uint32 addrToInt(struct sockaddr_in* addr){
	return addr->sin_addr.s_addr;
}

char* addrToString(Uint32 addr){
	char* string = malloc(16);
	sprintf(string, "%d.%d.%d.%d", (Uint8)(addr & 0xFF), (Uint8)((addr & 0xFF00) >> 8), (Uint8)((addr & 0xFF0000) >> 16), (Uint8)((addr & 0xFF000000) >> 24));
	return string;
}

void addSelfPlayer(){
	client.selfEntry = addPlayer(addrToInt(&serverAddr));
}

PlayerEntry* addPlayer(Uint32 addr){
	PlayerEntry* newEntry = malloc(sizeof(PlayerEntry));
	if(!newEntry) return NULL;

	newEntry->addr = addr;
	newEntry->prev = NULL; newEntry->next = NULL;

	if(!headPlayer){
		headPlayer = newEntry;
		return newEntry;
	}

	PlayerEntry* currPlayer = headPlayer;
	while(currPlayer->next){
		currPlayer = currPlayer->next;
	}
	currPlayer->next = newEntry;
	newEntry->prev = currPlayer;

	return newEntry;
}

void removePlayer(PlayerEntry* player){
	if(!player) return;

	if(player->prev)player->prev->next = player->next;
	if(player->next)player->next->prev = player->prev;
	free(player);
}

PlayerEntry* playerFromAddr(Uint32 addr){
	PlayerEntry* currPlayer = headPlayer;
	while(currPlayer){
		if(currPlayer->addr == addr) return currPlayer;
		currPlayer = currPlayer->next;
	}

	return NULL;
}

void setupID(DataObj* head){
	DataObj* child = head->child;
	while (child) {
		DataObj *next = child->next;
		setupID(child);
		nextNetID++;
		child = next;
	}
}

DataObj* instFromID(DataObj* head, Uint32 id){
	if(id == 0) return head;

	DataObj* child = head->child;
	while (child) {
		if(child->netId == id) return child;
		DataObj *next = child->next;

		DataObj* foundItem = instFromID(child, id);
		if(foundItem) return foundItem;
		child = next;
	}

	return NULL;
}

bool initServer(Uint16 port){
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0) return FAILURE;

	int socketFlags = fcntl(sockfd, F_GETFL, 0);
	fcntl(sockfd, F_SETFL, socketFlags | O_NONBLOCK);

	struct sockaddr_in localAddr; memset(&localAddr, 0, sizeof(localAddr));
	localAddr.sin_family = AF_INET;
	localAddr.sin_port = htons(port);
	localAddr.sin_addr.s_addr = INADDR_ANY;

	if(bind(sockfd, (struct sockaddr*)&localAddr, sizeof(localAddr)) < 0){
		close(sockfd); sockfd = -1;
		printf("Failed to bind host to %d\n", port);
		return FAILURE;
	}

	printf("Initialised host server on %d!\n", port);
	return SUCCESS;
}
bool initClient(const char* ipAddr, Uint16 port){
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0) return FAILURE;

	int socketFlags = fcntl(sockfd, F_GETFL, 0);
	fcntl(sockfd, F_SETFL, socketFlags | O_NONBLOCK);

	memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(port);
	
	if(inet_pton(AF_INET, ipAddr, &serverAddr.sin_addr) <= 0){
		close(sockfd); sockfd = -1;
		printf("Address %s is invalid\n", ipAddr);
		return FAILURE;
	}

	printf("Connecting to server at %s:%d...\n", ipAddr, port);
	return SUCCESS;
}

char* createPack(Uint8 type, void* data, size_t size){
	char* newPack = malloc(size + 1);
	if(!newPack) return NULL;

	newPack[0] = type;
	if(data || size > 0)memcpy(&newPack[1], data, size);

	return newPack;
}

bool sendPing(void* packet, size_t size, struct sockaddr_in* target){
	if(sockfd < 0) return FAILURE;

	sendto(sockfd, packet, size, 0, (struct sockaddr*)target, sizeof(*target));
	return SUCCESS;
}

//stores retrieved data in storeLoc
ssize_t retrievePing(void *storeLoc, size_t size, struct sockaddr_in *fromAddr, socklen_t *addrLen){
	return recvfrom(sockfd, storeLoc, size, 0, (struct sockaddr*)fromAddr, addrLen);
}

void sendInitObj(DataObj* head, struct sockaddr_in* target);
void pollPings(){
	if(sockfd < 0) return;

	Uint8* buffer = malloc(512);
	struct sockaddr_in currAddr;
	socklen_t addrLen = sizeof(currAddr);

	char* ipString = NULL;
	char* serverMsg = NULL;

	bool bytesLeft = true;
	while(bytesLeft){
		ssize_t bytes = retrievePing(buffer, sizeof(buffer), &currAddr, &addrLen);
		if (bytes <= 0){
			bytesLeft = false;
			break;
		}

		Uint8 type = buffer[0];
		Uint32 addr = addrToInt(&currAddr);
		PlayerEntry* currPlayer = playerFromAddr(addr);

		printf("Buffer: %s\n", buffer);

		switch(type){
		case PACKET_CONNECT: 
			if(currPlayer) break;

			PlayerEntry* newEntry = addPlayer(addr);
			if(!newEntry) break;

			ipString = addrToString(addr);
			printf("Client %s connected!\n", ipString);
			serverMsg = malloc(256); sprintf(serverMsg, "Client %s joined!\n", ipString);
			sendPopup(serverMsg, NULL, NULL, 5);
			free(ipString);

			sendInitObj(client.gameWorld->headObj, &currAddr);
			break;
		case PACKET_DISCONNECT: 
			if(!currPlayer) break;

			removePlayer(currPlayer);

			ipString = addrToString(addr);
			printf("Client %s disconnected\n", ipString);
			serverMsg = malloc(256); sprintf(serverMsg, "Client %s left\n", ipString);
			sendPopup(serverMsg, NULL, NULL, 5);
			free(ipString);
			break;

		case PACKET_NEWINST:
			if(bytes < 4 + sizeof(DataObj)){
				printf("packet too short...!\n");
				break;
			}

			DataObj* parent = instFromID(client.gameWorld->headObj, (buffer[0] << 24) + (buffer[1] << 16) + (buffer[2] << 8) + buffer[3]);
			if(!parent){
				printf("no parents found...\n");
				break;
			}

			DataObj* newItem = malloc(sizeof(DataObj) + 4);
			DataObj* tempNewItem = memcpy(newItem, &buffer[4], sizeof(DataObj));
			if(tempNewItem)
				newItem = tempNewItem;
			else{
				free(newItem);
				break;
			}
			newItem->prev = NULL; newItem->next = NULL;
			for(int i=0; i<OBJVAL_MAX;i++){newItem->props[i] = NULL;}

			parentObject(newItem, parent);
			break;
		}
	}

	free(buffer);
}

void pingJoin(){
	if(sockfd <= 0) return;

	char* newPack = createPack(PACKET_CONNECT, NULL, 0);
	sendPing(newPack, 1, &serverAddr);
	free(newPack);
}
void pingDisconnect(){
	if(sockfd <= 0) return;

	char* newPack = createPack(PACKET_DISCONNECT, NULL, 0);
	sendPing(newPack, 1, &serverAddr);
	free(newPack);
}
void pingAddInst(DataObj* item, struct sockaddr_in* target){
	if(sockfd <= 0) return;

	char* objData = malloc(4 + sizeof(DataObj));
	sprintf(objData, "%d", item->parent->netId);
	memcpy(&objData[4], item, sizeof(DataObj));

	char* newPack = createPack(PACKET_NEWINST, objData, 0);
	sendPing(newPack, 4 + sizeof(DataObj), target);
	free(newPack); free(objData);
}

void sendInitObj(DataObj* head, struct sockaddr_in* target){
	DataObj* child = head->child;
	while (child) {
		DataObj *next = child->next;
		pingAddInst(child, target);
		child = next;
	}
}

void closeConnection(){
	if(sockfd < 0) return;

	if(client.selfEntry)
		pingDisconnect();

	close(sockfd);
	sockfd = -1;
}

#else
void initServer(Uint16 port){}
void closeConnection(){}
bool initClient(const char* ipAddr, Uint16 port){return FAILURE;}

bool sendPing(void* packet, size_t size, struct sockaddr_in* target){return FAILURE;}
ssize_t retrievePing(void *storeLoc, size_t size, struct sockaddr_in *fromAddr, socklen_t *addrLen){return 0;}
void pollPings(){}
#endif