#include <SDL3/SDL.h>
#include <GL/glew.h>

#include <string.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>

#include "entities.h"
#include "instances.h"
#include "renderer.h"
#include "math.h"
#include "physics.h"
#include "opengl.h"
#include "bones.h"

extern ClientData client;
extern GameWorld game;
extern double deltaTime;
extern ButtonMap keyList[KEYBIND_MAX];

extern Mesh *cubePrim;
extern Mesh *planePrim;
extern TextureRef *homerTex;

extern TextureRef *textBufferTex;
extern Font defaultFont;

void killPlayer(){
	if(!client.gameWorld->currPlayer) return;

	char* printLog = malloc(256);
	sprintf(printLog, "%s has died!\n", client.gameWorld->currPlayer->name);
	logToConsole(printLog, CONSOLELOG_DEFAULT);

	DataObj *loopItem = client.gameWorld->currPlayer->child;
	while(loopItem){
		DataObj *nextItem = loopItem->next;
		removeObject(loopItem);
		loopItem = nextItem;
	}

	removeObject(client.gameWorld->currPlayer);
	client.gameWorld->currPlayer = NULL;
	client.gameWorld->playerRespawn = 0;
}

/*
	DataObj* parent;
	Uint32 shape;
	Vector3 pos, rot, scale;

	bool active; //no physics
	float mass; //either this or density
	
	void (*funkyCollision)(void); //custom collision function for COLLHULL_FUNCTION
*/

void playerInit(DataObj* object){
	//object->pos.y = 0;
	object->objVel = calloc(1, sizeof(Vector3));

	object->objColl = calloc(1, sizeof(CollisionHull));
	CollisionHull* collision = object->objColl;
	collision->parent = object; collision->shape = COLLHULL_CUBE;
	collision->pos = (Vector3){-0.5, 4, -0.5}; collision->scale = (Vector3){1, 4, 1};
	collision->active = true;
}
void playerUpdate(DataObj* object){
	if(!game.currPlayer || object != game.currPlayer) return;
	
	Vector3 *playerVel = object->objVel;
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
	//lazyCollisionLoop(object, game.headObj);
	
	float friction = 0.90 - 0.005 * (floorY > -INFINITY && object->pos.y <= floorY);
	float acc = 1.2;
	
	playerVel->x = (playerVel->x + playerMove.x * acc) * friction;
	playerVel->z = (playerVel->z + playerMove.y * acc) * friction;

	playerVel->y += deltaTime * (-60 + 30 * (keyList[KEYBIND_SPACE].down && playerVel->y > 0));
	
	if(fabs(playerVel->x) + fabs(playerVel->z) > 0.008 || playerVel->y != 0)
		object->pos = (Vector3){object->pos.x + playerVel->x * deltaTime, object->pos.y + playerVel->y * deltaTime, object->pos.z + playerVel->z * deltaTime};

	if(floorY > -INFINITY && object->pos.y <= floorY){
		object->pos.y = floorY;
		playerVel->y = 20 * keyList[KEYBIND_SPACE].pressed;
	}

	object->colour.a = min(game.currCamera->focusDist / 2, 1) * 255;
	game.currCamera->pos = (Vector3){
		object->pos.x + (SDL_cos(game.currCamera->rot.x) * SDL_sin(game.currCamera->rot.y)) * game.currCamera->focusDist, 
		object->pos.y + pow(1 - min(game.currCamera->focusDist / 8, 1), 2) + 2.8 * object->scale.y - SDL_sin(game.currCamera->rot.x) * game.currCamera->focusDist, 
		object->pos.z + (SDL_cos(game.currCamera->rot.x) * SDL_cos(game.currCamera->rot.y)) * game.currCamera->focusDist
	};

	if(object->pos.y < -128){
		/*object->pos = (Vector3){0, 5, 0};
		*playerVel = (Vector3){0, 0, 0};*/
		killPlayer();
	}
}
extern Mesh *playerMesh;
extern Mesh *playerFemMesh;
void playerDraw(DataObj* object){
	SDL_FColor plrColour = ConvertSDLColour(object->colour);
	Mesh* plrMesh = playerMesh;
	DataObj* femBody = firstChildWithName(object, "femBody");
	if(femBody && femBody->classData->id == groupClass.id)
		plrMesh = playerFemMesh;
	drawMeshOpenGL(plrMesh, object->transform, plrColour, NULL);
	
	DataObj *hatItem = object->child;
	while(hatItem){
		if(hatItem->classData->id == accessoryClass.id){
			SDL_FColor hatCol = ConvertSDLColour(hatItem->colour); hatCol.a = plrColour.a;
			TextureRef *itemTex = hatItem->props[OBJVAL_TEXTURE];
			drawMeshOpenGL(hatItem->props[OBJVAL_MESH], object->transform, hatCol, itemTex);
		}
		hatItem = hatItem->next;
	}

	if(object == game.currPlayer && !client.pause) return;
	/*Vector3 textPos = vec3Add(object->pos, (Vector3){0, 5, 0});
	Vector3 textProj = projToScreen(viewProj(worldToCamera(textPos)));
	if(textProj.z >= 0) return;
	float nameScale = 2;
	drawText(renderer, &defaultFont, object->name, textProj.x - strlen(object->name) / 2 * defaultFont.kerning.x * nameScale, textProj.y - defaultFont.renderSize.y * nameScale, nameScale, (SDL_FColor){1, 1, 1, 1});
	*/

	float textRatio = bufferGLText(textBufferTex, &defaultFont, object->name, 4);
	float* textMatrix = genMatrix(vec3Add(object->pos, (Vector3){0, 6, 0}), (Vector3){1 / textRatio, 1, 1}, 
		(Vector3){
			client.gameWorld->currCamera->rot.x + HALFPI,
			client.gameWorld->currCamera->rot.y,
			client.gameWorld->currCamera->rot.z
		}
	);
	drawMeshOpenGL(planePrim, textMatrix, (SDL_FColor){1, 1, 1, 1}, textBufferTex);
	free(textMatrix);
}
void playerDestroy(DataObj* object){
	free(object->objVel); free(object->objColl);
}
DataType playerClass = {"Player\0", 2, 0, playerInit, playerUpdate, playerDraw, playerDestroy};

