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

extern DataObj* networkPlayer;

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

void netSend(void*data, size_t size) {
    if (sockfd < 0 || !hasRemote) return;

    sendto(sockfd, data, size, 0, (struct sockaddr*)&remoteAddr, sizeof(remoteAddr));
}

bool netReceive(void *out, size_t size) {
    if (sockfd < 0) return false;

    struct sockaddr_in fromAddr;
    socklen_t addrLen = sizeof(fromAddr);

    ssize_t bytes = recvfrom(sockfd, out, size, 0, (struct sockaddr*)&fromAddr, &addrLen);
    if (bytes ==size) {
        if (isNetworkHost && !hasRemote) {
            remoteAddr = fromAddr;
            hasRemote = true;
            printf("[NET] Host received connection from client!\n");
        }
        return true;
    }
    return false;
}

void netCleanup(void) {
    if (sockfd >= 0) {
        close(sockfd);
        sockfd = -1;
    }
}
#else
bool netInitHost(Uint16 port) { (void)port; return false; }
bool netInitClient(const char* ip, Uint16 port) { (void)ip; (void)port; return false; }
void netSend(void*data, size_t size) { (void)data; (void)size; }
bool netReceive(void *out, size_t size) { (void)out; (void)size; return false; }
void netCleanup(void) {}
#endif

// outside of platform specific stuff

void netSendPlayer(DataObj* player) {
    PacketPlayer pkt = {
        .type = PKT_PLAYER,
        .pos = player->pos,
        .rot = player->rot
    };
    netSend(&pkt, sizeof(pkt));
}

void netReceivePlayer(DataObj* player) {
    PacketPlayer pkt;
    while (netReceive(&pkt, sizeof(pkt))) {
        if (pkt.type == PKT_PLAYER) {
            player->pos = pkt.pos;
            player->rot = pkt.rot;
            player->networkExists = true;
        }
    }
}