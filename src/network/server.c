#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <SDL3/SDL.h>

#include "server.h"

#ifdef __linux__
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>

static int sockfd = -1;
static struct sockaddr_in remoteAddr;
static bool isNetworkHost = false;
static bool hasRemote = false;

bool netInitHost(Uint16 port) {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) return false;

    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    struct sockaddr_in localAddr;
    memset(&localAddr, 0, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(port);
    localAddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (struct sockaddr*)&localAddr, sizeof(localAddr)) < 0) {
        close(sockfd);
        sockfd = -1;
        printf("[NET] Host bind failed on port %d\n", port);
        return false;
    }

    isNetworkHost = true;
    hasRemote = false;
    printf("[NET] Hosting UDP on port %d...\n", port);
    return true;
}

bool netInitClient(const char* ip, Uint16 port) {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) return false;

    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    memset(&remoteAddr, 0, sizeof(remoteAddr));
    remoteAddr.sin_family = AF_INET;
    remoteAddr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &remoteAddr.sin_addr) <= 0) {
        close(sockfd);
        sockfd = -1;
        printf("[NET] Invalid IP: %s\n", ip);
        return false;
    }

    isNetworkHost = false;
    hasRemote = true;
    printf("[NET] Connecting to host %s:%d...\n", ip, port);
    return true;
}

void netSend(void* data, size_t size) {
    if (sockfd < 0 || !hasRemote) return;

    sendto(sockfd, data, size, 0, (struct sockaddr*)&remoteAddr, sizeof(remoteAddr));
}

void netCleanup(void) {
    if (sockfd >= 0) {
        netSendLeave();
        close(sockfd);
        sockfd = -1;
    }
}
#else
bool netInitHost(Uint16 port) { (void)port; return false; }
bool netInitClient(const char* ip, Uint16 port) { (void)ip; (void)port; return false; }
void netSend(void* data, size_t size) { (void)data; (void)size; }
void netCleanup(void) {}
#endif

void netSendJoin(const char *name) {
    PacketJoin pkt = { .type = PKT_JOIN };
    if (name) strncpy(pkt.name, name, 31);
    netSend(&pkt, sizeof(pkt));
}

void netSendLeave(void) {
    PacketLeave pkt = { .type = PKT_LEAVE };
    netSend(&pkt, sizeof(pkt));
}

void netSendPlayer(DataObj *player) {
    if (!player) return;
    PacketPlayer pkt = {
        .type = PKT_PLAYER,
        .pos = player->pos,
        .rot = player->rot
    };
    netSend(&pkt, sizeof(pkt));
}

void netPoll(DataObj **networkPlayerPtr) {
    if (sockfd < 0) return;

    uint8_t buffer[512];
    struct sockaddr_in fromAddr;
    socklen_t addrLen = sizeof(fromAddr);

    while (1) {
        ssize_t bytes = recvfrom(sockfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&fromAddr, &addrLen);
        if (bytes <= 0) break;

        if (isNetworkHost && !hasRemote) {
            remoteAddr = fromAddr;
            hasRemote = true;
            printf("[NET] Host received remote connection!\n");
        }

        uint8_t type = buffer[0];
        switch (type) {
            case PKT_JOIN: {
                PacketJoin* pkt = (PacketJoin*)buffer;
                printf("[NET] Player '%s' joined the game!\n", pkt->name);

                DataObj* player = *networkPlayerPtr;
                player->networkExists = true;
                
                if (isNetworkHost) {
                    netSendJoin("Host");
                }
                break;
            }
            case PKT_LEAVE: {
                printf("[NET] Player left the game!\n");

                DataObj* player = *networkPlayerPtr;
                player->networkExists = false;
            
                if (isNetworkHost) {
                    hasRemote = false;
                }
                break;
            }
            case PKT_PLAYER: {
                if (bytes == sizeof(PacketPlayer) && networkPlayerPtr && *networkPlayerPtr) {
                    PacketPlayer* pkt = (PacketPlayer*)buffer;
                    DataObj* player = *networkPlayerPtr;
                    player->pos = pkt->pos;
                    player->rot = pkt->rot;
                }
                break;
            }
        }
    }
}