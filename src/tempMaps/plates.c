#include <SDL3/SDL.h>

#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "structs.h"
#include "../entities.h"
#include "../instances.h"
#include "../renderer.h"
#include "../math.h"
#include "../physics.h"

#include "plates.h"

extern double deltaTime;

//game zone = 208x208
bool platesMade = false;
SDL_Point plateArea = {6, 6};
void PLATE_genPlates(DataObj *item){
	for(int i=0; i<plateArea.x * plateArea.y; i++){
		DataObj* newPlate = newObject(&blockClass);
		if(!newPlate) continue;

		newPlate->scale = (Vector3){20, 1, 20};
		newPlate->pos = vec3Add((Vector3){
			(i % plateArea.x) * 32 + 14, 
			0, 
			(i / plateArea.x) * 32 + 14
		}, item->pos);

		CollisionHull *collider = malloc(sizeof(CollisionHull));
		collider->shape = COLLHULL_CUBE;
		collider->offset = (Vector3){0, 0, 0}; collider->scale = newPlate->scale;
		newPlate->objColl = collider;

		parentObject(newPlate, item);
	}

	platesMade = true;
}


void roundPlate_grow(DataObj *plate){
	plate->scale = vec3Add(plate->scale, (Vector3){1, 0, 1});
	plate->pos = vec3Add(plate->pos, (Vector3){-0.5, 0, -0.5});

	CollisionHull *collider = plate->objColl; if(!collider) return;
	collider->scale = (Vector3){plate->scale.x, plate->scale.y, plate->scale.z};
}
void roundPlate_shrink(DataObj *plate){
	plate->scale = vec3Add(plate->scale, (Vector3){-1, 0, -1});
	plate->pos = vec3Add(plate->pos, (Vector3){0.5, 0, 0.5});

	CollisionHull *collider = plate->objColl; if(!collider) return;
	collider->scale = (Vector3){plate->scale.x, plate->scale.y, plate->scale.z};
}

void roundPlate_raise(DataObj *plate){
	plate->pos = vec3Add(plate->pos, (Vector3){0, 4, 0});
}
void roundPlate_lower(DataObj *plate){
	plate->pos = vec3Add(plate->pos, (Vector3){0, -4, 0});
}

void roundPlayer_green(DataObj *plr){
	plr->colour = (CharColour){0, 255, 0, 255, 0, COLOURMODE_RGB};
}

DataObj* plateHead = NULL;

Uint8 plateFuncCount = 4;
void (*plateFuncs[4])(DataObj*) = {roundPlate_grow, roundPlate_shrink, roundPlate_raise, roundPlate_lower};

DataObj* getRandomChild(DataObj* item){
	if(!item->child) return NULL;

	Uint16 childCount = 0;
	DataObj* currItem = item->child;
	while(currItem){
		childCount++;
		currItem = currItem->next;
	}

	Uint16 randChild = SDL_rand(childCount);
	currItem = item->child;
	for(int i=0; i<randChild; i++){
		if(!currItem->next) break;
		currItem = currItem->next;
	}
	return currItem;
}

void PLATE_doRound(){
	if(!plateHead) return;

	if(!plateHead->child) return;
	plateFuncs[SDL_rand(plateFuncCount)](getRandomChild(plateHead));
}

float roundInterval = 10; //time between rounds
float roundTimer = 0;
void PLATE_mainLoop(DataObj *item){
	plateHead = item;
	if(!platesMade) PLATE_genPlates(item);

	roundTimer -= deltaTime;
	if(roundTimer < 0){
		roundTimer = roundInterval;

		char* popupText = malloc(64); sprintf(popupText, "Round commenced!");
		sendPopup(popupText, NULL, NULL, 3);
		PLATE_doRound();
	}
}