void accessoryUpdate(DataObj* object){
	if(object->parent->classData == &playerClass){
		object->pos = object->parent->pos;
		object->scale = object->parent->scale;
		object->rot = object->parent->rot;
		return;
	}

	//do the physics thing and fall
}
void accessoryDraw(DataObj* object){
	if(object->parent->classData == &playerClass)
		return;

	TextureRef *itemTex = object->props[OBJVAL_TEXTURE];
	drawMeshOpenGL(object->props[OBJVAL_MESH], object->transform, ConvertSDLColour(object->colour), itemTex);
}
DataType accessoryClass = {"Accessory\0", 10, 0, NULL, accessoryUpdate, accessoryDraw, NULL};
DataType meshClass = (DataType){"Mesh\0", 4, 0, NULL, NULL, NULL, NULL};

void blockInit(DataObj* object){
	(void)object;
}
void blockDraw(DataObj* object){
	Mesh *itemMesh = cubePrim;
	TextureRef *itemTex = NULL;
	DataObj *meshItem = firstChildOfType(object, &meshClass);
	float *meshTransform = object->transform;
	float *meshMatrix;
	if(meshItem){
		if(meshItem->props[OBJVAL_MESH]) itemMesh = meshItem->props[OBJVAL_MESH];
		if(meshItem->props[OBJVAL_TEXTURE]) itemTex = meshItem->props[OBJVAL_TEXTURE];
		meshMatrix = genMatrix(meshItem->pos, meshItem->scale, meshItem->rot);
		meshTransform = multMatrix(meshMatrix, object->transform);
	}
	drawMeshOpenGL(itemMesh, meshTransform, ConvertSDLColour(object->colour), itemTex);
	if(meshItem){ 
		free(meshTransform); 
		free(meshMatrix);
	}
}
DataType blockClass = {"Block\0", 3, 0, blockInit, NULL, blockDraw, NULL};

