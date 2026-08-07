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

bool initServer(Uint16 port){
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0) return FAILURE;

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

void closeConnection(){
	if(sockfd < 0) return;

	close(sockfd);
	sockfd = -1;
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

void pollPings(){

}

#else
void initServer(Uint16 port){}
void closeConnection(){}
bool initClient(const char* ipAddr, Uint16 port){return FAILURE;}

bool sendPing(void* packet, size_t size, struct sockaddr_in* target){return FAILURE;}
ssize_t retrievePing(void *storeLoc, size_t size, struct sockaddr_in *fromAddr, socklen_t *addrLen){return 0;}
void pollPings(){}
#endif