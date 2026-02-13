#ifndef BONES_H
#define BONES_H

#include <SDL3/SDL.h>

#include <structs.h>

typedef struct BoneItem{
	struct BoneItem* prev;
	struct BoneItem* next;
	struct BoneItem* parent;
	struct BoneItem* child;

	Vector3 pos, rot;
} BoneItem;

typedef struct{
	float** boneMatrixes;
	//not sure what else to put here
} BonePose;

typedef struct{
	BoneItem* rootBone;
	Uint32 boneCount;
	BonePose* pose;
} Skeleton;

Skeleton* genTestRig();
void drawBone(BoneItem* bone);

#endif