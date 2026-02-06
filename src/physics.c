#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "physics.h"
#include "math.h"

// check if a point is inside a block's bounding box (AABB collision)
// returns the Y position of the top of the block if collision, or -INFINITY if no collision
float checkBlockCollisionY(Vector3 pos, float footY, DataObj* block){
	CollisionHull *collider = block->asVoidptr[OBJVAL_COLLIDER];
	
	if(!collider) return -INFINITY; // collision disabled
	
	if(collider->shape != COLLHULL_CUBE) return -INFINITY; // not a solid block
	
	// block bounds (pos is corner, scale is size)
	Vector3 bMin = {block->pos.x - 1, block->pos.y - block->scale.y - 4, block->pos.z - 1};
	Vector3 bMax = {block->pos.x + block->scale.x + 1, block->pos.y, block->pos.z + block->scale.z + 1};
	
	if(pos.x >= bMin.x && pos.x <= bMax.x &&
	   pos.z >= bMin.z && pos.z <= bMax.z){
		if(footY <= bMax.y && footY >= bMin.y - 0.5f){
			return bMax.y;
		}
	}
	return -INFINITY;
}

float checkSphereCollisionY(Vector3 pos, float footY, DataObj* block){
	CollisionHull *collider = block->asVoidptr[OBJVAL_COLLIDER];
	
	if(!collider) return -INFINITY; // collision disabled
	
	if(collider->shape != COLLHULL_SPHERE) return -INFINITY; // not a solid block
	
	// block bounds (pos is corner, scale is size)
	float bRadius = (block->scale.x + block->scale.y + block->scale.z) / 6;
	Vector3 bDist = {block->pos.x + block->scale.x / 2 - pos.x, block->pos.y - block->scale.y / 2  - pos.y, block->pos.z + block->scale.z / 2  - pos.z};
	float pythag = bDist.x * bDist.x + bDist.y * bDist.y + bDist.z * bDist.z;
	
	if(pythag <= bRadius * bRadius){
		return block->pos.y - fabs(SDL_sin(bDist.x / block->scale.x)) * fabs(SDL_cos(bDist.z / block->scale.z)) * (block->scale.y / 2);
	}
	return -INFINITY;
}

float findFloorY(Vector3 pos, float footY, DataObj* item){
	float highestFloor = -INFINITY;
	
	float blockFloor = max(checkBlockCollisionY(pos, footY, item), checkSphereCollisionY(pos, footY, item));
	highestFloor = max(blockFloor, highestFloor);
	
	DataObj* child = item->child;
	while(child){
		float childFloor = findFloorY(pos, footY, child);
		if(childFloor > highestFloor) highestFloor = childFloor;
		child = child->next;
	}
	
	return highestFloor;
}

CollsionReturn* getCollision(CollisionHull* itemA, CollisionHull* itemB){
	CollsionReturn *output = NULL;
	
	if(itemA->shape == COLLHULL_CUBE && itemB->shape == COLLHULL_CUBE){
		if(!(between(itemA->pos.x, itemB->pos.x, itemB->pos.x + itemB->scale.x) && between(itemA->pos.y, itemB->pos.y, itemB->pos.y + itemB->scale.y) && between(itemA->pos.z, itemB->pos.z, itemB->pos.z + itemB->scale.z))) return NULL;
		output = malloc(sizeof(CollsionReturn));
		output->outNorm = (Vector3){0, itemA->pos.y - itemB->pos.y, 0};
	}
	
	/*if(collide is yes) then do'eth
		tell me the collision outputs then please
	  else*/
	return output;
	//end
}