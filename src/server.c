#include <SDL3/SDL.h>
#include <SDL3_net/SDL_net.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include <structs.h>

extern ClientData client;

//how the fuck do i make a server system
NET_Server *hostServer;
NET_StreamSocket *gameClient;

int serverInit(){
	//gameClient = NET_CreateClient(NET_GetLocalAddresses(NULL)[0], 8080);
	if(!gameClient){
		printf("Failed to make server!\n");
		return 1;
	}
	printf("Successed to make server!\n");
	return 0;
}