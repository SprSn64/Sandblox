#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "physics.h"
#include "math.h"

// check if a point is inside a block's bounding box (AABB collision)
// returns the Y position of the top of the block if collision, or -INFINITY if no collision
float checkBlockCollisionY(Vector3 pos, float footY, DataObj* block){
	if(block->asInt[0] == 0) return -INFINITY; // collision disabled
	
	if(block->asInt[1] != 1) return -INFINITY; // not a solid block
	
	// block bounds (pos is corner, scale is size)
	float bMinX = block->pos.x;
	float bMaxX = block->pos.x + block->scale.x;
	float bMinY = block->pos.y - block->scale.y;
	float bMaxY = block->pos.y;
	float bMinZ = block->pos.z;
	float bMaxZ = block->pos.z + block->scale.z;
	
	if(pos.x >= bMinX && pos.x <= bMaxX &&
	   pos.z >= bMinZ && pos.z <= bMaxZ){
		if(footY <= bMaxY && footY >= bMinY - 0.5f){
			return bMaxY;
		}
	}
	return -INFINITY;
}

float findFloorY(Vector3 pos, float footY, DataObj* item){
	float highestFloor = -INFINITY;
	
	float blockFloor = checkBlockCollisionY(pos, footY, item);
	if(blockFloor > highestFloor) highestFloor = blockFloor;
	
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