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

int serverInit(){
	socketfd = socket(AF_INET, SOCK_STREAM, 0);

	address.sin_family = AF_INET;
	address.sin_port = htons(8080);
	address.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(socketfd, (struct sockaddr*)&address, sizeof(address)) >= 0)
		printf("Successfully made server!\n");
	else
		printf("Fuck! server no worked!\n");

	return 0;
}

int serverUpdate(){
	listen(socketfd, 8);

	return 0;
}