#include <SDL3/SDL.h>

#include <string.h>
#include <math.h>
#include <stdlib.h>

#include "entities.h"
#include "instances.h"
#include "renderer.h"
#include "math.h"
#include "physics.h"

extern GameWorld game;
extern double deltaTime;
extern ButtonMap keyList[KEYBIND_MAX];
extern Mesh *playerMesh;
extern Mesh *cubePrim;
extern SDL_Texture *homerTex;

DataType accessoryClass = {"Accessory\0", 10, 0, NULL, NULL, NULL};

void playerInit(DataObj* object){
	//object->pos.y = 0;
}

void playerUpdate(DataObj* object){
	if(!game.currPlayer || object != game.currPlayer) return;
	
	Vector3 *playerVel = &object->objVel;
	//Vector3 gravity = {-object->pos.x, object->pos.y, -object->pos.z};
	
	SDL_FPoint playerMove = {0, 0};
	
	bool plrMoving = abs(keyList[KEYBIND_D].down - keyList[KEYBIND_A].down) + abs(keyList[KEYBIND_S].down - keyList[KEYBIND_W].down);
	
	if(plrMoving){
		playerMove = normalize2((SDL_FPoint){
			(SDL_cos(game.currCamera->rot.y) * (keyList[KEYBIND_D].down - keyList[KEYBIND_A].down)) + (SDL_sin(game.currCamera->rot.y) * (keyList[KEYBIND_S].down - keyList[KEYBIND_W].down)),
			(-SDL_sin(game.currCamera->rot.y) * (keyList[KEYBIND_D].down - keyList[KEYBIND_A].down)) + (SDL_cos(game.currCamera->rot.y) * (keyList[KEYBIND_S].down - keyList[KEYBIND_W].down)),
		});
		object->rot.y = atan2(playerMove.x, playerMove.y) + 3.14159;
	}
	
	if(game.currCamera->focusDist == 0){
		object->rot.y = game.currCamera->rot.y;
	}
	
	float floorY = findFloorY(object->pos, object->pos.y, game.headObj);
	
	float friction = (0.92 - 0.005 * (floorY > -INFINITY && object->pos.y <= floorY));
	float acc = 1.2;
	
	playerVel->x = (playerVel->x + playerMove.x * acc) * friction;
	playerVel->z = (playerVel->z + playerMove.y * acc) * friction;
	playerVel->y += -1 + 0.5 * (keyList[KEYBIND_SPACE].down && playerVel->y > 0);
	
	object->pos = (Vector3){object->pos.x + playerVel->x * deltaTime, object->pos.y + playerVel->y * deltaTime, object->pos.z + playerVel->z * deltaTime};
	
	if(floorY > -INFINITY && object->pos.y <= floorY){
		object->pos.y = floorY;
		playerVel->y = 20 * keyList[KEYBIND_SPACE].pressed;
	}
	if(object->pos.y < -128){
		object->pos = (Vector3){0, 5, 0};
		*playerVel = (Vector3){0, 0, 0};
	}

	object->colour.a = min(game.currCamera->focusDist / 2, 1) * 255;
	game.currCamera->pos = (Vector3){
		object->pos.x + (SDL_cos(game.currCamera->rot.x) * SDL_sin(game.currCamera->rot.y)) * game.currCamera->focusDist, 
		object->pos.y + pow(1 - min(game.currCamera->focusDist / 8, 1), 2) + 2.5 * object->scale.y - SDL_sin(game.currCamera->rot.x) * game.currCamera->focusDist, 
		object->pos.z + (SDL_cos(game.currCamera->rot.x) * SDL_cos(game.currCamera->rot.y)) * game.currCamera->focusDist};
}

void playerDraw(DataObj* object){
	SDL_FColor plrColour = ConvertSDLColour(object->colour);
	drawMesh(playerMesh, object->transform, plrColour, NULL, true);
	
	DataObj *hatItem = object->child;
	while(hatItem){
		if(hatItem->classData->id == accessoryClass.id){
			SDL_FColor hatCol = ConvertSDLColour(hatItem->colour); hatCol.a = plrColour.a;
			drawMesh(hatItem->asVoidptr[OBJVAL_MESH], object->transform, hatCol, hatItem->asVoidptr[OBJVAL_TEXTURE], true);
		}
		hatItem = hatItem->next;
	}
}

DataType meshClass = (DataType){"Mesh\0", 4, 0, NULL, NULL, NULL};

void blockInit(DataObj* object){
	object->asInt[0] = 1; // enabled
	object->asInt[1] = 1; // block type
}

void blockDraw(DataObj* object){
	Mesh *itemMesh = cubePrim;
	SDL_Texture *itemTex = NULL;
	DataObj *meshItem = firstChildOfType(object, meshClass);
	float *meshTransform = object->transform;
	float *meshMatrix;
	if(meshItem){
		if(meshItem->asVoidptr[OBJVAL_MESH])itemMesh = meshItem->asVoidptr[OBJVAL_MESH];
		if(meshItem->asVoidptr[OBJVAL_TEXTURE])itemTex = meshItem->asVoidptr[OBJVAL_TEXTURE];
		meshMatrix = genMatrix(meshItem->pos, meshItem->scale, meshItem->rot);
		meshTransform = multMatrix(meshMatrix, object->transform);
	}
	drawMesh(itemMesh, meshTransform, ConvertSDLColour(object->colour), itemTex, true);
	if(meshItem){ 
		free(meshTransform); 
		free(meshMatrix);
	}
}

void homerDraw(DataObj* object){
	drawBillboard(homerTex, (SDL_FRect){0, 0, 300, 500}, object->pos, (SDL_FPoint){1.5, 2.5}, (SDL_FPoint){3, 5});
}

DataType playerClass = {"Player\0", 2, 0, playerInit, playerUpdate, playerDraw};
DataType fuckingBeerdrinkerClass = {"beer drinker\0", 666, 0, NULL, NULL, homerDraw};
DataType blockClass = {"Block\0", 3, 0, blockInit, NULL, blockDraw};

DataType groupClass = {"Group\0", 5, 0, NULL, NULL, NULL};

void objSpinFunc(DataObj* object){
	object->rot = (Vector3){object->rot.x + 0.02, object->rot.y + 0.01, object->rot.z + 0.005};
}

extern Mesh* planePrim;
void imageDraw(DataObj* object){
	float* transform = genMatrix(object->pos, object->scale, object->rot);
	drawMesh(planePrim, transform, ConvertSDLColour(object->colour), object->asVoidptr[OBJVAL_TEXTURE], true);
}

DataType imageClass = {"Image\0", 8, 0, NULL, NULL, imageDraw};