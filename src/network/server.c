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

static struct sockaddr_in serverAddress;
static int sockfd = -1;
bool hosting = false;

#ifdef __linux__
#include <fcntl.h>
#include <unistd.h>

#define SUCCESS true
#define FAILURE false

bool initServer(Uint16 port){
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(!sockfd) return FAILURE;

	struct sockaddr_in localAddress; memset(&localAddress, 0, sizeof(localAddress));
	localAddress.sin_family = AF_INET;
	localAddress.sin_port = htons(port);
	localAddress.sin_addr.s_addr = INADDR_ANY;

	if (bind(sockfd, (struct sockaddr*)&localAddress, sizeof(localAddress)) < 0) {
		close(sockfd);
		printf("Failed to bind host to %d\n", port);
		return FAILURE;
	}

	hosting = true;

	printf("Initialised host server on %d!\n", port);
	return SUCCESS;
}

void closeServer(){
	if(!sockfd) return;

	close(sockfd);
	sockfd = -1;
}

#else
void initServer(Uint16 port){printf("Fuck!\n");}
void closeServer(){printf("Fuck!\n");}
#endif