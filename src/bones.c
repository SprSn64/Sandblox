#include <SDL3/SDL.h>

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>

#include <structs.h>
#include "bones.h"
#include "instances.h"
#include "renderer.h"
#include "math.h"
#include "loader.h"

extern Mesh* boneMesh;
extern SDL_Texture *boneTex;

BoneItem* newBone(BoneItem* parent, Vector3 pos, Vector3 rot){
	BoneItem* newBone = calloc(1, sizeof(BoneItem)); 
	newBone->pos = pos; newBone->rot = rot;

	if(parent == NULL) return newBone;
	newBone->parent = parent;
	newBone->next = NULL;

	if(!parent->child){
		parent->child = newBone;
	}else{
		BoneItem *loopItem = parent->child;
		while(loopItem->next){
			loopItem = loopItem->next;
		}
		loopItem->next = newBone;
		newBone->prev = loopItem;
	}

	return newBone;
}

Skeleton* genTestRig(){
	Skeleton* newRig = malloc(sizeof(Skeleton));
	newRig->boneCount = 6; newRig->pose = NULL;

	BoneItem* torsoBone = newBone(NULL, (Vector3){0, 1.5, 0}, (Vector3){0, 0, 0});
	newBone(torsoBone, (Vector3){0, 3, 0}, (Vector3){0, 0, 0});
	newBone(torsoBone, (Vector3){-0.5, 2.8, 0}, (Vector3){HALFPI, -HALFPI, 0});
	newBone(torsoBone, (Vector3){0.5, 2.8, 0}, (Vector3){HALFPI, HALFPI, 0});
	newBone(torsoBone, (Vector3){-0.5, 1.5, 0}, (Vector3){PI, 0, 0});
	newBone(torsoBone, (Vector3){0.5, 1.5, 0}, (Vector3){PI, 0, 0});

	newRig->rootBone = torsoBone;

	return newRig;
}

void drawBone(BoneItem* bone){
	BoneItem* currBone = bone;
	while (currBone) {
		float* boneMatrix = genMatrix(currBone->pos, (Vector3){1, 1, 1}, currBone->rot);
		drawMesh(boneMesh, boneMatrix, (SDL_FColor){1, 1, 1, 1}, boneTex, true);
		free(boneMatrix);
		if(currBone->child)
			drawBone(currBone->child);
		currBone = currBone->next;
	}
}