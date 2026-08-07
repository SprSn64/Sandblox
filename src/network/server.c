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

#ifdef __linux__
#include <fcntl.h>
#include <unistd.h>

#else

#endif