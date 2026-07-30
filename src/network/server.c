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

static bool isNetworkHost = false;
static struct sockaddr_in serverAddr;
static NetClient netClients[MAX_NET_PLAYERS];
static uint8_t localNetId = 0;
static uint8_t nextHostAssignId = 1;

#ifdef __linux__
#include <fcntl.h>
#include <unistd.h>

static int sockfd = -1;

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
    localNetId = 0;
    memset(netClients, 0, sizeof(netClients));
    printf("[NET] Hosting UDP on port %d...\n", port);
    return true;
}

bool netInitClient(const char* ip, Uint16 port) {
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) return false;

    int flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &serverAddr.sin_addr) <= 0) {
        close(sockfd);
        sockfd = -1;
        printf("[NET] Invalid IP: %s\n", ip);
        return false;
    }

    isNetworkHost = false;
    localNetId = 255;
    memset(netClients, 0, sizeof(netClients));
    printf("[NET] Connecting to host %s:%d...\n", ip, port);
    return true;
}

void netSendTo(void* data, size_t size, struct sockaddr_in* target) {
    if (sockfd < 0) return;
    sendto(sockfd, data, size, 0, (struct sockaddr*)target, sizeof(*target));
}

void netBroadcast(void* data, size_t size, int excludeIdx) {
    for (int i = 0; i < MAX_NET_PLAYERS; i++) {
        if (netClients[i].active && i != excludeIdx) {
            netSendTo(data, size, &netClients[i].addr);
        }
    }
}

void netSend(void* data, size_t size) {
    if (isNetworkHost) {
        netBroadcast(data, size, -1);
    } else {
        netSendTo(data, size, &serverAddr);
    }
}

