#ifndef NETWORK_SERVER_H
#define NETWORK_SERVER_H

Server* serverInit(Uint16 port);
int serverUpdate();

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

#endif