void homerDraw(DataObj* object){
	drawBillboard(homerTex, (SDL_FRect){0, 0, 300, 500}, object->pos, (SDL_FPoint){1.5, 2.5}, (SDL_FPoint){3, 5});
}
DataType fuckingBeerdrinkerClass = {"beer drinker\0", 666, 0, NULL, NULL, homerDraw, NULL};

DataType groupClass = {"Group\0", 5, 0, NULL, NULL, NULL, NULL};

void cameraInit(DataObj* object){
	Camera *cam = calloc(1, sizeof(Camera));
	cam->fov = 90; cam->zoom = 90; cam->focusDist = 16;
	object->props[OBJVAL_OTHER] = cam;
}
void cameraUpdate(DataObj* object){
	Camera *cam = object->props[OBJVAL_OTHER];
	if(cam != NULL){
		free(cam->transform);
		cam->transform = genMatrix(object->pos, object->scale, object->rot);
	}
}
void cameraDestroy(DataObj* object){
	free(object->props[OBJVAL_OTHER]);
}

DataType cameraClass = {"Camera\0", 6, 0, cameraInit, cameraUpdate, NULL, cameraDestroy};

extern Mesh* planePrim;
void imageDraw(DataObj* object){
	TextureRef *itemTex = object->props[OBJVAL_TEXTURE];
	if(!itemTex) return;
	drawMeshOpenGL(planePrim, object->transform, ConvertSDLColour(object->colour), itemTex);
}
DataType imageClass = {"Image\0", 8, 0, NULL, NULL, imageDraw, NULL};

extern Mesh* boneMesh;
void armatureInit(DataObj* object){
	Skeleton *itemSkele = genTestRig();
	object->props[OBJVAL_OTHER] = itemSkele;
}
void armatureUpdate(DataObj* object){
	if(object->parent->classData != &playerClass) return;

	object->pos = object->parent->pos;
	object->scale = object->parent->scale;
	object->rot = object->parent->rot;
}
void armatureDraw(DataObj* object){
	if(!client.debug) return;
	Skeleton *itemSkele = object->props[OBJVAL_OTHER];
	if(!itemSkele) return;

	setGlValue(GL_DEPTH_TEST, false);
	drawBone(itemSkele->rootBone, object->transform);
	setGlValue(GL_DEPTH_TEST, true);
}
void armatureDestroy(DataObj* object){
	free(object->props[OBJVAL_OTHER]);
}
DataType armatureClass = {"Armature\0", 11, 0, armatureInit, armatureUpdate, armatureDraw, armatureDestroy};

void objSpinFunc(DataObj* object){
	object->parent->rot = vec3Add(object->parent->rot, (Vector3){0.02, 0.01, 0.005});
}

void killBrickFunc(DataObj* object){
	if(!game.currPlayer) return;
	Vector3 *playerPos = &game.currPlayer->pos;
	DataObj* parent = object->parent;
	if(
		between(playerPos->x, parent->pos.x - 1, parent->pos.x + parent->scale.x + 1) && 
		between(playerPos->y, parent->pos.y - parent->scale.y - 4, parent->pos.y + 1) &&
		between(playerPos->z, parent->pos.z - 1, parent->pos.z + parent->scale.z + 1)
	)
		killPlayer();
}

void scriptUpdate(DataObj* object){
	ScriptItem *scriptFunc = object->props[OBJVAL_SCRIPT];
	if(scriptFunc != NULL)
		scriptFunc->func(object);
}

DataType scriptClass = {"Script\0", 9, 0, NULL, scriptUpdate, NULL, NULL};