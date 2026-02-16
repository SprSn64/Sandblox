#include <SDL3/SDL.h>

#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include <structs.h>
#include "../entities.h"
#include "../instances.h"
#include "../renderer.h"
#include "../math.h"
#include "../physics.h"

#include "knight.h"

extern GameWorld game;
extern double deltaTime;
extern float timer;
extern ButtonMap keyList[KEYBIND_MAX];

void knightHoverFunc(DataObj* object){
	DataObj* item = object->parent;
	item->pos = (Vector3){item->pos.x, 2 + SDL_sin(timer * 1.2) / 2, item->pos.z};
}

void soulControlFunc(DataObj* object){
	DataObj* item = object->parent;
	float speed = 4;
	game.currCamera->pos = (Vector3){0, 6, -28};
	game.currCamera->rot = (Vector3){(-8 + SDL_sin(timer / 4)) * DEG2RAD, PI + SDL_cos(timer / 6.28) * DEG2RAD, 0};
	item->pos = vec3Add(item->pos, (Vector3){
		(keyList[KEYBIND_LEFT].down - keyList[KEYBIND_RIGHT].down) * speed * deltaTime, 
		(keyList[KEYBIND_UP].down - keyList[KEYBIND_DOWN].down) * speed * deltaTime, 
		0
	});
}