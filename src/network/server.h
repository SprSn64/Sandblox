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

/*CLIENT JOIN BASIC METHOD PROBABLY:
	Client sends enter request with player ID to server
	Server sends client enter approval ping on success, or a kick ping if theyre banned
	Add player to server player list

	Server sends all current instance data to client
	Once all data is sent, load in the player character or something else
*/

/*CLIENT TO SERVER UPDATE LOOP BASIC METHOD:
	Client sends ping to server with the ping start time
	Server retrieves ping and stores the time between the client ping and when it retrieved the ping
	If server doesn't retrieve a ping from the client for 15-30 seconds, remove the player from the server

	Client sends all local instance updates to server
	Server sends all global instance updates to client
	Retrieve unloaded image, model and sound assets from server
*/

typedef enum PacketFlags{
	PACKET_NONE = 0x00,
	PACKET_CONNECT = 0x01,
	PACKET_DISCONNECT = 0x02,

	PACKET_PING = 0x03,
	PACKET_UPDATE = 0x04,

	PACKET_NEWINST = 0x0001 << 8
} PacketFlags;

bool initServer(Uint16 port);
bool initClient(const char* ipAddr, Uint16 port);
void closeConnection();

bool sendPing();
ssize_t retrievePing();
void pollPings();

#endif