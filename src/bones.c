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

Skeleton* genTestRig(){
	Skeleton* newRig = malloc(sizeof(Skeleton));
	newRig->boneCount = 2; newRig->pose = NULL;

	BoneItem* boneA = calloc(1, sizeof(BoneItem));
	BoneItem* boneB = calloc(1, sizeof(BoneItem)); boneB->pos = (Vector3){0, 1, 0};

	boneA->next = boneB; boneB->prev = boneA;
	newRig->rootBone = boneA;

	return newRig;
}

void drawSkeleton(Skeleton* rig){
	BoneItem* currBone = rig->rootBone;
	while (currBone) {
		float* boneMatrix = genMatrix(currBone->pos, (Vector3){1, 1, 1}, currBone->rot);
		drawMesh(boneMesh, boneMatrix, (SDL_FColor){1, 1, 1, 1}, boneTex, true);
		currBone = currBone->next;
	}
}