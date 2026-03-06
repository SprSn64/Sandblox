#include <SDL3/SDL.h>
//#include <SDL3_net/SDL_net.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <netinet/in.h>
#include <sys/socket.h>

#include <structs.h>
#include "server.h"

struct sockaddr_in address; 
int socketfd;

Server* serverInit(Uint16 port){
	socketfd = socket(AF_INET, SOCK_STREAM, 0);

	address.sin_family = AF_INET;
	address.sin_port = htons(port);
	address.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(socketfd, (struct sockaddr*)&address, sizeof(address)) < 0){
		printf("Fuck! server no worked!\n"); return NULL;
	}

	Server* newServer = malloc(sizeof(Server));
	if(newServer){
		//newServer->serverIP = address.sin_addr.s_addr; newServer->port = address.sin_port; 
		printf("Successfully made server at address %d:%d\n", address.sin_addr.s_addr, address.sin_port);
	}
	return newServer;
}

int serverUpdate(){
	listen(socketfd, 8);

	return 0;
}