ssize_t netGet(void *buffer, size_t size, struct sockaddr_in *fromAddr, socklen_t *addrLen) {
    return recvfrom(sockfd, buffer, size, 0, (struct sockaddr*)fromAddr, addrLen);
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
void netBroadcast(void* data, size_t size, int excludeIdx){ (void)data; (void)size; (void)excludeIdx; }
void netSend(void* data, size_t size) { (void)data; (void)size; }
void netSendTo(void* data, size_t size, struct sockaddr_in* target) { (void)data; (void)size; (void)target; }
ssize_t netGet(void *buffer, size_t size, struct sockaddr_in *fromAddr, socklen_t *addrLen) { (void)buffer; (void)size; (void)fromAddr, (void)addrLen; return 0; }
void netCleanup(void) {}
#endif

void netSendJoin(const char *name) {
    NetPacketJoin pkt;
    memset(&pkt, 0, sizeof(pkt));
    pkt.type = PKT_JOIN;
    pkt.id = localNetId;
    if (name) strncpy(pkt.name, name, 30);
    netSend(&pkt, sizeof(pkt));
}

void netSendLeave(void) {
    NetPacketLeave pkt = { .type = PKT_LEAVE, .id = localNetId };
    netSend(&pkt, sizeof(pkt));
}

void netSendPlayer(DataObj *player) {
    if (!player) return;
    NetPacketPlayer pkt = {
        .type = PKT_PLAYER,
        .id = localNetId,
        .pos = player->pos,
        .rot = player->rot
    };
    netSend(&pkt, sizeof(pkt));
}

static int findClientById(uint8_t id) {
    for (int i = 0; i < MAX_NET_PLAYERS; i++) {
        if (netClients[i].active && netClients[i].id == id) return i;
    }
    return -1;
}

static int findClientByAddr(struct sockaddr_in* addr) {
    for (int i = 0; i < MAX_NET_PLAYERS; i++) {
        if (netClients[i].active &&
            netClients[i].addr.sin_addr.s_addr == addr->sin_addr.s_addr &&
            netClients[i].addr.sin_port == addr->sin_port) {
            return i;
        }
    }
    return -1;
}

static int addClient(struct sockaddr_in* addr, uint8_t id) {
    for (int i = 0; i < MAX_NET_PLAYERS; i++) {
        if (!netClients[i].active) {
            netClients[i].active = true;
            netClients[i].addr = *addr;
            netClients[i].id = id;
            netClients[i].obj = newObject(&playerClass);
            netClients[i].obj->networkPlayerID = id;
            netClients[i].obj->networkExists = true;
            parentObject(netClients[i].obj, client.gameWorld->headObj);
            return i;
        }
    }
    return -1;
}

static void removeClient(int idx) {
    if (idx >= 0 && idx < MAX_NET_PLAYERS && netClients[idx].active) {
        if (netClients[idx].obj) {
            if (netClients[idx].obj->name) free(netClients[idx].obj->name);
            removeObject(netClients[idx].obj);
        }
        netClients[idx].active = false;
    }
}

void netPoll(void) {
    uint8_t buffer[512];
    struct sockaddr_in fromAddr;
    socklen_t addrLen = sizeof(fromAddr);

    while (1) {
        ssize_t bytes = netGet(buffer, sizeof(buffer), &fromAddr, &addrLen);
        if (bytes <= 0) break;

        uint8_t type = buffer[0];
        int cIdx = findClientByAddr(&fromAddr);

        if (isNetworkHost) {
            if (type == PKT_JOIN) {
                NetPacketJoin* pkt = (NetPacketJoin*)buffer;
                if (cIdx == -1) {
                    uint8_t newId = nextHostAssignId++;
                    cIdx = addClient(&fromAddr, newId);
                    if (cIdx != -1) {
                        netClients[cIdx].obj->name = strdup(pkt->name);
                        printf("[NET] Player '%s' joined! Assigned ID: %d\n", pkt->name, newId);

                        char* serverMsg = malloc(256); sprintf(serverMsg, "%s joined the game!\n", pkt->name);
                		sendPopup(serverMsg, NULL, NULL, 3);

                        NetPacketJoin replyPkt;
                        memset(&replyPkt, 0, sizeof(replyPkt));
                        replyPkt.type = PKT_JOIN;
                        replyPkt.id = newId;
                        strncpy(replyPkt.name, "AssignID", 30);
                        netSendTo(&replyPkt, sizeof(replyPkt), &fromAddr);

                        for (int i = 0; i < MAX_NET_PLAYERS; i++) {
                            if (netClients[i].active && i != cIdx) {
                                NetPacketJoin syncPkt;
                                memset(&syncPkt, 0, sizeof(syncPkt));
                                syncPkt.type = PKT_JOIN;
                                syncPkt.id = netClients[i].id;
                                strncpy(syncPkt.name, netClients[i].obj->name ? netClients[i].obj->name : "Player", 30);
                                netSendTo(&syncPkt, sizeof(syncPkt), &fromAddr);
                            }
                        }

                        NetPacketJoin hostPkt;
                        memset(&hostPkt, 0, sizeof(hostPkt));
                        hostPkt.type = PKT_JOIN;
                        hostPkt.id = 0;
                        strncpy(hostPkt.name, "Host", 30);
                        netSendTo(&hostPkt, sizeof(hostPkt), &fromAddr);

                        pkt->id = newId;
                        netBroadcast(buffer, bytes, cIdx);
                    }
                }
            } else if (type == PKT_LEAVE) {
                if (cIdx != -1) {
                	char* serverMsg = malloc(256); sprintf(serverMsg, "Player ID %d left the game!\n", netClients[cIdx].id);
                	sendPopup(serverMsg, NULL, NULL, 3);

                    printf("[NET] Player ID %d left the game!\n", netClients[cIdx].id);
                    netBroadcast(buffer, bytes, cIdx);
                    removeClient(cIdx);
                }
            } else if (type == PKT_PLAYER) {
                if (cIdx != -1 && bytes == sizeof(NetPacketPlayer)) {
                    NetPacketPlayer* pkt = (NetPacketPlayer*)buffer;
                    pkt->id = netClients[cIdx].id;
                    netClients[cIdx].obj->pos = pkt->pos;
                    netClients[cIdx].obj->rot = pkt->rot;
                    netBroadcast(buffer, bytes, cIdx);
                }
            }
        } else {
            if (type == PKT_JOIN) {
                NetPacketJoin* pkt = (NetPacketJoin*)buffer;
                if (strncmp(pkt->name, "AssignID", 8) == 0) {
                    localNetId = pkt->id;
                    if (client.gameWorld->currPlayer) {
                        client.gameWorld->currPlayer->networkPlayerID = localNetId;
                    }
                    printf("[NET] Connected to host! Assigned ID: %d\n", localNetId);
                } else {
                    int existIdx = findClientById(pkt->id);
                    if (existIdx == -1) {
                        int newIdx = addClient(&serverAddr, pkt->id);
                        if (newIdx != -1) {
                            netClients[newIdx].obj->name = strdup(pkt->name);
                            printf("[NET] Remote player '%s' added (ID: %d)\n", pkt->name, pkt->id);
                        }
                    }
                }
            } else if (type == PKT_LEAVE) {
                NetPacketLeave* pkt = (NetPacketLeave*)buffer;
                int remIdx = findClientById(pkt->id);
                if (remIdx != -1) {
                    printf("[NET] Remote player ID %d left\n", pkt->id);
                    removeClient(remIdx);
                }
            } else if (type == PKT_PLAYER) {
                if (bytes == sizeof(NetPacketPlayer)) {
                    NetPacketPlayer* pkt = (NetPacketPlayer*)buffer;
                    if (pkt->id != localNetId) {
                        int pIdx = findClientById(pkt->id);
                        if (pIdx != -1) {
                            netClients[pIdx].obj->pos = pkt->pos;
                            netClients[pIdx].obj->rot = pkt->rot;
                        }
                    }
                }
            }
        }
    }
}